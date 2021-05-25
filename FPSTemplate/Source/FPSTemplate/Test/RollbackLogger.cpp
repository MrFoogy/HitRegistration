// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/RollbackLogger.h"
#include "GauntletModule.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"

FRollbackLogger::FRollbackLogger()
{
}

FRollbackLogger::~FRollbackLogger()
{
}

void FRollbackLogger::CreateLogFile(FString FileName) {
	if (FileName.IsEmpty()) {
		FDateTime DateTime = FDateTime::Now();
		FString TimeString = DateTime.ToString();
		DiscrepancyFilePath = FString::Printf(TEXT("D:/Exjobb-Jonathan/HitRegistration/FPSTemplate/DebugLogs/%s-DiscrepancyLog.txt"), *TimeString);
		OptimalFudgeFilePath = FString::Printf(TEXT("D:/Exjobb-Jonathan/HitRegistration/FPSTemplate/DebugLogs/%s-FudgeLog.txt"), *TimeString);
	}
	else {
		DiscrepancyFilePath = FString::Printf(TEXT("%s-DiscrepancyLog.txt"), *FileName);
		OptimalFudgeFilePath = FString::Printf(TEXT("%s-FudgeLog.txt"), *FileName);
	}
}

void FRollbackLogger::WriteString(FString* Str, FString* FilePath) {
	FFileHelper::SaveStringToFile(*Str, **FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	//FFileHelper::SaveStringToFile(Str, *FilePath);
}

void FRollbackLogger::DumpLogFile() 
{
	UE_LOG(LogTemp, Warning, TEXT("Log: %s"), *DiscrepancyLogString);
	if (DiscrepancyLogString.Len() > 0) {
		WriteString(&DiscrepancyLogString, &DiscrepancyFilePath);
		UE_LOG(LogGauntlet, Display, TEXT("Written disc log!"));
	}
	if (OptimalFudgeLogString.Len() > 0) {
		WriteString(&OptimalFudgeLogString, &OptimalFudgeFilePath);
		UE_LOG(LogGauntlet, Display, TEXT("Written fudge log!"));
		UE_LOG(LogGauntlet, Display, TEXT("Written to %s"), *OptimalFudgeFilePath);
	}
}

void FRollbackLogger::LogDiscrepancy(float Time, float RandomHitPrecision, RepAnimationSnapshot* LocalSnapshot, RepAnimationSnapshot* RollbackSnapshot)
{
	DiscrepancyLogString += FString::Printf(TEXT("Time: %f\n"), Time);
	DiscrepancyLogString += FString::Printf(TEXT("Precision: %f\n"), RandomHitPrecision);
	for (auto KV : LocalSnapshot->GetShapeTransforms()) {
		physx::PxShape* Shape = KV.Key;
		physx::PxTransform LocalTransform = LocalSnapshot->GetShapeTransforms()[Shape];
		physx::PxTransform RollbackTransform = RollbackSnapshot->GetShapeTransforms()[Shape];
		float AngleDiff = LocalTransform.q.getAngle(RollbackTransform.q);
		AngleDiff = FMath::Min(AngleDiff, 2.0f * UKismetMathLibrary::GetPI() - AngleDiff);
		float DistDiff = (RollbackTransform.p - LocalTransform.p).magnitude();
		DiscrepancyLogString += FString::Printf(TEXT("Angle: %f\n"), AngleDiff);
		DiscrepancyLogString += FString::Printf(TEXT("Distance: %f\n"), DistDiff);
	}
}

void FRollbackLogger::LogOptimalFudge(float Time, float OptimalFudge, float OptimalAngDiff, float OptimalPosDiff, float OptimalHitRate, float TransmissionTime)
{
	OptimalFudgeLogString += FString::Printf(TEXT("Time: %f\n"), Time);
	OptimalFudgeLogString += FString::Printf(TEXT("Transmission: %f\n"), TransmissionTime);
	OptimalFudgeLogString += FString::Printf(TEXT("OptimalFudge: %f\n"), OptimalFudge);
	OptimalFudgeLogString += FString::Printf(TEXT("Angle: %f\n"), OptimalAngDiff);
	OptimalFudgeLogString += FString::Printf(TEXT("Distance: %f\n"), OptimalPosDiff);
	OptimalFudgeLogString += FString::Printf(TEXT("Precision: %f\n"), OptimalHitRate);
}
