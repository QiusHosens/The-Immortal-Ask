// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthApiClient.h"

#include "Auth/AuthSettings.h"
#include "Async/Async.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogTheImmortalAskApi, Log, All);

namespace
{
	TArray<TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>> GPendingAuthRequests;

	FString ExtractErrorMessage(const FString& ResponseBody, const int32 ResponseCode)
	{
		if (!ResponseBody.IsEmpty())
		{
			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
			if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
			{
				FString Error;
				if (JsonObject->TryGetStringField(TEXT("error"), Error) && !Error.IsEmpty())
				{
					return Error;
				}
			}

			if (ResponseBody.Len() <= 160)
			{
				return ResponseBody;
			}
		}

		if (ResponseCode > 0)
		{
			return FString::Printf(TEXT("请求失败（HTTP %d）"), ResponseCode);
		}

		return TEXT("请求失败，请稍后重试");
	}

	FString BuildJsonPayload(const TSharedRef<FJsonObject>& JsonObject)
	{
		FString Payload;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload);
		FJsonSerializer::Serialize(JsonObject, Writer);
		return Payload;
	}

	void SendJsonPost(
		const FString& Url,
		const TSharedRef<FJsonObject>& Body,
		TFunction<void(bool bSuccess, const FString& Message, int32 ResponseCode, const FString& ResponseBody)> Callback)
	{
		const FString Payload = BuildJsonPayload(Body);
		TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
		GPendingAuthRequests.Add(Request);

		Request->SetURL(Url);
		Request->SetVerb(TEXT("POST"));
		Request->SetHeader(TEXT("Content-Type"), TEXT("application/json; charset=utf-8"));
		Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
		Request->SetContentAsString(Payload);
		Request->SetTimeout(15.0f);

		UE_LOG(LogTheImmortalAskApi, Log, TEXT("POST %s payload=%s"), *Url, *Payload);

		Request->OnProcessRequestComplete().BindLambda(
			[Request, Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
			{
				const int32 ResponseCode = Response.IsValid() ? Response->GetResponseCode() : 0;
				const FString ResponseBody = Response.IsValid() ? Response->GetContentAsString() : FString();
				const bool bSuccess = bConnectedSuccessfully && Response.IsValid() && ResponseCode >= 200 && ResponseCode < 300;
				const FString Message = bSuccess
					? FString()
					: (!bConnectedSuccessfully || !Response.IsValid())
						? TEXT("无法连接服务器，请确认网关已启动")
						: ExtractErrorMessage(ResponseBody, ResponseCode);

				UE_LOG(
					LogTheImmortalAskApi,
					Log,
					TEXT("POST complete success=%s code=%d body=%s"),
					bSuccess ? TEXT("true") : TEXT("false"),
					ResponseCode,
					*ResponseBody);

				AsyncTask(ENamedThreads::GameThread, [Callback, bSuccess, Message, ResponseCode, ResponseBody]()
				{
					Callback(bSuccess, Message, ResponseCode, ResponseBody);
				});

				GPendingAuthRequests.Remove(Request);
			});

		if (!Request->ProcessRequest())
		{
			GPendingAuthRequests.Remove(Request);
			AsyncTask(ENamedThreads::GameThread, [Callback]()
			{
				Callback(false, TEXT("无法发起网络请求"), 0, FString());
			});
		}
	}
}

FString FAuthApiClient::GetGatewayBaseUrl()
{
	return UAuthSettings::ResolveGatewayBaseUrl();
}

void FAuthApiClient::Register(
	const UObject* WorldContext,
	const FString& Username,
	const FString& Password,
	const FString& Email,
	FAuthMessageCallback Callback)
{
	const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("username"), Username);
	Body->SetStringField(TEXT("password"), Password);
	if (!Email.IsEmpty())
	{
		Body->SetStringField(TEXT("email"), Email);
	}

	const FString Url = FString::Printf(TEXT("%s/api/v1/auth/register"), *GetGatewayBaseUrl());
	SendJsonPost(Url, Body, [Callback](const bool bSuccess, const FString& Message, const int32, const FString&)
	{
		if (bSuccess)
		{
			Callback(true, TEXT("注册成功，请登录"));
			return;
		}

		Callback(false, Message);
	});
}

void FAuthApiClient::Login(
	const UObject* WorldContext,
	const FString& Username,
	const FString& Password,
	FLoginCallback Callback)
{
	const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetStringField(TEXT("username"), Username);
	Body->SetStringField(TEXT("password"), Password);

	const FString Url = FString::Printf(TEXT("%s/api/v1/auth/login"), *GetGatewayBaseUrl());
	SendJsonPost(Url, Body, [Callback](const bool bSuccess, const FString& Message, const int32, const FString& ResponseBody)
	{
		if (!bSuccess)
		{
			Callback(false, Message, 0, FString(), 0);
			return;
		}

		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
		{
			Callback(false, TEXT("登录响应解析失败"), 0, FString(), 0);
			return;
		}

		double AccountIdValue = 0.0;
		double ExpiresInValue = 0.0;
		FString AccessToken;
		JsonObject->TryGetNumberField(TEXT("account_id"), AccountIdValue);
		JsonObject->TryGetNumberField(TEXT("expires_in"), ExpiresInValue);
		JsonObject->TryGetStringField(TEXT("access_token"), AccessToken);
		Callback(true, TEXT("登录成功，欢迎入道"), static_cast<int64>(AccountIdValue), AccessToken, static_cast<int64>(ExpiresInValue));
	});
}
