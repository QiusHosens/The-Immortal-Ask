// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/AuthWidgetHost.h"

#include "Auth/AuthApiClient.h"
#include "Auth/AuthSession.h"
#include "UI/AuthWidgetStyles.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

using namespace AuthWidgetStyles;

DEFINE_LOG_CATEGORY_STATIC(LogTheImmortalAskAuth, Log, All);

namespace
{
	void AnchorFullScreen(UCanvasPanelSlot* Slot)
	{
		if (!Slot)
		{
			return;
		}

		Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		Slot->SetOffsets(FMargin(0.f));
		Slot->SetAlignment(FVector2D(0.f, 0.f));
	}

	void AnchorCenterAuto(UCanvasPanelSlot* Slot)
	{
		if (!Slot)
		{
			return;
		}

		Slot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		Slot->SetAlignment(FVector2D(0.5f, 0.5f));
		Slot->SetAutoSize(true);
	}

	UWidget* WrapFormPanel(UWidgetTree* Tree, UVerticalBox* Panel, const FVector2D& Size)
	{
		UBorder* PanelBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
		StylePanel(PanelBorder);
		PanelBorder->SetContent(Panel);

		USizeBox* SizeBox = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SizeBox->SetWidthOverride(Size.X);
		SizeBox->SetHeightOverride(Size.Y);
		SizeBox->SetContent(PanelBorder);
		return SizeBox;
	}
}

TSharedRef<SWidget> UAuthWidgetHost::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildLayout();
	}

	return Super::RebuildWidget();
}

void UAuthWidgetHost::NativeConstruct()
{
	Super::NativeConstruct();
	ShowLogin();
	UE_LOG(LogTheImmortalAskAuth, Log, TEXT("AuthWidgetHost constructed, root=%s"),
		WidgetTree && WidgetTree->RootWidget ? *WidgetTree->RootWidget->GetName() : TEXT("null"));
}

