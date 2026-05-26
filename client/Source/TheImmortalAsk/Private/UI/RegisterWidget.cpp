// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/RegisterWidget.h"

#include "Auth/AuthApiClient.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

using namespace AuthWidgetStyles;

void URegisterWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
}

void URegisterWidget::BuildLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Canvas;

	UBorder* PanelBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	StylePanel(PanelBorder);

	UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	PanelBorder->SetContent(Panel);

	if (UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(PanelBorder))
	{
		PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		PanelSlot->SetSize(FVector2D(420.f, 560.f));
	}

	UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Heading->SetText(FText::FromString(TEXT("注 册")));
	Heading->SetFont(TitleFont());
	Heading->SetColorAndOpacity(FSlateColor(AccentColor()));
	AddSpacedChild(Panel, Heading, 20.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("道号")));
	UsernameInput = MakeInput(WidgetTree, TEXT("3-32 个字符"));
	AddSpacedChild(Panel, UsernameInput, 16.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("传音石（邮箱，可选）")));
	EmailInput = MakeInput(WidgetTree, TEXT("name@example.com"));
	AddSpacedChild(Panel, EmailInput, 16.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("口令")));
	PasswordInput = MakeInput(WidgetTree, TEXT("至少 6 个字符"), true);
	AddSpacedChild(Panel, PasswordInput, 16.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("确认口令")));
	ConfirmPasswordInput = MakeInput(WidgetTree, TEXT("再次输入密码"), true);
	AddSpacedChild(Panel, ConfirmPasswordInput, 20.f);

	RegisterButton = MakePrimaryButton(WidgetTree, TEXT("开辟道途"));
	RegisterButton->OnClicked.AddDynamic(this, &URegisterWidget::HandleRegisterClicked);
	AddSpacedChild(Panel, RegisterButton, 16.f);

	UButton* LoginLink = MakeLinkButton(WidgetTree, TEXT("已有道号？返回登录"));
	LoginLink->OnClicked.AddDynamic(this, &URegisterWidget::HandleLoginLinkClicked);
	AddSpacedChild(Panel, LoginLink, 12.f);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetFont(HintFont());
	StatusText->SetAutoWrapText(true);
	AddSpacedChild(Panel, StatusText, 0.f);
}

void URegisterWidget::SetStatus(const FString& Message, const bool bIsError)
{
	if (!StatusText)
	{
		return;
	}
	StatusText->SetText(FText::FromString(Message));
	StatusText->SetColorAndOpacity(FSlateColor(bIsError ? ErrorColor() : SuccessColor()));
}

void URegisterWidget::SetBusy(const bool bBusy)
{
	if (RegisterButton)
	{
		RegisterButton->SetIsEnabled(!bBusy);
	}
	if (UsernameInput)
	{
		UsernameInput->SetIsEnabled(!bBusy);
	}
	if (EmailInput)
	{
		EmailInput->SetIsEnabled(!bBusy);
	}
	if (PasswordInput)
	{
		PasswordInput->SetIsEnabled(!bBusy);
	}
	if (ConfirmPasswordInput)
	{
		ConfirmPasswordInput->SetIsEnabled(!bBusy);
	}
}

void URegisterWidget::HandleRegisterClicked()
{
	const FString Username = UsernameInput ? UsernameInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Email = EmailInput ? EmailInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();
	const FString ConfirmPassword = ConfirmPasswordInput ? ConfirmPasswordInput->GetText().ToString() : FString();

	if (Username.Len() < 3)
	{
		SetStatus(TEXT("用户名至少 3 个字符"), true);
		return;
	}
	if (Password.Len() < 6)
	{
		SetStatus(TEXT("密码至少 6 个字符"), true);
		return;
	}
	if (Password != ConfirmPassword)
	{
		SetStatus(TEXT("两次输入的密码不一致"), true);
		return;
	}

	SetBusy(true);
	SetStatus(TEXT("正在注册…"), false);

	FAuthApiClient::Register(
		this,
		Username,
		Password,
		Email,
		[this](const bool bSuccess, const FString& Message)
		{
			SetBusy(false);
			SetStatus(Message, !bSuccess);

			if (bSuccess)
			{
				OnShowLoginRequested.Broadcast();
			}
		});
}

void URegisterWidget::HandleLoginLinkClicked()
{
	OnShowLoginRequested.Broadcast();
}
