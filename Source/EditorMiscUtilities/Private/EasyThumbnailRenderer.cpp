// Copyright (C) Vasily Bulgakov. 2023. All Rights Reserved.


#include "EasyThumbnailRenderer.h"
#include "EditorMiscUtilitiesModule.h"

#include <ThumbnailRendering/ThumbnailManager.h>
#include <CanvasItem.h>
#include <CanvasTypes.h>



UEasyThumbnailRenderer::FEasyThumbnailRendererOptions UEasyThumbnailRenderer::InitializationData;

UEasyThumbnailRenderer::UEasyThumbnailRenderer()
{	
	if(!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (InitializationData.bWaiting)
		{
			InitializationData.bWaiting = false;

			TargetClass = InitializationData.TargetClass;
			ThumbnailProperty = InitializationData.ThumbnailProperty;
			ThumbnailFunction = InitializationData.ThumbnailFunction;
			Settings = InitializationData.Settings;
		}
		else
		{
			UE_LOG(LogEditorMiscUtilities, Error, TEXT("Failed to register EasyThumbnailRenderer: No initialization data"));
		}
	}
}

bool UEasyThumbnailRenderer::TryRegisterForClass(class UThumbnailManager& Manager, FSoftClassPath ClassPath, const FAssetThumbnailSettings& Settings)
{
	if (!ClassPath.IsValid() || Settings.PropertyOrFunction.IsNone())
	{
		UE_LOG(LogEditorMiscUtilities, Error, TEXT("Failed to register EasyThumbnailRenderer: Invalid settings"));
		return false;
	}
	
	UClass* AssetClass = ClassPath.TryLoadClass<UObject>();
	if (AssetClass == nullptr)
	{
		UE_LOG(LogEditorMiscUtilities, Error, TEXT("Failed to register EasyThumbnailRenderer: Invalid class"));
		return false;
	}


	FProperty* SourceProperty = nullptr;
	UFunction* SourceFunction = nullptr;
	{
		FProperty* Property = AssetClass->FindPropertyByName(Settings.PropertyOrFunction);
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (StructProperty->Struct == FSlateBrush::StaticStruct())
			{
				SourceProperty = Property;
			}
		}

		if (SourceProperty == nullptr)
		{
			UFunction* Function = AssetClass->FindFunctionByName(Settings.PropertyOrFunction);
			if (Function)
			{
				bool bHasInputArguments = false;
				int32 ReturnArguments = 0;
				FProperty* ReturnType = nullptr;
				for (TFieldIterator<FProperty> It(Function); It; ++It)
				{
					FProperty* Prop = *It;
					if (Prop->HasAnyPropertyFlags(CPF_Parm) && !Prop->HasAnyPropertyFlags(CPF_OutParm))
					{
						bHasInputArguments = true;
						break;
					}
					else if (Prop->HasAllPropertyFlags(CPF_Parm | CPF_OutParm))
					{
						ReturnArguments++;
						ReturnType = Prop;
					}
				}

				if (!bHasInputArguments && ReturnArguments == 1)
				{
					if (FStructProperty* StructProperty = CastField<FStructProperty>(ReturnType))
					{
						if (StructProperty->Struct == FSlateBrush::StaticStruct())
						{
							SourceFunction = Function;
						}
					}
				}
			}
		}
	}	

	if (SourceProperty == nullptr && SourceFunction == nullptr)
	{
		UE_LOG(LogEditorMiscUtilities, Error, TEXT("Failed to register EasyThumbnailRenderer for %s: PropertyOrFunction %s not found"), *GetNameSafe(AssetClass), *Settings.PropertyOrFunction.ToString());
		return false;
	}
	InitializationData.bWaiting = true;
	InitializationData.TargetClass = AssetClass;
	InitializationData.ThumbnailProperty = SourceProperty;
	InitializationData.ThumbnailFunction = SourceFunction;
	InitializationData.Settings = Settings;

	Manager.RegisterCustomRenderer(AssetClass, UEasyThumbnailRenderer::StaticClass());
	InitializationData.bWaiting = false;

	return true;
}

