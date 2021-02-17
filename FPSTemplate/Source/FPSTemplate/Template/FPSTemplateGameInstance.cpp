// Fill out your copyright notice in the Description page of Project Settings.


#include "Template/FPSTemplateGameInstance.h"

// PlayGroundGameInstance Constructor
UFPSTemplateGameInstance::UFPSTemplateGameInstance(const FObjectInitializer & ObjectInitializer)
{
	// Find the Widget and assigned to InGameUIClass
	static ConstructorHelpers::FClassFinder<UUserWidget> RBTUIClass(TEXT("/Game/UI/RollbackTimeline"));

	if (RBTUIClass.Class != nullptr) {
		RollbackTimelineUIClass = RBTUIClass.Class;
	}
}