void UAuthWidgetHost::BuildLayout()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ApplySolidBrush(Backdrop, FLinearColor(0.01f, 0.02f, 0.06f, 0.95f));
	AnchorFullScreen(RootCanvas->AddChildToCanvas(Backdrop));

	UVerticalBox* MainColumn = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AnchorCenterAuto(RootCanvas->AddChildToCanvas(MainColumn));

	UTextBlock* TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TitleText->SetText(FText::FromString(TEXT("宿命劫：问仙")));
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetFont(TitleFont());
	TitleText->SetColorAndOpacity(FSlateColor(AccentColor()));
	AddSpacedChild(MainColumn, TitleText, 8.f);

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Subtitle->SetText(FText::FromString(TEXT("凡人问道，仙途始于此")));
	Subtitle->SetJustification(ETextJustify::Center);
	Subtitle->SetFont(HintFont());
	Subtitle->SetColorAndOpacity(FSlateColor(TextColor()));
	AddSpacedChild(MainColumn, Subtitle, 28.f);

	LoginPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	{
		UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Heading->SetText(FText::FromString(TEXT("登 录")));
		Heading->SetFont(TitleFont());
		Heading->SetColorAndOpacity(FSlateColor(AccentColor()));
		AddSpacedChild(LoginPanel, Heading, 20.f);

		AddSpacedChild(LoginPanel, MakeLabel(WidgetTree, TEXT("道号")));
		LoginUsernameInput = MakeInput(WidgetTree, TEXT("请输入用户名"));
		AddSpacedChild(LoginPanel, LoginUsernameInput, 16.f);

		AddSpacedChild(LoginPanel, MakeLabel(WidgetTree, TEXT("口令")));
		LoginPasswordInput = MakeInput(WidgetTree, TEXT("请输入密码"), true);
		AddSpacedChild(LoginPanel, LoginPasswordInput, 20.f);

		LoginButton = MakePrimaryButton(WidgetTree, TEXT("踏入仙途"));
		LoginButton->OnClicked.AddDynamic(this, &UAuthWidgetHost::HandleLoginClicked);
		AddSpacedChild(LoginPanel, LoginButton, 16.f);

		UButton* RegisterLink = MakeLinkButton(WidgetTree, TEXT("尚无道号？点击注册"));
		RegisterLink->OnClicked.AddDynamic(this, &UAuthWidgetHost::HandleShowRegister);
		AddSpacedChild(LoginPanel, RegisterLink, 12.f);

		LoginStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		LoginStatusText->SetFont(HintFont());
		LoginStatusText->SetAutoWrapText(true);
		AddSpacedChild(LoginPanel, LoginStatusText, 0.f);
	}

	RegisterPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	{
		UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Heading->SetText(FText::FromString(TEXT("注 册")));
		Heading->SetFont(TitleFont());
		Heading->SetColorAndOpacity(FSlateColor(AccentColor()));
		AddSpacedChild(RegisterPanel, Heading, 20.f);

		AddSpacedChild(RegisterPanel, MakeLabel(WidgetTree, TEXT("道号")));
		RegisterUsernameInput = MakeInput(WidgetTree, TEXT("3-32 个字符"));
		AddSpacedChild(RegisterPanel, RegisterUsernameInput, 16.f);

		AddSpacedChild(RegisterPanel, MakeLabel(WidgetTree, TEXT("传音石（邮箱，可选）")));
		RegisterEmailInput = MakeInput(WidgetTree, TEXT("name@example.com"));
		AddSpacedChild(RegisterPanel, RegisterEmailInput, 16.f);

		AddSpacedChild(RegisterPanel, MakeLabel(WidgetTree, TEXT("口令")));
		RegisterPasswordInput = MakeInput(WidgetTree, TEXT("至少 6 个字符"), true);
		AddSpacedChild(RegisterPanel, RegisterPasswordInput, 16.f);

		AddSpacedChild(RegisterPanel, MakeLabel(WidgetTree, TEXT("确认口令")));
		RegisterConfirmPasswordInput = MakeInput(WidgetTree, TEXT("再次输入密码"), true);
		AddSpacedChild(RegisterPanel, RegisterConfirmPasswordInput, 20.f);

		RegisterButton = MakePrimaryButton(WidgetTree, TEXT("开辟道途"));
		RegisterButton->OnClicked.AddDynamic(this, &UAuthWidgetHost::HandleRegisterClicked);
		AddSpacedChild(RegisterPanel, RegisterButton, 16.f);

		UButton* LoginLink = MakeLinkButton(WidgetTree, TEXT("已有道号？返回登录"));
		LoginLink->OnClicked.AddDynamic(this, &UAuthWidgetHost::HandleShowLogin);
		AddSpacedChild(RegisterPanel, LoginLink, 12.f);

		RegisterStatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		RegisterStatusText->SetFont(HintFont());
		RegisterStatusText->SetAutoWrapText(true);
		AddSpacedChild(RegisterPanel, RegisterStatusText, 0.f);
	}

	LoginFormRoot = WrapFormPanel(WidgetTree, LoginPanel, FVector2D(420.f, 460.f));
	RegisterFormRoot = WrapFormPanel(WidgetTree, RegisterPanel, FVector2D(420.f, 560.f));
	AddSpacedChild(MainColumn, LoginFormRoot, 0.f);
	AddSpacedChild(MainColumn, RegisterFormRoot, 0.f);
	RegisterFormRoot->SetVisibility(ESlateVisibility::Collapsed);
}

