// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/AuthWidgetHost.h"

#include "UI/LoginWidget.h"
#include "UI/RegisterWidget.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

using namespace AuthWidgetStyles;

void UAuthWidgetHost::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
	ShowLogin();
}

void UAuthWidgetHost::BuildLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UOverlay* RootOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	WidgetTree->RootWidget = RootOverlay;

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Backdrop->SetBrushColor(FLinearColor(0.01f, 0.02f, 0.06f, 1.f));
	if (UOverlaySlot* BackdropSlot = RootOverlay->AddChildToOverlay(Backdrop))
	{
		BackdropSlot->SetHorizontalAlignment(HAlign_Fill);
		BackdropSlot->SetVerticalAlignment(VAlign_Fill);
	}

	UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UOverlaySlot* LayoutSlot = RootOverlay->AddChildToOverlay(Layout))
	{
		LayoutSlot->SetHorizontalAlignment(HAlign_Center);
		LayoutSlot->SetVerticalAlignment(VAlign_Top);
		LayoutSlot->SetPadding(FMargin(0.f, 80.f, 0.f, 0.f));
	}

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TitleText->SetText(FText::FromString(TEXT("宿命劫：问仙")));
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetFont(TitleFont());
	TitleText->SetColorAndOpacity(FSlateColor(AccentColor()));
	AddSpacedChild(Layout, TitleText, 8.f);

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Subtitle->SetText(FText::FromString(TEXT("凡人问道，仙途始于此")));
	Subtitle->SetJustification(ETextJustify::Center);
	Subtitle->SetFont(HintFont());
	Subtitle->SetColorAndOpacity(FSlateColor(TextColor()));
	AddSpacedChild(Layout, Subtitle, 36.f);

	UOverlay* FormOverlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	AddSpacedChild(Layout, FormOverlay, 0.f);

	LoginWidget = CreateWidget<ULoginWidget>(GetOwningPlayer(), ULoginWidget::StaticClass());
	RegisterWidget = CreateWidget<URegisterWidget>(GetOwningPlayer(), URegisterWidget::StaticClass());

	if (LoginWidget)
	{
		LoginWidget->OnShowRegisterRequested.AddDynamic(this, &UAuthWidgetHost::HandleShowRegister);
		FormOverlay->AddChild(LoginWidget);
	}
	if (RegisterWidget)
	{
		RegisterWidget->OnShowLoginRequested.AddDynamic(this, &UAuthWidgetHost::HandleShowLogin);
		FormOverlay->AddChild(RegisterWidget);
	}
}

void UAuthWidgetHost::ShowLogin()
{
	if (LoginWidget)
	{
		LoginWidget->SetVisibility(ESlateVisibility::Visible);
	}
	if (RegisterWidget)
	{
		RegisterWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAuthWidgetHost::ShowRegister()
{
	if (LoginWidget)
	{
		LoginWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (RegisterWidget)
	{
		RegisterWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void UAuthWidgetHost::HandleShowRegister()
{
	ShowRegister();
}

void UAuthWidgetHost::HandleShowLogin()
{
	ShowLogin();
}
