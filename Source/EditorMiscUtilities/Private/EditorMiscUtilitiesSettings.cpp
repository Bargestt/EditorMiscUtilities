// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.


#include "EditorMiscUtilitiesSettings.h"


TArray<FActorComponentTagOptionInfo> UEditorMiscUtilities::GetCommonActorComponentTagOptions(const UClass* ActorClass, const UClass* ComponentClass) const
{
	if (GetActorComponentTagOptionsOverride.IsBound())
	{
		return GetActorComponentTagOptionsOverride.Execute(ActorClass, ComponentClass);
	}

	TArray<FActorComponentTagOptionInfo> Options;
	for (const auto& Pair : ActorComponentTags)
	{
		const TSoftClassPtr<UActorComponent>& ClassFilter = Pair.Key;
		const FActorComponentComponentTagOptions& TagOptions = Pair.Value;

		const UClass* ResolvedClass = ClassFilter.Get();
		if (ClassFilter.IsNull() || !ComponentClass || ComponentClass->IsChildOf(ResolvedClass))
		{	
			FString Category = ResolvedClass ? ResolvedClass->GetDisplayNameText().ToString() : TEXT("");

			for (const auto& TagInfo : TagOptions.ComponentTags)
			{
				Options.Add(FActorComponentTagOptionInfo(TagInfo.Key, Category, TagInfo.Value));
			}
		}
	}

	return Options;
}
