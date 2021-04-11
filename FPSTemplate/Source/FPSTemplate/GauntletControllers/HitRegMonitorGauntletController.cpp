// Fill out your copyright notice in the Description page of Project Settings.

#include "HitRegMonitorGauntletController.h"
#include "Test/RollbackDebugComponent.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "TimerManager.h"
#include "Async/Async.h"
#include "GameFramework/GameUserSettings.h"

void UHitRegMonitorGauntletController::OnInit()
{
    UE_LOG(LogGauntlet, Display, TEXT("HitRegMonitorGauntletController started"));

    /*
    */
}

void UHitRegMonitorGauntletController::PrepareTest()
{
	FTimerHandle dummy;

    bool UseInterpolation = false;
    bool MonitorDiscrepancy = true;
    FString InterpolationParam;
    FString MonitorTypeParam;
    FString LogFileNameParam;

    if (FParse::Value(FCommandLine::Get(), TEXT("Interpolation"), InterpolationParam)) {
        InterpolationParam = InterpolationParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT("")); 
    }
    if (InterpolationParam == "True" || InterpolationParam == "true") {
        UseInterpolation = true;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("MonitorType"), MonitorTypeParam)) {
        MonitorTypeParam = MonitorTypeParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT(""));
    }
    if (MonitorTypeParam == "Fudge" || MonitorTypeParam == "fudge") {
        MonitorDiscrepancy = false;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("LogFileName"), LogFileNameParam)) {
        LogFileNameParam = LogFileNameParam.Replace(TEXT("="), TEXT("")).Replace(TEXT("\""), TEXT(""));
    }

	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::StartTesting, PrepareTime, false);
    APlayerController* PlayerController = GetFirstPlayerController();
    Character = (AFPSTemplateCharacter*)PlayerController->GetPawn();
    Character->RollbackDebug->DebugPrepareMonitoringTest(UseInterpolation, LogFileNameParam);
    Character->RollbackDebug->IsMonitoringDiscrepancy = MonitorDiscrepancy;
}

void UHitRegMonitorGauntletController::StartTesting()
{
    Character->RollbackDebug->DebugStartMonitoring();
    FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::RecordResults, TestDuration, false);

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
    Character->RollbackDebug->SaveRollbackLog();
    UE_LOG(LogGauntlet, Display, TEXT("Written log!"));
	FTimerHandle dummy;
	GetWorld()->GetTimerManager().SetTimer(dummy, this, &UHitRegMonitorGauntletController::StopTesting, ShutDownTime, false);
}

void UHitRegMonitorGauntletController::StopTesting()
{
    EndTest(0);
}
