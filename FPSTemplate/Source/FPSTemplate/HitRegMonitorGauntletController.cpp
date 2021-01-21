// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegMonitorGauntletController.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Async/Async.h"

void UHitRegMonitorGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegMonitorGauntletController started"));

    /*
    */
}

void UHitRegMonitorGauntletController::PrepareTest()
{
	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::StartTesting, PrepareTime, false);
    APlayerController* PlayerController = GetFirstPlayerController();
    Character = (AFPSTemplateCharacter*)PlayerController->GetPawn();
    Character->DebugPrepareMonitoringTest();
}

void UHitRegMonitorGauntletController::StartTesting()
{
    Character->DebugStartMonitoring();
    FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::RecordResults, TestDuration, false);
}

void UHitRegMonitorGauntletController::OnTick(float DeltaTime)
{
    //TODO: this is where you can put stuff that should happen on tick
    
    if (!Initialized && GetWorld() != NULL) {
		FTimerHandle dummy;
		GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::PrepareTest, SpinUpTime, false);
        Initialized = true;
    }
}

void UHitRegMonitorGauntletController::RecordResults()
{
    Character->DebugFindOtherPlayer()->SaveRollbackLog();
    UE_LOG(LogGauntlet, Display, TEXT("Written log!"));
	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::StopTesting, ShutDownTime, false);
}

void UHitRegMonitorGauntletController::StopTesting()
{
    EndTest(0);
}
