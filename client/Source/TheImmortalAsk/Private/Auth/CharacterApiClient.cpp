// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/CharacterApiClient.h"

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

FString FCharacterApiClient::GetGatewayBaseUrl()
{
	return UAuthSettings::ResolveGatewayBaseUrl();
}

void FCharacterApiClient::SendAuthorizedGet(
	const FString& Url,
	const FString& AccessToken,
	TFunction<void(bool bSuccess, const FString& Message, int32 ResponseCode, const FString& Body)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器"), 0, FString());
				return;
			}

			const int32 Code = Response->GetResponseCode();
			const FString Body = Response->GetContentAsString();
			if (Code >= 200 && Code < 300)
			{
				Callback(true, FString(), Code, Body);
				return;
			}

			Callback(false, ExtractErrorMessage(Body), Code, Body);
		});

	Request->ProcessRequest();
}

void FCharacterApiClient::SendAuthorizedPost(
	const FString& Url,
	const FString& AccessToken,
	const FString& JsonBody,
	TFunction<void(bool bSuccess, const FString& Message, int32 ResponseCode, const FString& Body)> Callback)
{
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *AccessToken));
	Request->SetContentAsString(JsonBody);
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器"), 0, FString());
				return;
			}

			const int32 Code = Response->GetResponseCode();
			const FString Body = Response->GetContentAsString();
			if (Code >= 200 && Code < 300)
			{
				Callback(true, FString(), Code, Body);
				return;
			}

			Callback(false, ExtractErrorMessage(Body), Code, Body);
		});

	Request->ProcessRequest();
}

void FCharacterApiClient::ListRealms(const UObject* WorldContext, FRealmListCallback Callback)
{
	const FString Url = FString::Printf(TEXT("%s/api/v1/realms"), *GetGatewayBaseUrl());
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			TArray<FRealmInfo> Realms;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器"), Realms);
				return;
			}

			if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
			{
				Callback(false, ExtractErrorMessage(Response->GetContentAsString()), Realms);
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("区服列表解析失败"), Realms);
				return;
			}

			const TArray<TSharedPtr<FJsonValue>>* RealmArray = nullptr;
			if (JsonObject->TryGetArrayField(TEXT("realms"), RealmArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *RealmArray)
				{
					const TSharedPtr<FJsonObject> Item = Value->AsObject();
					if (!Item.IsValid())
					{
						continue;
					}

					FRealmInfo Realm;
					Item->TryGetNumberField(TEXT("id"), Realm.Id);
					Item->TryGetStringField(TEXT("code"), Realm.Code);
					Item->TryGetStringField(TEXT("name"), Realm.Name);
					Item->TryGetNumberField(TEXT("max_characters"), Realm.MaxCharacters);
					Realms.Add(Realm);
				}
			}

			Callback(true, FString(), Realms);
		});

	Request->ProcessRequest();
}

void FCharacterApiClient::GetCreationOptions(const UObject* WorldContext, FCreationOptionsCallback Callback)
{
	const FString Url = FString::Printf(TEXT("%s/api/v1/creation-options"), *GetGatewayBaseUrl());
	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetTimeout(15.0f);

	Request->OnProcessRequestComplete().BindLambda(
		[Callback](FHttpRequestPtr, const FHttpResponsePtr& Response, const bool bConnectedSuccessfully)
		{
			FCreationOptions Options;
			if (!bConnectedSuccessfully || !Response.IsValid())
			{
				Callback(false, TEXT("无法连接服务器"), Options);
				return;
			}

			if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300)
			{
				Callback(false, ExtractErrorMessage(Response->GetContentAsString()), Options);
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("创角选项解析失败"), Options);
				return;
			}

			if (const TArray<TSharedPtr<FJsonValue>>* SectArray = nullptr; JsonObject->TryGetArrayField(TEXT("sects"), SectArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *SectArray)
				{
					const TSharedPtr<FJsonObject> Item = Value->AsObject();
					if (!Item.IsValid())
					{
						continue;
					}
					FSectOption Sect;
					Item->TryGetNumberField(TEXT("id"), Sect.Id);
					Item->TryGetStringField(TEXT("name"), Sect.Name);
					Item->TryGetStringField(TEXT("element"), Sect.Element);
					Options.Sects.Add(Sect);
				}
			}

			if (const TArray<TSharedPtr<FJsonValue>>* RootArray = nullptr; JsonObject->TryGetArrayField(TEXT("spirit_roots"), RootArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *RootArray)
				{
					const TSharedPtr<FJsonObject> Item = Value->AsObject();
					if (!Item.IsValid())
					{
						continue;
					}
					FSpiritRootOption Root;
					Item->TryGetNumberField(TEXT("id"), Root.Id);
					Item->TryGetStringField(TEXT("name"), Root.Name);
					Item->TryGetNumberField(TEXT("rarity"), Root.Rarity);
					Options.SpiritRoots.Add(Root);
				}
			}

			Callback(true, FString(), Options);
		});

	Request->ProcessRequest();
}

