// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/Timelines/RepAnimationSnapshot.h"

/**
 * 
 */
class FPSTEMPLATE_API FRollbackLogger
{
public:
	FRollbackLogger();
	~FRollbackLogger();

	void CreateLogFile(FString FileName);
	void WriteString(FString* Str, FString* FilePath);
	void LogDiscrepancy(float Time, float RandomHitPrecision, RepAnimationSnapshot* LocalSnapshot, RepAnimationSnapshot* RollbackSnapshot);
	void LogOptimalFudge(float Time, float OptimalFudge, float OptimalAngDiff, float OptimalPosDiff, float OptimalHitRate, float TransmissionTime);
	void DumpLogFile();

protected:
	FString DiscrepancyFilePath;
	FString OptimalFudgeFilePath;
	FString DiscrepancyLogString;
	FString OptimalFudgeLogString;
};
