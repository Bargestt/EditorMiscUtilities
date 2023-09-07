// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#include "EditorMiscUtilitiesModule.h"
#include "Modules/ModuleManager.h"

#include "EditorMiscUtilitiesSettings.h"
#include "MapPickerMenu.h"
#include "EasyThumbnailRenderer.h"


DEFINE_LOG_CATEGORY(LogEditorMiscUtilities);


#define LOCTEXT_NAMESPACE "EditorMiscUtilities"




class FEditorMiscUtilitiesModule : public IEditorMiscUtilitiesModule
{
public:
	static const TArray<FSoftObjectPath>& GetCommonMaps()
	{
		return GetDefault<UEditorMiscUtilities>()->CommonEditorMaps;
	}

    virtual void StartupModule() override
    {
		const UEditorMiscUtilities* Settings = GetDefault<UEditorMiscUtilities>();

		CommonMaps = FMapPickerMenu::Create(TEXT("CommonMapOptions"), FMapPicker_GetMaps::CreateStatic(&FEditorMiscUtilitiesModule::GetCommonMaps));


		UThumbnailManager& ThumbnailManager = UThumbnailManager::Get();
		for (const TPair<FSoftClassPath, FAssetThumbnailSettings>& Thumbnail : Settings->AssetThumbnails)
		{
			if (UEasyThumbnailRenderer::TryRegisterForClass(ThumbnailManager, Thumbnail.Key, Thumbnail.Value))
			{
				RegisteredThumbnails.Add(Thumbnail.Key.ResolveClass());
			}						
		}
  
  
		FCoreDelegates::OnFEngineLoopInitComplete.AddLambda([]()
		{
			TArray<FSoftClassPath> HideClasses = GetDefault<UEditorMiscUtilities>()->HideClasses;

			for (const FSoftClassPath& ClassPath : HideClasses)
			{	
				if (!ClassPath.IsValid())
				{
					continue;
				}

				UClass* Class = ClassPath.ResolveClass();
				if (Class == nullptr)
				{
					Class = ClassPath.TryLoadClass<UClass>();
				}
				if (Class)
				{
					EnumAddFlags(Class->ClassFlags, CLASS_Hidden);
				}
			}
		});  
  
	}

    virtual void ShutdownModule() override
    {	
		if (UThumbnailManager* ThumbnailManager = UThumbnailManager::TryGet())
		{
			for (TWeakObjectPtr<UClass>& WeakAssetClass : RegisteredThumbnails)
			{
				if (UClass* AssetClass = WeakAssetClass.Get())
				{
					ThumbnailManager->UnregisterCustomRenderer(AssetClass);
				}				
			}
		}	
		RegisteredThumbnails.Empty();
		CommonMaps.Reset();
    }

private:

	TSharedPtr<FMapPickerMenu> CommonMaps;

	TArray<TWeakObjectPtr<UClass>> RegisteredThumbnails;
};


IMPLEMENT_MODULE(FEditorMiscUtilitiesModule, EditorMiscUtilities);

#undef LOCTEXT_NAMESPACE
