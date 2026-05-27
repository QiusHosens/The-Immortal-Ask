// Copyright The Immortal Ask Team. All Rights Reserved.

#include "UI/CharacterSelectWidgetHost.h"

#include "Auth/AuthSession.h"
#include "Auth/AuthSettings.h"
#include "Auth/CharacterApiClient.h"
#include "UI/AuthWidgetStyles.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

using namespace AuthWidgetStyles;

namespace
{
	void AnchorFullScreen(UCanvasPanelSlot* Slot)
	{
		if (Slot)
		{
			Slot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
			Slot->SetOffsets(FMargin(0.f));
			Slot->SetAlignment(FVector2D(0.f, 0.f));
		}
	}

	void AnchorCenterAuto(UCanvasPanelSlot* Slot)
	{
		if (Slot)
		{
			Slot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
			Slot->SetAlignment(FVector2D(0.5f, 0.5f));
			Slot->SetAutoSize(true);
		}
	}

	UWidget* WrapPanel(UWidgetTree* Tree, UVerticalBox* Panel, const FVector2D& Size)
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

TSharedRef<SWidget> UCharacterSelectWidgetHost::RebuildWidget()
{
	if (WidgetTree && !WidgetTree->RootWidget)
	{
		BuildLayout();
	}
	return Super::RebuildWidget();
}

void UCharacterSelectWidgetHost::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshData();
}

void UCharacterSelectWidgetHost::BuildLayout()
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

	RealmTitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	RealmTitleText->SetFont(TitleFont());
	RealmTitleText->SetColorAndOpacity(FSlateColor(AccentColor()));
	RealmTitleText->SetJustification(ETextJustify::Center);
	AddSpacedChild(MainColumn, RealmTitleText, 8.f);

	UTextBlock* Subtitle = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Subtitle->SetText(FText::FromString(TEXT("择道号，定灵根，入仙途")));
	Subtitle->SetFont(HintFont());
	Subtitle->SetColorAndOpacity(FSlateColor(TextColor()));
	Subtitle->SetJustification(ETextJustify::Center);
	AddSpacedChild(MainColumn, Subtitle, 20.f);

	UVerticalBox* ContentPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

	UTextBlock* ListHeading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ListHeading->SetText(FText::FromString(TEXT("已有化身")));
	ListHeading->SetFont(BodyFont());
	ListHeading->SetColorAndOpacity(FSlateColor(TextColor()));
	AddSpacedChild(ContentPanel, ListHeading, 8.f);

	CharacterListBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AddSpacedChild(ContentPanel, CharacterListBox, 8.f);

	UHorizontalBox* CharacterNav = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	PrevCharacterButton = MakeLinkButton(WidgetTree, TEXT("上一个"));
	PrevCharacterButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandlePrevCharacter);
	NextCharacterButton = MakeLinkButton(WidgetTree, TEXT("下一个"));
	NextCharacterButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleNextCharacter);
	if (UHorizontalBoxSlot* PrevSlot = CharacterNav->AddChildToHorizontalBox(PrevCharacterButton))
	{
		PrevSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
	}
	CharacterNav->AddChildToHorizontalBox(NextCharacterButton);
	AddSpacedChild(ContentPanel, CharacterNav, 16.f);

	UTextBlock* CreateHeading = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	CreateHeading->SetText(FText::FromString(TEXT("凝练新化身")));
	CreateHeading->SetFont(BodyFont());
	CreateHeading->SetColorAndOpacity(FSlateColor(TextColor()));
	AddSpacedChild(ContentPanel, CreateHeading, 8.f);

	AddSpacedChild(ContentPanel, MakeLabel(WidgetTree, TEXT("道号")));
	NameInput = MakeInput(WidgetTree, TEXT("2-16 个字符"));
	AddSpacedChild(ContentPanel, NameInput, 12.f);

	UButton* GenderButton = MakeLinkButton(WidgetTree, TEXT("性别：男"));
	GenderButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleGenderCycle);
	if (GenderButton->GetChildrenCount() > 0)
	{
		GenderValueText = Cast<UTextBlock>(GenderButton->GetChildAt(0));
	}
	AddSpacedChild(ContentPanel, GenderButton, 8.f);

	UButton* SectButton = MakeLinkButton(WidgetTree, TEXT("门派：--"));
	SectButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleSectCycle);
	if (SectButton->GetChildrenCount() > 0)
	{
		SectValueText = Cast<UTextBlock>(SectButton->GetChildAt(0));
	}
	AddSpacedChild(ContentPanel, SectButton, 8.f);

	UButton* RootButton = MakeLinkButton(WidgetTree, TEXT("灵根：--"));
	RootButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleSpiritRootCycle);
	if (RootButton->GetChildrenCount() > 0)
	{
		SpiritRootValueText = Cast<UTextBlock>(RootButton->GetChildAt(0));
	}
	AddSpacedChild(ContentPanel, RootButton, 12.f);

	CreateButton = MakePrimaryButton(WidgetTree, TEXT("凝道化身"));
	CreateButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleCreateClicked);
	AddSpacedChild(ContentPanel, CreateButton, 12.f);

	EnterButton = MakePrimaryButton(WidgetTree, TEXT("踏入仙途"));
	EnterButton->OnClicked.AddDynamic(this, &UCharacterSelectWidgetHost::HandleEnterClicked);
	AddSpacedChild(ContentPanel, EnterButton, 12.f);

	StatusText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	StatusText->SetFont(HintFont());
	StatusText->SetAutoWrapText(true);
	AddSpacedChild(ContentPanel, StatusText, 0.f);

	AddSpacedChild(MainColumn, WrapPanel(WidgetTree, ContentPanel, FVector2D(520.f, 720.f)), 0.f);
}

