// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuthWidgetHost.generated.h"

class ULoginWidget;
class URegisterWidget;
class UTextBlock;

UCLASS()
class THEIMMORTALASK_API UAuthWidgetHost : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<ULoginWidget> LoginWidget;

	UPROPERTY()
	TObjectPtr<URegisterWidget> RegisterWidget;

	UPROPERTY()
	TObjectPtr<UTextBlock> TitleText;

	void BuildLayout();
	void ShowLogin();
	void ShowRegister();

	UFUNCTION()
	void HandleShowRegister();

	UFUNCTION()
	void HandleShowLogin();
};
