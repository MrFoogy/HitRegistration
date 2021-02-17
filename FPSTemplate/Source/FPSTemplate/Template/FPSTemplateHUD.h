// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Runtime/Engine/Public/EngineGlobals.h"
#include "FPSTemplateHUD.generated.h"

UCLASS()
class AFPSTemplateHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSTemplateHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

