// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Auth/CharacterApiClient.h"
#include "CharacterSelectWidgetHost.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterFlowCompleted);

UCLASS()
class THEIMMORTALASK_API UCharacterSelectWidgetHost : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Auth")
	FOnCharacterFlowCompleted OnCharacterFlowCompleted;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> RealmTitleText;

	UPROPERTY()
	TObjectPtr<UVerticalBox> CharacterListBox;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> NameInput;

	UPROPERTY()
	TObjectPtr<UTextBlock> GenderValueText;

	UPROPERTY()
	TObjectPtr<UTextBlock> SectValueText;

	UPROPERTY()
	TObjectPtr<UTextBlock> SpiritRootValueText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UButton> EnterButton;

	UPROPERTY()
	TObjectPtr<UButton> CreateButton;

	UPROPERTY()
	TObjectPtr<UButton> PrevCharacterButton;

	UPROPERTY()
	TObjectPtr<UButton> NextCharacterButton;

	TArray<FCharacterSummary> Characters;
	TArray<FSectOption> SectOptions;
	TArray<FSpiritRootOption> SpiritRootOptions;

	FString SelectedPublicId;
	int32 SelectedCharacterIndex = INDEX_NONE;
	int32 SelectedRealmId = 1;
	FString SelectedRealmName;
	int32 SelectedGender = 1;
	int32 SelectedSectIndex = 0;
	int32 SelectedSpiritRootIndex = 0;
	bool bBusy = false;
	FString PendingSelectPublicId;

	void BuildLayout();
	void RefreshData();
	void ReloadCharacterList();
	void RebuildCharacterList();
	void SetStatus(const FString& Message, bool bIsError);
	void SetBusy(bool bInBusy);

	FString GetAccessToken() const;

	UFUNCTION()
	void HandleEnterClicked();

	UFUNCTION()
	void HandleCreateClicked();

	UFUNCTION()
	void HandleGenderCycle();

	UFUNCTION()
	void HandleSectCycle();

	UFUNCTION()
	void HandleSpiritRootCycle();

	UFUNCTION()
	void HandlePrevCharacter();

	UFUNCTION()
	void HandleNextCharacter();
};