FString UCharacterSelectWidgetHost::GetAccessToken() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
		{
			return Session->GetAccessToken();
		}
	}
	return FString();
}

void UCharacterSelectWidgetHost::RefreshData()
{
	if (const UAuthSettings* Settings = UAuthSettings::Get())
	{
		SelectedRealmId = Settings->DefaultRealmId;
	}

	FCharacterApiClient::ListRealms(
		this,
		[this](const bool bSuccess, const FString& Message, const TArray<FRealmInfo>& Realms)
		{
			if (bSuccess)
			{
				for (const FRealmInfo& Realm : Realms)
				{
					if (Realm.Id == SelectedRealmId || SelectedRealmId <= 0)
					{
						SelectedRealmId = Realm.Id;
						SelectedRealmName = Realm.Name;
						break;
					}
				}
				if (SelectedRealmName.IsEmpty() && Realms.Num() > 0)
				{
					SelectedRealmId = Realms[0].Id;
					SelectedRealmName = Realms[0].Name;
				}
			}

			if (RealmTitleText)
			{
				RealmTitleText->SetText(FText::FromString(FString::Printf(TEXT("区服 · %s"), *SelectedRealmName)));
			}

			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
				{
					Session->SetSelectedRealm(SelectedRealmId, SelectedRealmName);
				}
			}
		});

	FCharacterApiClient::GetCreationOptions(
		this,
		[this](const bool bSuccess, const FString& Message, const FCreationOptions& Options)
		{
			if (bSuccess)
			{
				SectOptions = Options.Sects;
				SpiritRootOptions = Options.SpiritRoots;
				SelectedSectIndex = 0;
				SelectedSpiritRootIndex = 0;
				if (SectValueText && SectOptions.IsValidIndex(0))
				{
					SectValueText->SetText(FText::FromString(FString::Printf(TEXT("门派：%s"), *SectOptions[0].Name)));
				}
				if (SpiritRootValueText && SpiritRootOptions.IsValidIndex(0))
				{
					SpiritRootValueText->SetText(FText::FromString(FString::Printf(TEXT("灵根：%s"), *SpiritRootOptions[0].Name)));
				}
			}
			else
			{
				SetStatus(Message, true);
			}
		});

	ReloadCharacterList();
}

