// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RepAnimationSnapshot.h"

/**
 * 
 */
class FPSTEMPLATE_API FRollbackLogger
{
public:
	FRollbackLogger();
	~FRollbackLogger();

	void CreateLogFile();
	void WriteString(FString Str);
	void LogDiscrepancy(float Time, float RandomHitPrecision, RepAnimationSnapshot* LocalSnapshot, RepAnimationSnapshot* RollbackSnapshot);
	void DumpLogFile();

protected:
	FString FilePath;
	FString LogString;
};
