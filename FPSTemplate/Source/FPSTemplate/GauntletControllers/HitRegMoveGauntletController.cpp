// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegMoveGauntletController.h"
#include "Test/RollbackDebugComponent.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Async/Async.h"
#include "GameFramework/GameUserSettings.h"

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
    /*
    Character->RollbackDebug->DebugPrepareMonitoredTest();
    */
    FString MovementParam;

    //string
    if (FParse::Value(FCommandLine::Get(), TEXT("MovementType"), MovementParam)) {
        MovementParam = MovementParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT(""));
    }
    MovementType = AutomatedMovementType::None;
    if (MovementParam == "Straight") {
        MovementType = AutomatedMovementType::MoveStraight;
    }
    if (MovementParam == "Alternate") {
        MovementType = AutomatedMovementType::Alternate;
    }
}

void UHitRegMoveGauntletController::StartTesting()
{
    Character->RollbackDebug->StartDebugMovement(MovementType);
    FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMoveGauntletController::StopTesting, TestDuration + ShutDownTime, false);
    /*
    UGameUserSettings* GameUserSettings = GEngine->GetGameUserSettings();
    if (GameUserSettings) {
        GameUserSettings->SetFrameRateLimit(10.f);
        GameUserSettings->ConfirmVideoMode();
        UE_LOG(LogGauntlet, Display, TEXT("SET FPS!"));
    }
    else {
        UE_LOG(LogGauntlet, Display, TEXT("DON'T SET FPS!"));
    }
    */
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