void UCharacterSelectWidgetHost::ReloadCharacterList()
{
	const FString Token = GetAccessToken();
	if (Token.IsEmpty())
	{
		SetStatus(TEXT("未登录，无法加载角色"), true);
		return;
	}

	FCharacterApiClient::ListCharacters(
		this,
		Token,
		SelectedRealmId,
		[this](const bool bSuccess, const FString& Message, const TArray<FCharacterSummary>& InCharacters)
		{
			if (!bSuccess)
			{
				SetStatus(Message, true);
				return;
			}

			Characters = InCharacters;
			SelectedCharacterIndex = Characters.Num() > 0 ? 0 : INDEX_NONE;
			SelectedPublicId = Characters.IsValidIndex(SelectedCharacterIndex) ? Characters[SelectedCharacterIndex].PublicId : FString();

			if (!PendingSelectPublicId.IsEmpty())
			{
				for (int32 Index = 0; Index < Characters.Num(); ++Index)
				{
					if (Characters[Index].PublicId == PendingSelectPublicId)
					{
						SelectedCharacterIndex = Index;
						SelectedPublicId = PendingSelectPublicId;
						break;
					}
				}
				PendingSelectPublicId.Reset();
			}

			RebuildCharacterList();
			SetStatus(Characters.Num() > 0 ? TEXT("请选择角色，或创建新化身") : TEXT("暂无角色，请创建新化身"), false);
		});
}

void UCharacterSelectWidgetHost::RebuildCharacterList()
{
	if (!CharacterListBox)
	{
		return;
	}

	CharacterListBox->ClearChildren();

	if (Characters.Num() == 0)
	{
		UTextBlock* Empty = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Empty->SetText(FText::FromString(TEXT("（尚无化身）")));
		Empty->SetFont(HintFont());
		Empty->SetColorAndOpacity(FSlateColor(TextColor()));
		AddSpacedChild(CharacterListBox, Empty, 0.f);
		return;
	}

	for (int32 Index = 0; Index < Characters.Num(); ++Index)
	{
		const FCharacterSummary& Character = Characters[Index];
		const bool bSelected = Index == SelectedCharacterIndex;
		const FString Label = FString::Printf(
			TEXT("%s%s  Lv.%d  %s  %s"),
			bSelected ? TEXT(">> ") : TEXT("   "),
			*Character.Name,
			Character.Level,
			*Character.RealmStageName,
			*Character.SectName);

		UTextBlock* RowText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		RowText->SetText(FText::FromString(Label));
		RowText->SetFont(BodyFont());
		RowText->SetColorAndOpacity(FSlateColor(bSelected ? AccentColor() : TextColor()));
		AddSpacedChild(CharacterListBox, RowText, 4.f);
	}
}

void UCharacterSelectWidgetHost::HandlePrevCharacter()
{
	if (Characters.Num() == 0)
	{
		return;
	}

	SelectedCharacterIndex = (SelectedCharacterIndex - 1 + Characters.Num()) % Characters.Num();
	SelectedPublicId = Characters[SelectedCharacterIndex].PublicId;
	RebuildCharacterList();
	SetStatus(FString::Printf(TEXT("已选择：%s"), *Characters[SelectedCharacterIndex].Name), false);
}

void UCharacterSelectWidgetHost::HandleNextCharacter()
{
	if (Characters.Num() == 0)
	{
		return;
	}

	SelectedCharacterIndex = (SelectedCharacterIndex + 1) % Characters.Num();
	SelectedPublicId = Characters[SelectedCharacterIndex].PublicId;
	RebuildCharacterList();
	SetStatus(FString::Printf(TEXT("已选择：%s"), *Characters[SelectedCharacterIndex].Name), false);
}

void UCharacterSelectWidgetHost::SetStatus(const FString& Message, const bool bIsError)
{
	if (StatusText)
	{
		StatusText->SetText(FText::FromString(Message));
		StatusText->SetColorAndOpacity(FSlateColor(bIsError ? ErrorColor() : SuccessColor()));
	}
}

