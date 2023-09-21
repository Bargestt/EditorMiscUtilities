// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include <PropertyEditorModule.h>

class IPropertyHandle;


class FActorComponentTagsCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FActorComponentTagsCustomization);
	}

	//~ Begin IPropertyTypeCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	//~ End IPropertyTypeCustomization Interface	

	class FPropertyTypeIdentifier : public IPropertyTypeIdentifier
	{
	public:
		virtual bool IsPropertyTypeCustomized(const IPropertyHandle& PropertyHandle) const override
		{
			const FProperty* Property = PropertyHandle.GetProperty();
			if (Property && Property->GetFName() == GET_MEMBER_NAME_CHECKED(UActorComponent, ComponentTags))
			{
				UClass* OwnerClass = Property->GetOwnerClass();
				return OwnerClass && OwnerClass->IsChildOf(UActorComponent::StaticClass());
			}
			return false;
		}
	};


private:
	TSharedRef<SWidget> GetComponentTagOptions();
	void AddTag(FName Tag);

	TSharedPtr<IPropertyHandle> TagsPropertyHandle;
	TSharedPtr<IPropertyUtilities> Utils;

};