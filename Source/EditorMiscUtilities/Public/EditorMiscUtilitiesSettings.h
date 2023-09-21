// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "EditorMiscUtilitiesSettings.generated.h"


/** Ensure identical to EThumbnailRenderFrequency */
UENUM()
enum class EAssetThumbnailUpdateFrequence : uint8
{	
	/** Always render when requested, used for assets needing live animated thumbnails like materials */
	Realtime,
	/** Render whenever a property has changed on request */
	OnPropertyChange,
	/** Render only on asset save */
	OnAssetSave,
	/** Render thumbnail only once */
	Once,
};




/**  */
USTRUCT()
struct FAssetThumbnailSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FName PropertyOrFunction;

	UPROPERTY(EditAnywhere)
	EAssetThumbnailUpdateFrequence UpdateFrequency;	

	/** Use checker texture instead of filled background */
	UPROPERTY(EditAnywhere)
	bool bDrawChecker;

	UPROPERTY(EditAnywhere)
	int32 CheckerDensity;

	UPROPERTY(EditAnywhere)
	FLinearColor BackgroundColor;

	FAssetThumbnailSettings()
		: PropertyOrFunction()
		, UpdateFrequency(EAssetThumbnailUpdateFrequence::OnAssetSave)
		, bDrawChecker(false)
		, CheckerDensity(8)
		, BackgroundColor(FLinearColor(0.010330f, 0.010330f, 0.010330f))
	{
	}
};

/**  */
USTRUCT()
struct FActorComponentComponentTagOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TMap<FName, FString> ComponentTags;
};

struct EDITORMISCUTILITIES_API FActorComponentTagOptionInfo
{
	FName Name;

	FString Category;

	FString Description;

	FActorComponentTagOptionInfo()
	{ }

	FActorComponentTagOptionInfo(const FName& InName, const FString& InCategory = TEXT(""), const FString& InDescription = TEXT(""))
		: Name(InName)
		, Category(InCategory)
		, Description(InDescription)
	{ }
};
DECLARE_DELEGATE_RetVal_TwoParams(TArray<FActorComponentTagOptionInfo>, FGetActorComponentTagOptions, const UClass* /*ActorClass*/, const UClass* /*ComponentClass*/);




/**
 * 
 */
UCLASS(Config = EditorPerProjectUserSettings, defaultconfig, meta = (DisplayName = "Editor Misc Utilities"))
class EDITORMISCUTILITIES_API UEditorMiscUtilities : public UDeveloperSettings
{
	GENERATED_BODY()

	virtual FName GetContainerName() const override { return TEXT("Project"); }
	virtual FName GetCategoryName() const override { return TEXT("Editor"); }

public:
	/** A list of common maps that will be accessible via the editor toolbar */
	UPROPERTY(config, EditAnywhere, Category = "Editor", meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> CommonEditorMaps;



	/** Asset thumbnails. Restart required to apply changes */
	UPROPERTY(config, EditAnywhere, Category = "Editor", meta = (ConfigRestartRequired = true))
	TMap<FSoftClassPath, FAssetThumbnailSettings> AssetThumbnails;



	/** Mark these classes as hidden. Use as last resort to hide classes in pickers */
	UPROPERTY(config, EditAnywhere, Category = "Editor", meta = (ConfigRestartRequired = true))
	TArray<FSoftClassPath> HideClasses;




	/** Show tag picker menu in ActorComponent.ComponentTags property. Requires restart to enable */
	UPROPERTY(config, EditAnywhere, Category = "Actor Component Tags", meta = (ConfigRestartRequired = true))
	bool bShowActorComponentTagPicker;

	UPROPERTY(config, EditAnywhere, Category = "Actor Component Tags", meta = (AllowAbstract = true, EditCondition = "bShowActorComponentTagPicker"))
	TMap<TSoftClassPtr<UActorComponent>, FActorComponentComponentTagOptions> ActorComponentTags;

public:
	FGetActorComponentTagOptions GetActorComponentTagOptionsOverride;

	TArray<FActorComponentTagOptionInfo> GetCommonActorComponentTagOptions(const UClass* ActorClass, const UClass* ComponentClass) const;


};