EThumbnailRenderFrequency UEasyThumbnailRenderer::GetThumbnailRenderFrequency(UObject* Object) const
{
	return static_cast<EThumbnailRenderFrequency>(Settings.UpdateFrequency);
}

bool UEasyThumbnailRenderer::CanVisualizeAsset(UObject* Object)
{	
	return TargetClass && Object->IsA(TargetClass) && (ThumbnailProperty.IsValid() || ThumbnailFunction.IsValid());
}

void UEasyThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget*, FCanvas* Canvas, bool bAdditionalViewFamily)
{		
	FSlateBrush Brush;

	if (ThumbnailProperty.IsValid())
	{
		FSlateBrush* ThumbnailBrushPtr = ThumbnailProperty->ContainerPtrToValuePtr<FSlateBrush>(Object);
		if (ThumbnailBrushPtr)
		{
			Brush = *ThumbnailBrushPtr;
		}
	}
	else if (ThumbnailFunction.IsValid())
	{
		FSlateBrush Value;
		Object->ProcessEvent(ThumbnailFunction.Get(), &Brush);
	}


	if (Brush.GetDrawType() == ESlateBrushDrawType::NoDrawType)
	{			
		return;
	}

	UTexture2D* Texture = Cast<UTexture2D>(Brush.GetResourceObject());

	// Draw the background checkboard pattern
	if (Settings.bDrawChecker)
	{	
		UTexture2D* Checker = UThumbnailManager::Get().CheckerboardTexture;
		Canvas->DrawTile(
			0.0f, 0.0f, Width, Height,							// Dimensions
			0.0f, 0.0f, Settings.CheckerDensity, Settings.CheckerDensity,			// UVs
			FLinearColor::White, Checker->GetResource());			// Tint & Texture
	}
	else
	{
		Canvas->DrawTile(
			0.0f, 0.0f, Width, Height,							
			0.0f, 0.0f, 1.0f, 1.0f,			
			Settings.BackgroundColor, nullptr);
	}

	if (Texture)
	{
		switch (Brush.DrawAs)
		{
		case ESlateBrushDrawType::Image:
		{
			FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->GetResource(), FVector2D(Width, Height), Brush.TintColor.GetSpecifiedColor());
			CanvasTile.BlendMode = SE_BLEND_Translucent;
			CanvasTile.Draw(Canvas);
		}
		break;
		case ESlateBrushDrawType::Border:
		{
			FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->GetResource(), FVector2D(Width, Height), Brush.TintColor.GetSpecifiedColor());
			CanvasTile.BlendMode = SE_BLEND_Translucent;
			CanvasTile.Draw(Canvas);
		}
		break;
		case ESlateBrushDrawType::RoundedBox:
		case ESlateBrushDrawType::Box:
		{
			float NaturalWidth = Texture->GetSurfaceWidth();
			float NaturalHeight = Texture->GetSurfaceHeight();

			float TopPx = FMath::Clamp<float>(NaturalHeight * Brush.Margin.Top, 0, Height);
			float BottomPx = FMath::Clamp<float>(NaturalHeight * Brush.Margin.Bottom, 0, Height);
			float VerticalCenterPx = FMath::Clamp<float>(Height - TopPx - BottomPx, 0, Height);
			float LeftPx = FMath::Clamp<float>(NaturalWidth * Brush.Margin.Left, 0, Width);
			float RightPx = FMath::Clamp<float>(NaturalWidth * Brush.Margin.Right, 0, Width);
			float HorizontalCenterPx = FMath::Clamp<float>(Width - LeftPx - RightPx, 0, Width);

			// Top-Left
			FVector2D TopLeftSize(LeftPx, TopPx);
			{
				FVector2D UV0(0, 0);
				FVector2D UV1(Brush.Margin.Left, Brush.Margin.Top);

				FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->GetResource(), TopLeftSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			// Bottom-Left
			FVector2D BottomLeftSize(LeftPx, BottomPx);
			{
				FVector2D UV0(0, 1 - Brush.Margin.Bottom);
				FVector2D UV1(Brush.Margin.Left, 1);

				FCanvasTileItem CanvasTile(FVector2D(X, Y + Height - BottomPx), Texture->GetResource(), BottomLeftSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			// Top-Right
			FVector2D TopRightSize(RightPx, TopPx);
			{
				FVector2D UV0(1 - Brush.Margin.Right, 0);
				FVector2D UV1(1, Brush.Margin.Top);

				FCanvasTileItem CanvasTile(FVector2D(X + Width - RightPx, Y), Texture->GetResource(), TopRightSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			// Bottom-Right
			FVector2D BottomRightSize(RightPx, BottomPx);
			{
				FVector2D UV0(1 - Brush.Margin.Right, 1 - Brush.Margin.Bottom);
				FVector2D UV1(1, 1);

				FCanvasTileItem CanvasTile(FVector2D(X + Width - RightPx, Y + Height - BottomPx), Texture->GetResource(), BottomRightSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			//-----------------------------------------------------------------------

			// Center-Vertical-Left
			FVector2D CenterVerticalLeftSize(LeftPx, VerticalCenterPx);
			{
				FVector2D UV0(0, Brush.Margin.Top);
				FVector2D UV1(Brush.Margin.Left, 1 - Brush.Margin.Bottom);

				FCanvasTileItem CanvasTile(FVector2D(X, Y + TopPx), Texture->GetResource(), CenterVerticalLeftSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			// Center-Vertical-Right
			FVector2D CenterVerticalRightSize(RightPx, VerticalCenterPx);
			{
				FVector2D UV0(1 - Brush.Margin.Right, Brush.Margin.Top);
				FVector2D UV1(1, 1 - Brush.Margin.Bottom);

				FCanvasTileItem CanvasTile(FVector2D(X + Width - RightPx, Y + TopPx), Texture->GetResource(), CenterVerticalRightSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			//-----------------------------------------------------------------------

			// Center-Horizontal-Top
			FVector2D CenterHorizontalTopSize(HorizontalCenterPx, TopPx);
			{
				FVector2D UV0(Brush.Margin.Left, 0);
				FVector2D UV1(1 - Brush.Margin.Right, Brush.Margin.Top);

				FCanvasTileItem CanvasTile(FVector2D(X + LeftPx, Y), Texture->GetResource(), CenterHorizontalTopSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			// Center-Horizontal-Bottom
			FVector2D CenterHorizontalBottomSize(HorizontalCenterPx, BottomPx);
			{
				FVector2D UV0(Brush.Margin.Left, 1 - Brush.Margin.Bottom);
				FVector2D UV1(1 - Brush.Margin.Right, 1);

				FCanvasTileItem CanvasTile(FVector2D(X + LeftPx, Y + Height - BottomPx), Texture->GetResource(), CenterHorizontalBottomSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}

			//-----------------------------------------------------------------------

			// Center
			FVector2D CenterSize(HorizontalCenterPx, VerticalCenterPx);
			{
				FVector2D UV0(Brush.Margin.Left, Brush.Margin.Top);
				FVector2D UV1(1 - Brush.Margin.Right, 1 - Brush.Margin.Bottom);

				FCanvasTileItem CanvasTile(FVector2D(X + LeftPx, Y + TopPx), Texture->GetResource(), CenterSize, UV0, UV1, Brush.TintColor.GetSpecifiedColor());
				CanvasTile.BlendMode = SE_BLEND_Translucent;
				CanvasTile.Draw(Canvas);
			}
		}
		break;
		case ESlateBrushDrawType::NoDrawType:
		{
			FCanvasTileItem CanvasTile(FVector2D(X, Y), Texture->GetResource(), FVector2D(Width, Height), Brush.TintColor.GetSpecifiedColor());
			CanvasTile.BlendMode = SE_BLEND_Translucent;
			CanvasTile.Draw(Canvas);
		}
		break;		
		default:

			check(false);
		}
	}
	
}
