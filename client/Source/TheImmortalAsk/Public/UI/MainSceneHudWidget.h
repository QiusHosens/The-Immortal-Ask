// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainSceneHudWidget.generated.h"

class UTextBlock;
class UVerticalBox;

UCLASS()
class THEIMMORTALASK_API UMainSceneHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RefreshFromSession();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TObjectPtr<UTextBlock> CharacterNameText;

	UPROPERTY()
	TObjectPtr<UTextBlock> CharacterInfoText;

	UPROPERTY()
	TObjectPtr<UTextBlock> HintText;

	void BuildLayout();
};