void UCharacterSelectWidgetHost::SetBusy(const bool bInBusy)
{
	bBusy = bInBusy;
	if (EnterButton)
	{
		EnterButton->SetIsEnabled(!bBusy);
	}
	if (CreateButton)
	{
		CreateButton->SetIsEnabled(!bBusy);
	}
	if (NameInput)
	{
		NameInput->SetIsEnabled(!bBusy);
	}
}

void UCharacterSelectWidgetHost::HandleGenderCycle()
{
	SelectedGender = SelectedGender == 1 ? 2 : 1;
	if (GenderValueText)
	{
		GenderValueText->SetText(FText::FromString(FString::Printf(TEXT("性别：%s"), SelectedGender == 1 ? TEXT("男") : TEXT("女"))));
	}
}

void UCharacterSelectWidgetHost::HandleSectCycle()
{
	if (SectOptions.Num() == 0)
	{
		return;
	}
	SelectedSectIndex = (SelectedSectIndex + 1) % SectOptions.Num();
	if (SectValueText)
	{
		SectValueText->SetText(FText::FromString(FString::Printf(TEXT("门派：%s"), *SectOptions[SelectedSectIndex].Name)));
	}
}

void UCharacterSelectWidgetHost::HandleSpiritRootCycle()
{
	if (SpiritRootOptions.Num() == 0)
	{
		return;
	}
	SelectedSpiritRootIndex = (SelectedSpiritRootIndex + 1) % SpiritRootOptions.Num();
	if (SpiritRootValueText)
	{
		SpiritRootValueText->SetText(FText::FromString(FString::Printf(TEXT("灵根：%s"), *SpiritRootOptions[SelectedSpiritRootIndex].Name)));
	}
}

void UCharacterSelectWidgetHost::HandleCreateClicked()
{
	if (bBusy)
	{
		return;
	}

	const FString Name = NameInput ? NameInput->GetText().ToString().TrimStartAndEnd() : FString();
	if (Name.Len() < 2)
	{
		SetStatus(TEXT("道号至少 2 个字符"), true);
		return;
	}

	if (!SectOptions.IsValidIndex(SelectedSectIndex) || !SpiritRootOptions.IsValidIndex(SelectedSpiritRootIndex))
	{
		SetStatus(TEXT("创角选项尚未加载完成"), true);
		return;
	}

	const FString Token = GetAccessToken();
	if (Token.IsEmpty())
	{
		SetStatus(TEXT("登录已失效"), true);
		return;
	}

	SetBusy(true);
	SetStatus(TEXT("正在凝练化身…"), false);

	FCharacterApiClient::CreateCharacter(
		this,
		Token,
		SelectedRealmId,
		Name,
		SelectedGender,
		SectOptions[SelectedSectIndex].Id,
		SpiritRootOptions[SelectedSpiritRootIndex].Id,
		[this](const bool bSuccess, const FString& Message, const FString& PublicId, const FString& CharacterName)
		{
			SetBusy(false);
			SetStatus(Message, !bSuccess);
			if (bSuccess)
			{
				if (NameInput)
				{
					NameInput->SetText(FText::GetEmpty());
				}
				PendingSelectPublicId = PublicId;
				ReloadCharacterList();
			}
		});
}

void UCharacterSelectWidgetHost::HandleEnterClicked()
{
	if (bBusy)
	{
		return;
	}

	if (SelectedPublicId.IsEmpty())
	{
		SetStatus(TEXT("请先选择或创建一个角色"), true);
		return;
	}

	const FString Token = GetAccessToken();
	if (Token.IsEmpty())
	{
		SetStatus(TEXT("登录已失效"), true);
		return;
	}

	SetBusy(true);
	SetStatus(TEXT("正在进入世界…"), false);

	FCharacterApiClient::EnterWorld(
		this,
		Token,
		SelectedPublicId,
		[this](const bool bSuccess, const FString& Message, const FEnterWorldResult& Result)
		{
			SetBusy(false);
			SetStatus(Message, !bSuccess);
			if (!bSuccess)
			{
				return;
			}

			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (UAuthSession* Session = GameInstance->GetSubsystem<UAuthSession>())
				{
					Session->SetActiveCharacter(Result);
				}
			}

			OnCharacterFlowCompleted.Broadcast();
		});
}
