// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "FPSTemplateGameMode.h"
#include "FPSTemplateHUD.h"
#include "FPSTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

AFPSTemplateGameMode::AFPSTemplateGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSTemplateHUD::StaticClass();
}

void AFPSTemplateGameMode::StartPlay()
{
	Super::StartPlay();
	RepWorldTimelines.ClearWorldTimelines();
}

RepMovementTimeline& AFPSTemplateGameMode::GetRepMovementTimeline(IRepMovable* RepMovable)
{
	if (!RepWorldTimelines.HasTimeline(RepMovable)) {
		RepWorldTimelines.InitializeTimeline(RepMovable);
	}
	return RepWorldTimelines.GetMovementTimeline(RepMovable);
}

RepAnimationTimeline& AFPSTemplateGameMode::GetRepAnimationTimeline(IRepMovable* RepMovable)
{
	if (!RepWorldTimelines.HasTimeline(RepMovable)) {
		RepWorldTimelines.InitializeTimeline(RepMovable);
	}
	return RepWorldTimelines.GetAnimationTimeline(RepMovable);
}

URepWorldTimelines& AFPSTemplateGameMode::GetRepWorldTimelines()
{
	return RepWorldTimelines;
}
