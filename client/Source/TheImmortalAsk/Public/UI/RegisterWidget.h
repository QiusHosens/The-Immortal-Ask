// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuthWidgetStyles.h"
#include "RegisterWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowLoginRequested);

UCLASS()
class THEIMMORTALASK_API URegisterWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnShowLoginRequested OnShowLoginRequested;

private:
	UPROPERTY()
	TObjectPtr<UEditableTextBox> UsernameInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> EmailInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> PasswordInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> ConfirmPasswordInput;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UButton> RegisterButton;

	void BuildLayout();
	void SetStatus(const FString& Message, bool bIsError);
	void SetBusy(bool bBusy);

	UFUNCTION()
	void HandleRegisterClicked();

	UFUNCTION()
	void HandleLoginLinkClicked();
};
