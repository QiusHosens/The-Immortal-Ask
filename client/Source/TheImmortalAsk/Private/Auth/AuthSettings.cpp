// Copyright The Immortal Ask Team. All Rights Reserved.

#include "Auth/AuthSettings.h"

const UAuthSettings* UAuthSettings::Get()
{
	return GetDefault<UAuthSettings>();
}
