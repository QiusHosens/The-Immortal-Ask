// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogTheImmortalAskAuthSettings, Log, All);

namespace
{
	bool IsValidGatewayBaseUrl(const FString& BaseUrl)
	{
		if (BaseUrl.IsEmpty())
		{
			return false;
		}

		FString Scheme;
		FString HostAndPath;
		if (!BaseUrl.Split(TEXT("://"), &Scheme, &HostAndPath))
		{
			return false;
		}

		if (Scheme != TEXT("http") && Scheme != TEXT("https"))
		{
			return false;
		}

		FString Host;
		FString Remainder;
		if (!HostAndPath.Split(TEXT("/"), &Host, &Remainder))
		{
			Host = HostAndPath;
		}

		return !Host.IsEmpty();
	}

	FString TrimTrailingSlashes(FString BaseUrl)
	{
		BaseUrl.TrimStartAndEndInline();
		while (BaseUrl.EndsWith(TEXT("/")))
		{
			BaseUrl.LeftChopInline(1);
		}
		return BaseUrl;
	}
}

const UAuthSettings* UAuthSettings::Get()
{
	return GetDefault<UAuthSettings>();
}

FString UAuthSettings::ResolveGatewayBaseUrl()
{
	static const FString DefaultUrl = TEXT("http://127.0.0.1:8080");
	FString BaseUrl = DefaultUrl;

	if (const UAuthSettings* Settings = Get())
	{
		BaseUrl = Settings->GatewayBaseUrl;
	}

	BaseUrl = TrimTrailingSlashes(BaseUrl);
	if (!IsValidGatewayBaseUrl(BaseUrl))
	{
		UE_LOG(
			LogTheImmortalAskAuthSettings,
			Warning,
			TEXT("Invalid GatewayBaseUrl '%s' (ini 中含 // 时需加引号). Using %s"),
			*BaseUrl,
			*DefaultUrl);
		BaseUrl = DefaultUrl;
	}

	return BaseUrl;
}