void UAuthWidgetHost::ShowLogin()
{
	if (LoginFormRoot)
	{
		LoginFormRoot->SetVisibility(ESlateVisibility::Visible);
	}
	if (RegisterFormRoot)
	{
		RegisterFormRoot->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UAuthWidgetHost::ShowRegister()
{
	if (LoginFormRoot)
	{
		LoginFormRoot->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (RegisterFormRoot)
	{
		RegisterFormRoot->SetVisibility(ESlateVisibility::Visible);
	}
}

void UAuthWidgetHost::SetLoginStatus(const FString& Message, const bool bIsError)
{
	if (LoginStatusText)
	{
		LoginStatusText->SetText(FText::FromString(Message));
		LoginStatusText->SetColorAndOpacity(FSlateColor(bIsError ? ErrorColor() : SuccessColor()));
	}
}

void UAuthWidgetHost::SetRegisterStatus(const FString& Message, const bool bIsError)
{
	if (RegisterStatusText)
	{
		RegisterStatusText->SetText(FText::FromString(Message));
		RegisterStatusText->SetColorAndOpacity(FSlateColor(bIsError ? ErrorColor() : SuccessColor()));
	}
}

void UAuthWidgetHost::SetLoginBusy(const bool bBusy)
{
	if (LoginButton)
	{
		LoginButton->SetIsEnabled(!bBusy);
	}
	if (LoginUsernameInput)
	{
		LoginUsernameInput->SetIsEnabled(!bBusy);
	}
	if (LoginPasswordInput)
	{
		LoginPasswordInput->SetIsEnabled(!bBusy);
	}
}

void UAuthWidgetHost::SetRegisterBusy(const bool bBusy)
{
	if (RegisterButton)
	{
		RegisterButton->SetIsEnabled(!bBusy);
	}
	if (RegisterUsernameInput)
	{
		RegisterUsernameInput->SetIsEnabled(!bBusy);
	}
	if (RegisterEmailInput)
	{
		RegisterEmailInput->SetIsEnabled(!bBusy);
	}
	if (RegisterPasswordInput)
	{
		RegisterPasswordInput->SetIsEnabled(!bBusy);
	}
	if (RegisterConfirmPasswordInput)
	{
		RegisterConfirmPasswordInput->SetIsEnabled(!bBusy);
	}
}

void UAuthWidgetHost::HandleLoginClicked()
{
	const FString Username = LoginUsernameInput ? LoginUsernameInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = LoginPasswordInput ? LoginPasswordInput->GetText().ToString() : FString();

	if (Username.Len() < 3)
	{
		SetLoginStatus(TEXT("用户名至少 3 个字符"), true);
		return;
	}
	if (Password.Len() < 6)
	{
		SetLoginStatus(TEXT("密码至少 6 个字符"), true);
		return;
	}

	SetLoginBusy(true);
	SetLoginStatus(TEXT("正在验证身份…"), false);

	FAuthApiClient::Login(
		this,
		Username,
		Password,
		[this](const bool bSuccess, const FString& Message, const int64 AccountId, const FString& AccessToken, const int64 ExpiresIn)
		{
			SetLoginBusy(false);
			SetLoginStatus(Message, !bSuccess);

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

void UAuthWidgetHost::HandleRegisterClicked()
{
	const FString Username = RegisterUsernameInput ? RegisterUsernameInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Email = RegisterEmailInput ? RegisterEmailInput->GetText().ToString().TrimStartAndEnd() : FString();
	const FString Password = RegisterPasswordInput ? RegisterPasswordInput->GetText().ToString() : FString();
	const FString ConfirmPassword = RegisterConfirmPasswordInput ? RegisterConfirmPasswordInput->GetText().ToString() : FString();

	if (Username.Len() < 3)
	{
		SetRegisterStatus(TEXT("用户名至少 3 个字符"), true);
		return;
	}
	if (Password.Len() < 6)
	{
		SetRegisterStatus(TEXT("密码至少 6 个字符"), true);
		return;
	}
	if (Password != ConfirmPassword)
	{
		SetRegisterStatus(TEXT("两次输入的密码不一致"), true);
		return;
	}

	SetRegisterBusy(true);
	SetRegisterStatus(TEXT("正在注册…"), false);

	FAuthApiClient::Register(
		this,
		Username,
		Password,
		Email,
		[this](const bool bSuccess, const FString& Message)
		{
			SetRegisterBusy(false);
			SetRegisterStatus(Message, !bSuccess);

			if (bSuccess)
			{
				ShowLogin();
			}
		});
}

void UAuthWidgetHost::HandleShowRegister()
{
	ShowRegister();
}

void UAuthWidgetHost::HandleShowLogin()
{
	ShowLogin();
}
