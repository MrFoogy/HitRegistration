// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegGauntletController.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "Async/Async.h"

void UHitRegGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegGauntletController started"));
}

void UHitRegGauntletController::StartTesting()
{
	UseRollback = false;

	FString UseRollbackParam;
	if (FParse::Value(FCommandLine::Get(), TEXT("Rollback"), UseRollbackParam)) {
		UseRollbackParam = UseRollbackParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT(""));
	}
	if (UseRollbackParam == "True" || UseRollbackParam == "true") {
		UseRollback = true;
	}
	/*
	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegGauntletController::StopTesting, TestDuration + ShutDownTime, false);
	*/
}

void UHitRegGauntletController::OnTick(float DeltaTime)
{
    if (!Initialized && GetWorld() != NULL) {
		FTimerHandle dummy;
		GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegGauntletController::StopTesting, TestDuration, false);

		AFPSTemplateGameMode* GameMode = (AFPSTemplateGameMode*)GetWorld()->GetAuthGameMode();
		if (GameMode != NULL) {
			GameMode->ShouldUseRollback = UseRollback;
		}

		/*
		FPacketSimulationSettings CustomSettings;
		CustomSettings.PktIncomingLagMin = 300;
		CustomSettings.PktIncomingLagMax = 300;
		GetWorld()->GetNetDriver()->SetPacketSimulationSettings(CustomSettings);
		*/

		Initialized = true;
	}
}

void UHitRegGauntletController::StopTesting()
{
	EndTest(0);
}
