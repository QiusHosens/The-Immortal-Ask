// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthApiClient.h"

#include "Auth/AuthSettings.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

namespace
{
	FString ExtractErrorMessage(const FString& ResponseBody)
	{
		TSharedPtr<FJsonObject> JsonObject;
		const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject.IsValid())
		{
			FString Error;
			if (JsonObject->TryGetStringField(TEXT("error"), Error))
			{
				return Error;
			}
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
}

FString FAuthApiClient::GetGatewayBaseUrl()
{
	if (const UAuthSettings* Settings = UAuthSettings::Get())
	{
		return Settings->GatewayBaseUrl;
	}
	return TEXT("http://127.0.0.1:8080");
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
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BuildJsonPayload(Body));
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器，请确认网关已启动"));
				return;
			}

			const int32 Code = Response->GetResponseCode();
			if (Code >= 200 && Code < 300)
			{
				Callback(true, TEXT("注册成功，请登录"));
				return;
			}

			Callback(false, ExtractErrorMessage(Response->GetContentAsString()));
		});

	Request->ProcessRequest();
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
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(BuildJsonPayload(Body));
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器，请确认网关已启动"), 0, FString(), 0);
				return;
			}

			const int32 Code = Response->GetResponseCode();
			if (Code < 200 || Code >= 300)
			{
				Callback(false, ExtractErrorMessage(Response->GetContentAsString()), 0, FString(), 0);
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("登录响应解析失败"), 0, FString(), 0);
				return;
			}

			int64 AccountId = 0;
			int64 ExpiresIn = 0;
			FString AccessToken;
			JsonObject->TryGetNumberField(TEXT("account_id"), AccountId);
			JsonObject->TryGetNumberField(TEXT("expires_in"), ExpiresIn);
			JsonObject->TryGetStringField(TEXT("access_token"), AccessToken);
			Callback(true, TEXT("登录成功，欢迎入道"), AccountId, AccessToken, ExpiresIn);
		});

	Request->ProcessRequest();
}
