// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Core/FPSTemplateGameMode.h"
#include "Template/FPSTemplateHUD.h"
#include "Core/FPSTemplateCharacter.h"
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

RepTimeline<RepSnapshot>& AFPSTemplateGameMode::GetRepMovementTimeline(IRepMovable* RepMovable)
{
	if (!RepWorldTimelines.HasTimeline(RepMovable)) {
		RepWorldTimelines.CreateMovementTimeline(RepMovable);
	}
	return RepWorldTimelines.GetMovementTimeline(RepMovable);
}

RepTimeline<RepAnimationSnapshot>& AFPSTemplateGameMode::GetRepAnimationTimeline(IRepMovable* RepMovable)
{
	if (!RepWorldTimelines.HasTimeline(RepMovable)) {
		RepWorldTimelines.CreateAnimationTimeline(RepMovable);
	}
	return RepWorldTimelines.GetAnimationTimeline(RepMovable);
}

URepWorldTimelines& AFPSTemplateGameMode::GetRepWorldTimelines()
{
	return RepWorldTimelines;
}
