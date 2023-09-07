// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include <Templates/SharedPointer.h>


DECLARE_DELEGATE_RetVal(const TArray<FSoftObjectPath>&, FMapPicker_GetMaps);

struct FMapPickerMenu : public TSharedFromThis<FMapPickerMenu>
{	
	static TSharedPtr<FMapPickerMenu> Create(FName EntryName, FMapPicker_GetMaps Delegate, FText MenuName = FText::GetEmpty(), FText Tooltip = FText::GetEmpty(), FName IconStyle = NAME_None);

private:
	void RegisterGameEditorMenus();
	bool HasNoPlayWorld();
	void OpenCommonMap_Clicked(const FString MapPath);
	bool CanShowCommonMaps();
	TSharedRef<SWidget> GetCommonMapsDropdown();

private:	
	FName EntryName;
	FText MenuName;
	FText MenuTooltip;
	FName IconStyle;

	FMapPicker_GetMaps MapGetter;
};


