// Copyright The Immortal Ask Team. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"

namespace AuthWidgetStyles
{
	inline FSlateFontInfo TitleFont()
	{
		return FCoreStyle::GetDefaultFontStyle("Bold", 36);
	}

	inline FSlateFontInfo BodyFont()
	{
		return FCoreStyle::GetDefaultFontStyle("Regular", 16);
	}

	inline FSlateFontInfo HintFont()
	{
		return FCoreStyle::GetDefaultFontStyle("Regular", 14);
	}

	inline FLinearColor BackgroundColor()
	{
		return FLinearColor(0.02f, 0.04f, 0.10f, 0.92f);
	}

	inline FLinearColor AccentColor()
	{
		return FLinearColor(0.85f, 0.68f, 0.25f, 1.0f);
	}

	inline FLinearColor TextColor()
	{
		return FLinearColor(0.92f, 0.94f, 0.98f, 1.0f);
	}

	inline FLinearColor ErrorColor()
	{
		return FLinearColor(0.95f, 0.35f, 0.35f, 1.0f);
	}

	inline FLinearColor SuccessColor()
	{
		return FLinearColor(0.45f, 0.85f, 0.55f, 1.0f);
	}

	inline void StylePanel(UBorder* Border)
	{
		if (Border)
		{
			Border->SetBrushColor(BackgroundColor());
			Border->SetPadding(FMargin(28.f, 32.f));
		}
	}

	inline UTextBlock* MakeLabel(UWidgetTree* Tree, const FString& Text)
	{
		UTextBlock* Label = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Label->SetText(FText::FromString(Text));
		Label->SetColorAndOpacity(FSlateColor(TextColor()));
		Label->SetFont(BodyFont());
		return Label;
	}

	inline UEditableTextBox* MakeInput(UWidgetTree* Tree, const FString& Hint, bool bIsPassword = false)
	{
		UEditableTextBox* Input = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
		Input->SetHintText(FText::FromString(Hint));
		FEditableTextBoxStyle Style = Input->GetWidgetStyle();
		Style.TextStyle.Font = BodyFont();
		Input->SetWidgetStyle(Style);
		if (bIsPassword)
		{
			Input->SetIsPassword(true);
		}
		return Input;
	}

	inline UButton* MakePrimaryButton(UWidgetTree* Tree, const FString& Label)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
		FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		Style.Normal.TintColor = FSlateColor(AccentColor());
		Style.Hovered.TintColor = FSlateColor(AccentColor() * 1.1f);
		Style.Pressed.TintColor = FSlateColor(AccentColor() * 0.9f);
		Button->SetStyle(Style);
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetFont(BodyFont());
		Text->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
		Button->AddChild(Text);
		return Button;
	}

	inline UButton* MakeLinkButton(UWidgetTree* Tree, const FString& Label)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
		FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		Style.Normal.TintColor = FSlateColor(FLinearColor(0.f, 0.f, 0.f, 0.f));
		Button->SetStyle(Style);
		UTextBlock* Text = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetFont(HintFont());
		Text->SetColorAndOpacity(FSlateColor(AccentColor()));
		Button->AddChild(Text);
		return Button;
	}

	inline void AddSpacedChild(UVerticalBox* Box, UWidget* Child, float BottomPadding = 12.f)
	{
		if (Box && Child)
		{
			UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
			Slot->SetPadding(FMargin(0.f, 0.f, 0.f, BottomPadding));
			Slot->SetHorizontalAlignment(HAlign_Fill);
		}
	}
}