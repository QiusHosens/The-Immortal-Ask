// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/MainSceneHudWidget.h"

#include "Auth/AuthSession.h"
#include "UI/AuthWidgetStyles.h"
#include "World/ImmortalAskMapRegistry.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

using namespace AuthWidgetStyles;

TSharedRef<SWidget> UMainSceneHudWidget::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UMainSceneHudWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshFromSession();
}

void UMainSceneHudWidget::BuildLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = RootCanvas;

	UVerticalBox* TopBar = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UBorder* TopBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ApplySolidBrush(TopBorder, FLinearColor(0.01f, 0.02f, 0.06f, 0.75f));
	TopBorder->SetContent(TopBar);

	if (UCanvasPanelSlot* TopSlot = RootCanvas->AddChildToCanvas(TopBorder))
	{
		TopSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
		TopSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 96.f));
		TopSlot->SetAlignment(FVector2D(0.f, 0.f));
	}

	CharacterNameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	CharacterNameText->SetFont(TitleFont());
	CharacterNameText->SetColorAndOpacity(FSlateColor(AccentColor()));
	AddSpacedChild(TopBar, CharacterNameText, 4.f);

	CharacterInfoText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	CharacterInfoText->SetFont(BodyFont());
	CharacterInfoText->SetColorAndOpacity(FSlateColor(TextColor()));
	AddSpacedChild(TopBar, CharacterInfoText, 0.f);

	HintText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	HintText->SetFont(HintFont());
	HintText->SetColorAndOpacity(FSlateColor(TextColor()));
	HintText->SetJustification(ETextJustify::Center);

	if (UCanvasPanelSlot* HintSlot = RootCanvas->AddChildToCanvas(HintText))
	{
		HintSlot->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
		HintSlot->SetAlignment(FVector2D(0.5f, 1.f));
		HintSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 48.f));
		HintSlot->SetAutoSize(true);
	}
}

void UMainSceneHudWidget::RefreshFromSession()
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
		{
			if (CharacterNameText)
			{
				CharacterNameText->SetText(FText::FromString(Session->GetCharacterName()));
			}
			if (CharacterInfoText)
			{
				const FString MapName = FImmortalAskMapRegistry::GetMapInfo(Session->GetCharacterMapId()).DisplayName;
				const FString Info = FString::Printf(
					TEXT("Lv.%d  %s  %s  %s"),
					Session->GetCharacterLevel(),
					*Session->GetRealmStageName(),
					*Session->GetSectName(),
					*MapName);
				CharacterInfoText->SetText(FText::FromString(Info));
			}
		}
	}

	if (HintText)
	{
		HintText->SetText(FText::FromString(TEXT("WASD 移动 · 鼠标转向 · 空格跳跃 · 落霞村主场景")));
	}
}
