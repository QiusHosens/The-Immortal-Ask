// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuthWidgetStyles.h"
#include "LoginWidget.generated.h"

class UEditableTextBox;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShowRegisterRequested);

UCLASS()
class THEIMMORTALASK_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnShowRegisterRequested OnShowRegisterRequested;

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UEditableTextBox> UsernameInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> PasswordInput;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UButton> LoginButton;

	void BuildLayout();
	void SetStatus(const FString& Message, bool bIsError);
	void SetBusy(bool bBusy);

	UFUNCTION()
	void HandleLoginClicked();

	UFUNCTION()
	void HandleRegisterLinkClicked();
};
