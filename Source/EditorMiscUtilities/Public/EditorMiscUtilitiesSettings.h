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
	UPROPERTY(config, EditAnywhere, Category = Utilities, meta = (AllowedClasses = "/Script/Engine.World"))
	TArray<FSoftObjectPath> CommonEditorMaps;


	/** Asset thumbnails. Restart required to apply changes */
	UPROPERTY(config, EditAnywhere, Category = Utilities, meta = (ConfigRestartRequired = true))
	TMap<FSoftClassPath, FAssetThumbnailSettings> AssetThumbnails;


	/** Mark these classes as hidden. Use as last resort to hide classes in pickers */
	UPROPERTY(config, EditAnywhere, Category = Utilities, meta = (ConfigRestartRequired = true))
	TArray<FSoftClassPath> HideClasses;
};
