// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PropertyEditorModule.h"

class FCustomizationBinder
{
public:
	FCustomizationBinder()
	{

	}

	~FCustomizationBinder()
	{
		UnregisterAll();
	}

	void RegisterProperty(FPropertyEditorModule& PropertyModule, FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate, TSharedPtr<IPropertyTypeIdentifier> Identifier = nullptr)
	{
		PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate, Identifier);
	}

	void RegisterClass(FPropertyEditorModule& PropertyModule, FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
	{
		PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
	}

	void UnregisterAll()
	{
		if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
		{
			for (const FRegisteredCustomization& Customization : PropertyCustomizations)
			{
				PropertyModule->UnregisterCustomPropertyTypeLayout(Customization.Name, Customization.Identifier);
			}

			for (const FName& Customization : ClassCustomizations)
			{
				PropertyModule->UnregisterCustomClassLayout(Customization);
			}
		}
		PropertyCustomizations.Empty();
		ClassCustomizations.Empty();
	}	

private:
	struct FRegisteredCustomization
	{
		FName Name;
		TSharedPtr<IPropertyTypeIdentifier> Identifier;
	};

	TArray<FRegisteredCustomization> PropertyCustomizations;

	TArray<FName> ClassCustomizations;
};
