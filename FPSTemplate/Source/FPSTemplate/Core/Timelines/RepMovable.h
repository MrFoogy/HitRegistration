// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UObject/Interface.h"
#include "CoreMinimal.h"
#include "Core/Timelines/RepSnapshot.h"
#include "Core/Timelines/RepAnimationSnapshot.h"
#include "RepMovable.generated.h"

/**
 * 
 */
UINTERFACE()
class FPSTEMPLATE_API URepMovable : public UInterface
{
	GENERATED_BODY()
};

class IRepMovable
{
	GENERATED_BODY()

public:
	/** Add interface function declarations here */
	virtual void PrepareRollback() = 0;
	virtual void RollbackMovement(const RepSnapshot& RepSnapshot);
	virtual void RollbackAnimation(RepAnimationSnapshot& RepSnapshot);
	virtual void ResetRollback() = 0;
};
