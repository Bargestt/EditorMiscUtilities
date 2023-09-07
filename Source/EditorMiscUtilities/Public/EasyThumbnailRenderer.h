// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ThumbnailRendering/ThumbnailRenderer.h"
#include "EditorMiscUtilitiesSettings.h"
#include "EasyThumbnailRenderer.generated.h"


/**
 * 
 */
UCLASS()
class UEasyThumbnailRenderer : public UThumbnailRenderer
{
	GENERATED_BODY()
private:
	// Little kludge to send initialization data to new instance
	struct FEasyThumbnailRendererOptions
	{
		bool bWaiting;

		TObjectPtr<UClass> TargetClass;

		TWeakFieldPtr<FProperty> ThumbnailProperty;
		TWeakObjectPtr<UFunction> ThumbnailFunction;

		FAssetThumbnailSettings Settings;
	};
	static FEasyThumbnailRendererOptions InitializationData;


public:
	UPROPERTY()
	TObjectPtr<UClass> TargetClass;

	TWeakFieldPtr<FProperty> ThumbnailProperty;
	TWeakObjectPtr<UFunction> ThumbnailFunction;

	FAssetThumbnailSettings Settings;

public:
	UEasyThumbnailRenderer();
	static bool TryRegisterForClass(class UThumbnailManager& Manager, FSoftClassPath ClassPath, const FAssetThumbnailSettings& Settings);


	// Begin UThumbnailRenderer Object
	virtual EThumbnailRenderFrequency GetThumbnailRenderFrequency(UObject* Object) const override;
	virtual bool CanVisualizeAsset(UObject* Object) override;
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily) override;
	// End UThumbnailRenderer Object
};
