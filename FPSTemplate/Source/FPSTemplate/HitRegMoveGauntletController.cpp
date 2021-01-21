// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegMoveGauntletController.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Async/Async.h"

void UHitRegMoveGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegMoveGauntletController started"));

    /*
    */
}

void UHitRegMoveGauntletController::PrepareTest()
{
    FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMoveGauntletController::StartTesting, PrepareTime, false);
    APlayerController* PlayerController = GetFirstPlayerController();
    Character = (AFPSTemplateCharacter*)PlayerController->GetPawn();
    Character->DebugPrepareMonitoredTest();
}

void UHitRegMoveGauntletController::StartTesting()
{
    Character->StartDebugMovement();
    FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMoveGauntletController::StopTesting, TestDuration + ShutDownTime, false);
}

void UHitRegMoveGauntletController::OnTick(float DeltaTime)
{
    //TODO: this is where you can put stuff that should happen on tick
    
    if (!Initialized && GetWorld() != NULL) {
		FTimerHandle dummy;
		GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMoveGauntletController::PrepareTest, SpinUpTime, false);
        Initialized = true;
    }
}

void UHitRegMoveGauntletController::StopTesting()
{
    EndTest(0);
}
