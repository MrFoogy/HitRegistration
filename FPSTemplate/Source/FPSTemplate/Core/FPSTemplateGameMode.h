// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Core/Timelines/RepWorldTimelines.h"
#include "Core/Timelines/RepSnapshot.h"
#include "Core/Timelines/RepTimeline.h"
#include "Core/Timelines/RepAnimationSnapshot.h"
#include "FPSTemplateGameMode.generated.h"

UCLASS(minimalapi)
class AFPSTemplateGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFPSTemplateGameMode();
	virtual void StartPlay() override;
	virtual RepTimeline<RepSnapshot>& GetRepMovementTimeline(IRepMovable* RepMovable);
	virtual RepTimeline<RepAnimationSnapshot>& GetRepAnimationTimeline(IRepMovable* RepMovable);
	virtual URepWorldTimelines& GetRepWorldTimelines();

protected:
	URepWorldTimelines RepWorldTimelines;
};



