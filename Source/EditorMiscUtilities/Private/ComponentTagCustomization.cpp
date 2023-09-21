// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "ComponentTagCustomization.h"
#include <DetailWidgetRow.h>
#include <IDetailChildrenBuilder.h>
#include <IPropertyUtilities.h>

#include <PropertyCustomizationHelpers.h>
#include "EditorMiscUtilitiesModule.h"

#include "Components/ActorComponent.h"
#include "EditorMiscUtilitiesSettings.h"


#define LOCTEXT_NAMESPACE "ComponentTagCustomization"

void FActorComponentTagsCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TagsPropertyHandle = PropertyHandle;
	Utils = CustomizationUtils.GetPropertyUtilities();

	FText DisplayText;
	PropertyHandle->GetValueAsDisplayText(DisplayText);
	
	HeaderRow
	.NameContent()
	.HAlign(HAlign_Fill)
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(175)
	.MaxDesiredWidth(4096)
	[	
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 5, 0)
		[
			SNew(SComboButton)
			.ButtonStyle(FAppStyle::Get(), "SimpleButton")
			.HasDownArrow(false)			
			.ToolTipText(LOCTEXT("PickComponentTag", "Pick Component Tag"))
			.OnGetMenuContent(this, &FActorComponentTagsCustomization::GetComponentTagOptions)
			.ButtonContent()
			[
				SNew(SImage)
				.Image(FAppStyle::Get().GetBrush("Icons.BrowseContent"))
				.ColorAndOpacity(FSlateColor::UseForeground())
			]
		]
		+ SHorizontalBox::Slot()
		[
			PropertyHandle->CreatePropertyValueWidget(false)
		]
	];
}

void FActorComponentTagsCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const bool bTagsEditable = true;

	uint32 NumChildren = 0;
	PropertyHandle->GetNumChildren(NumChildren);
	for (uint32 Index = 0; Index < NumChildren; Index++)
	{
		ChildBuilder.AddProperty(PropertyHandle->GetChildHandle(Index).ToSharedRef());
	}
}

TSharedRef<SWidget> FActorComponentTagsCustomization::GetComponentTagOptions()
{
	FMenuBuilder MenuBuilder(true, nullptr);
	
	const UClass* ComponentClass = TagsPropertyHandle->GetOuterBaseClass();

	const UEditorMiscUtilities* Settings = GetDefault<UEditorMiscUtilities>();

	TArray<FActorComponentTagOptionInfo> Options = Settings->GetCommonActorComponentTagOptions(nullptr, ComponentClass);

	TMap<FString, TArray<FActorComponentTagOptionInfo>> Categories;
	for (const FActorComponentTagOptionInfo& Option : Options)
	{
		TArray<FActorComponentTagOptionInfo>& Arr = Categories.FindOrAdd(Option.Category);

		if (!Arr.ContainsByPredicate([&Option](const FActorComponentTagOptionInfo& Info) { return Info.Name == Option.Name; }))
		{
			Arr.Add(Option);
		}		
	}
	Categories.KeyStableSort([](const FString& A, const FString& B) { return A < B; });
	
	for (const auto& Pair : Categories)
	{
		const FString CategoryName = Pair.Key;

		MenuBuilder.BeginSection(*CategoryName, FText::FromString(CategoryName));

		for (const FActorComponentTagOptionInfo& Option : Pair.Value)
		{			
			MenuBuilder.AddMenuEntry(
				FText::FromName(Option.Name),
				FText::FromString(Option.Description),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &FActorComponentTagsCustomization::AddTag, Option.Name),
					FCanExecuteAction(),
					FIsActionChecked(),
					FIsActionButtonVisible()
				)
			);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}


void FActorComponentTagsCustomization::AddTag(FName Tag)
{
	if (TagsPropertyHandle.IsValid())
	{
		TSharedPtr<IPropertyHandleArray> AsArray = TagsPropertyHandle->AsArray();
		if (AsArray->AddItem() == FPropertyAccess::Success)
		{
			uint32 NumElements;
			if (AsArray->GetNumElements(NumElements) == FPropertyAccess::Success && NumElements > 0)
			{
				AsArray->GetElement(NumElements - 1)->SetValue(Tag);			
				
				if (Utils.IsValid())
				{
					Utils->ForceRefresh();
				}				
			}			
		}
	}
}

#undef LOCTEXT_NAMESPACE

