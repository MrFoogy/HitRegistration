// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"
#include "FPSTemplateGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPSTEMPLATE_API UFPSTemplateGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFPSTemplateGameInstance(const FObjectInitializer & ObjectInitializer);

protected:
	TSubclassOf<class UUserWidget> RollbackTimelineUIClass;
	
};
