// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "MapPickerMenu.h"

#include <ToolMenus.h>
#include <ToolMenuEntry.h>
#include <Framework/Application/SlateApplication.h>
#include <Editor/EditorEngine.h>
#include <Subsystems/AssetEditorSubsystem.h>
#include <Styling/AppStyle.h>

#define LOCTEXT_NAMESPACE "MapPickerMenu"


TSharedPtr<FMapPickerMenu> FMapPickerMenu::Create(FName InEntryName, FMapPicker_GetMaps Delegate, FText InMenuName /*= FText::GetEmpty()*/, FText InTooltip /*= FText::GetEmpty()*/, FName InIconStyle/* = NAME_None*/)
{
	TSharedPtr<FMapPickerMenu> Picker;

	check(!InEntryName.IsNone());
	check(Delegate.IsBound());

	if (!IsRunningGame() && FSlateApplication::IsInitialized())
	{
		Picker = MakeShared<FMapPickerMenu>();
		Picker->EntryName = InEntryName;
		Picker->MapGetter = Delegate;
		Picker->MenuName = InMenuName.IsEmpty() ? LOCTEXT("DefaultMenuName", "Maps") : InMenuName;
		Picker->MenuTooltip = InTooltip.IsEmpty() ? LOCTEXT("DefaultMenuTooltip", "Some commonly desired maps while using the editor") : InTooltip;
		Picker->IconStyle = InIconStyle.IsNone() ? TEXT("WorldBrowser.DetailsButtonBrush") : InIconStyle;

		UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateSP(Picker.ToSharedRef(), &FMapPickerMenu::RegisterGameEditorMenus));
	}

	return Picker;
}

void FMapPickerMenu::RegisterGameEditorMenus()
{
	const FName SectionName = TEXT("PlayGameExtensions");

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection* Section = Menu->FindSection(SectionName);
	if (Section == nullptr)
	{
		Section = &Menu->AddSection(SectionName, TAttribute<FText>(), FToolMenuInsert("Play", EToolMenuInsertType::After));
	}	

	FToolMenuEntry CommonMapEntry = FToolMenuEntry::InitComboButton(
		EntryName,
		FUIAction(
			FExecuteAction(),
			FCanExecuteAction::CreateSP(this, &FMapPickerMenu::HasNoPlayWorld),
			FIsActionChecked(),
			FIsActionButtonVisible::CreateSP(this, &FMapPickerMenu::CanShowCommonMaps)),
		FOnGetContent::CreateSP(this, &FMapPickerMenu::GetCommonMapsDropdown),
		MenuName,
		MenuTooltip,
		FSlateIcon(FAppStyle::GetAppStyleSetName(), IconStyle)
	);
	CommonMapEntry.StyleNameOverride = "CalloutToolbar";

	Section->AddEntry(CommonMapEntry);
}

bool FMapPickerMenu::HasNoPlayWorld()
{
	return GEditor->PlayWorld == nullptr;
}

void FMapPickerMenu::OpenCommonMap_Clicked(const FString MapPath)
{
	if (ensure(MapPath.Len()))
	{
		GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(MapPath);
	}
}

bool FMapPickerMenu::CanShowCommonMaps()
{
	if (HasNoPlayWorld())
	{
		const TArray<FSoftObjectPath>& Maps = MapGetter.Execute();
		return Maps.Num() > 0;
	}
	return false;
}

TSharedRef<SWidget> FMapPickerMenu::GetCommonMapsDropdown()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	const TArray<FSoftObjectPath> Maps = MapGetter.Execute();
	for (const FSoftObjectPath& Path : Maps)
	{
		if (!Path.IsValid())
		{
			continue;
		}

		const FText DisplayName = FText::FromString(Path.GetAssetName());
		MenuBuilder.AddMenuEntry(
			DisplayName,
			LOCTEXT("CommonPathDescription", "Opens this map in the editor"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FMapPickerMenu::OpenCommonMap_Clicked, Path.ToString()),
				FCanExecuteAction::CreateSP(this, &FMapPickerMenu::HasNoPlayWorld),
				FIsActionChecked(),
				FIsActionButtonVisible::CreateSP(this, &FMapPickerMenu::HasNoPlayWorld)
			)
		);
	}

	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE