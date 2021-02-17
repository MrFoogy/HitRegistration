// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegGauntletController.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Async/Async.h"

void UHitRegGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegGauntletController started"));
}

void UHitRegGauntletController::StartTesting()
{
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
        Initialized = true;
    }
}

void UHitRegGauntletController::StopTesting()
{
    EndTest(0);
}
