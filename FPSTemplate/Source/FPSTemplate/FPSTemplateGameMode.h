// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RepWorldTimelines.h"
#include "RepMovementTimeline.h"
#include "RepAnimationTimeline.h"
#include "FPSTemplateGameMode.generated.h"

UCLASS(minimalapi)
class AFPSTemplateGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSTemplateGameMode();
	virtual void StartPlay() override;
	virtual RepMovementTimeline& GetRepMovementTimeline(IRepMovable* RepMovable);
	virtual RepAnimationTimeline& GetRepAnimationTimeline(IRepMovable* RepMovable);
	virtual URepWorldTimelines& GetRepWorldTimelines();

protected:
	URepWorldTimelines RepWorldTimelines;
};



