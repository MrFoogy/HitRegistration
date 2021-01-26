// Fill out your copyright notice in the Description page of Project Settings.


#include "RollbackLogger.h"
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

void FRollbackLogger::CreateLogFile() {
	FDateTime DateTime = FDateTime::Now();
	FString TimeString = DateTime.ToString();
	//FilePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()) + FString::Printf(TEXT("/DebugLogs/%s-Log.txt"), *TimeString);
	FilePath = FString::Printf(TEXT("D:/Exjobb-Jonathan/HitRegistration/FPSTemplate/DebugLogs/%s-Log.txt"), *TimeString);
}

void FRollbackLogger::WriteString(FString Str) {
	FFileHelper::SaveStringToFile(Str, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
	//FFileHelper::SaveStringToFile(Str, *FilePath);
}

void FRollbackLogger::DumpLogFile() 
{
	UE_LOG(LogTemp, Warning, TEXT("Log: %s"), *LogString);
	WriteString(LogString);
}

void FRollbackLogger::LogDiscrepancy(float Time, float RandomHitPrecision, RepAnimationSnapshot* LocalSnapshot, RepAnimationSnapshot* RollbackSnapshot)
{
	LogString += FString::Printf(TEXT("Time: %f\n"), Time);
	LogString += FString::Printf(TEXT("Precision: %f\n"), RandomHitPrecision);
	for (auto KV : LocalSnapshot->GetShapeTransforms()) {
		physx::PxShape* Shape = KV.Key;
		physx::PxTransform LocalTransform = LocalSnapshot->GetShapeTransforms()[Shape];
		physx::PxTransform RollbackTransform = RollbackSnapshot->GetShapeTransforms()[Shape];
		float AngleDiff = LocalTransform.q.getAngle(RollbackTransform.q);
		AngleDiff = FMath::Min(AngleDiff, 2.0f * UKismetMathLibrary::GetPI() - AngleDiff);
		float DistDiff = (RollbackTransform.p - LocalTransform.p).magnitude();
		LogString += FString::Printf(TEXT("Angle: %f\n"), AngleDiff);
		LogString += FString::Printf(TEXT("Distance: %f\n"), DistDiff);
	}
}
