// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AuthWidgetHost.generated.h"

class UBorder;
class UButton;
class UEditableTextBox;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAuthFlowCompleted);

UCLASS()
class THEIMMORTALASK_API UAuthWidgetHost : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnAuthFlowCompleted OnAuthFlowCompleted;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UVerticalBox> LoginPanel;

	UPROPERTY()
	TObjectPtr<UVerticalBox> RegisterPanel;

	UPROPERTY()
	TObjectPtr<UWidget> LoginFormRoot;

	UPROPERTY()
	TObjectPtr<UWidget> RegisterFormRoot;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> LoginUsernameInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> LoginPasswordInput;

	UPROPERTY()
	TObjectPtr<UTextBlock> LoginStatusText;

	UPROPERTY()
	TObjectPtr<UButton> LoginButton;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> RegisterUsernameInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> RegisterEmailInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> RegisterPasswordInput;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> RegisterConfirmPasswordInput;

	UPROPERTY()
	TObjectPtr<UTextBlock> RegisterStatusText;

	UPROPERTY()
	TObjectPtr<UButton> RegisterButton;

	void BuildLayout();
	void ShowLogin();
	void ShowRegister();
	void SetLoginStatus(const FString& Message, bool bIsError);
	void SetRegisterStatus(const FString& Message, bool bIsError);
	void SetLoginBusy(bool bBusy);
	void SetRegisterBusy(bool bBusy);

	UFUNCTION()
	void HandleLoginClicked();

	UFUNCTION()
	void HandleRegisterClicked();

	UFUNCTION()
	void HandleShowRegister();

	UFUNCTION()
	void HandleShowLogin();
};
