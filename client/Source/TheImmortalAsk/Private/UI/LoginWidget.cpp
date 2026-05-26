// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/LoginWidget.h"

#include "Auth/AuthApiClient.h"
#include "Auth/AuthSession.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"

using namespace AuthWidgetStyles;

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BuildLayout();
}

void ULoginWidget::BuildLayout()
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
		PanelSlot->SetSize(FVector2D(420.f, 460.f));
	}

	UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Heading->SetText(FText::FromString(TEXT("登 录")));
	Heading->SetFont(TitleFont());
	Heading->SetColorAndOpacity(FSlateColor(AccentColor()));
	AddSpacedChild(Panel, Heading, 20.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("道号")));
	UsernameInput = MakeInput(WidgetTree, TEXT("请输入用户名"));
	AddSpacedChild(Panel, UsernameInput, 16.f);

	AddSpacedChild(Panel, MakeLabel(WidgetTree, TEXT("口令")));
	PasswordInput = MakeInput(WidgetTree, TEXT("请输入密码"), true);
	AddSpacedChild(Panel, PasswordInput, 20.f);

	LoginButton = MakePrimaryButton(WidgetTree, TEXT("踏入仙途"));
	LoginButton->OnClicked.AddDynamic(this, &ULoginWidget::HandleLoginClicked);
	AddSpacedChild(Panel, LoginButton, 16.f);

	UButton* RegisterLink = MakeLinkButton(WidgetTree, TEXT("尚无道号？点击注册"));
	RegisterLink->OnClicked.AddDynamic(this, &ULoginWidget::HandleRegisterLinkClicked);
	AddSpacedChild(Panel, RegisterLink, 12.f);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetFont(HintFont());
	StatusText->SetAutoWrapText(true);
	AddSpacedChild(Panel, StatusText, 0.f);
}

void ULoginWidget::SetStatus(const FString& Message, const bool bIsError)
{
	if (!StatusText)
	{
		return;
	}
	StatusText->SetText(FText::FromString(Message));
	StatusText->SetColorAndOpacity(FSlateColor(bIsError ? ErrorColor() : SuccessColor()));
}

void ULoginWidget::SetBusy(const bool bBusy)
{
	if (LoginButton)
	{
		LoginButton->SetIsEnabled(!bBusy);
	}
	if (UsernameInput)
	{
		UsernameInput->SetIsEnabled(!bBusy);
	}
	if (PasswordInput)
	{
		PasswordInput->SetIsEnabled(!bBusy);
	}
}

void ULoginWidget::HandleLoginClicked()
{
	const FString Username = UsernameInput ? UsernameInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = PasswordInput ? PasswordInput->GetText().ToString() : FString();

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

	SetBusy(true);
	SetStatus(TEXT("正在验证身份…"), false);

	FAuthApiClient::Login(
		this,
		Username,
		Password,
		[this](const bool bSuccess, const FString& Message, const int64 AccountId, const FString& AccessToken, const int64 ExpiresIn)
		{
			SetBusy(false);
			SetStatus(Message, !bSuccess);

			if (bSuccess)
			{
				if (UGameInstance* GameInstance = GetGameInstance())
				{
					if (UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
					{
						Session->SetSession(AccountId, AccessToken, ExpiresIn);
					}
				}
			}
		});
}

void ULoginWidget::HandleRegisterLinkClicked()
{
	OnShowRegisterRequested.Broadcast();
}