void FCharacterApiClient::ListCharacters(
	const UObject* WorldContext,
	const FString& AccessToken,
	const int32 RealmId,
	FCharacterListCallback Callback)
{
	const FString Url = FString::Printf(TEXT("%s/api/v1/characters?realm_id=%d"), *GetGatewayBaseUrl(), RealmId);
	SendAuthorizedGet(Url, AccessToken,
		[Callback](const bool bSuccess, const FString& Message, const int32 ResponseCode, const FString& Body)
		{
			TArray<FCharacterSummary> Characters;
			if (!bSuccess)
			{
				Callback(false, Message, Characters);
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Body);
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("角色列表解析失败"), Characters);
				return;
			}

			if (const TArray<TSharedPtr<FJsonValue>>* CharacterArray = nullptr; JsonObject->TryGetArrayField(TEXT("characters"), CharacterArray))
			{
				for (const TSharedPtr<FJsonValue>& Value : *CharacterArray)
				{
					const TSharedPtr<FJsonObject> Item = Value->AsObject();
					if (!Item.IsValid())
					{
						continue;
					}

					FCharacterSummary Summary;
					Item->TryGetStringField(TEXT("public_id"), Summary.PublicId);
					Item->TryGetStringField(TEXT("name"), Summary.Name);
					Item->TryGetNumberField(TEXT("level"), Summary.Level);
					Item->TryGetNumberField(TEXT("combat_power"), Summary.CombatPower);
					Item->TryGetStringField(TEXT("realm_stage_name"), Summary.RealmStageName);
					Item->TryGetStringField(TEXT("sect_name"), Summary.SectName);
					Characters.Add(Summary);
				}
			}

			Callback(true, FString(), Characters);
		});
}

void FCharacterApiClient::CreateCharacter(
	const UObject* WorldContext,
	const FString& AccessToken,
	const int32 RealmId,
	const FString& Name,
	const int32 Gender,
	const int32 SectId,
	const int32 SpiritRootId,
	FCreateCharacterCallback Callback)
{
	const TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
	Body->SetNumberField(TEXT("realm_id"), RealmId);
	Body->SetStringField(TEXT("name"), Name);
	Body->SetNumberField(TEXT("gender"), Gender);
	Body->SetNumberField(TEXT("sect_id"), SectId);
	Body->SetNumberField(TEXT("spirit_root_id"), SpiritRootId);

	const FString Url = FString::Printf(TEXT("%s/api/v1/characters"), *GetGatewayBaseUrl());
	SendAuthorizedPost(Url, AccessToken, BuildJsonPayload(Body),
		[Callback](const bool bSuccess, const FString& Message, const int32 ResponseCode, const FString& BodyText)
		{
			if (!bSuccess)
			{
				Callback(false, Message, FString(), FString());
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyText);
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("创角响应解析失败"), FString(), FString());
				return;
			}

			FString PublicId;
			FString NameResult;
			JsonObject->TryGetStringField(TEXT("public_id"), PublicId);
			JsonObject->TryGetStringField(TEXT("name"), NameResult);
			Callback(true, TEXT("创角成功"), PublicId, NameResult);
		});
}

void FCharacterApiClient::EnterWorld(
	const UObject* WorldContext,
	const FString& AccessToken,
	const FString& PublicId,
	FEnterWorldCallback Callback)
{
	const FString Url = FString::Printf(TEXT("%s/api/v1/characters/%s/enter"), *GetGatewayBaseUrl(), *PublicId);
	SendAuthorizedPost(Url, AccessToken, TEXT("{}"),
		[Callback](const bool bSuccess, const FString& Message, const int32 ResponseCode, const FString& BodyText)
		{
			FEnterWorldResult Result;
			if (!bSuccess)
			{
				Callback(false, Message, Result);
				return;
			}

			TSharedPtr<FJsonObject> JsonObject;
			const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(BodyText);
			if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
			{
				Callback(false, TEXT("进世界响应解析失败"), Result);
				return;
			}

			JsonObject->TryGetStringField(TEXT("public_id"), Result.PublicId);
			JsonObject->TryGetStringField(TEXT("name"), Result.Name);
			JsonObject->TryGetNumberField(TEXT("level"), Result.Level);
			JsonObject->TryGetNumberField(TEXT("map_id"), Result.MapId);
			JsonObject->TryGetNumberField(TEXT("pos_x"), Result.PosX);
			JsonObject->TryGetNumberField(TEXT("pos_y"), Result.PosY);
			JsonObject->TryGetNumberField(TEXT("pos_z"), Result.PosZ);
			JsonObject->TryGetNumberField(TEXT("rotation_yaw"), Result.RotationYaw);
			JsonObject->TryGetStringField(TEXT("realm_stage_name"), Result.RealmStageName);
			JsonObject->TryGetStringField(TEXT("sect_name"), Result.SectName);
			Callback(true, TEXT("踏入仙途"), Result);
		});
}
