// Fill out your copyright notice in the Description page of Project Settings.


#include "Test/RollbackTimelineWidget.h"
#include "Test/RollbackDebugComponent.h"

bool URollbackTimelineWidget::Initialize()
{
	bool Res = UUserWidget::Initialize();

	TimelineSlider->OnValueChanged.AddDynamic(this, &URollbackTimelineWidget::OnTimelineSliderValueChanged);
	FudgeSlider->OnValueChanged.AddDynamic(this, &URollbackTimelineWidget::OnFudgeSliderValueChanged);
	TimelineSlider->OnMouseCaptureBegin.AddDynamic(this, &URollbackTimelineWidget::OnStartSliding);
	TimelineSlider->OnMouseCaptureEnd.AddDynamic(this, &URollbackTimelineWidget::OnStopSliding);

	InterpolateCheckbox->OnCheckStateChanged.AddDynamic(this, &URollbackTimelineWidget::OnInterpolateCheckboxChanged);

	RefreshButton->OnClicked.AddDynamic(this, &URollbackTimelineWidget::OnRefreshButtonPress);

	return Res;
}

void URollbackTimelineWidget::AssignTargetCharacter(AFPSTemplateCharacter* ControlledCharacter)
{
	//UE_LOG(LogTemp, Warning, TEXT("We're here!"));
	TargetCharacter = ControlledCharacter->RollbackDebug->DebugFindOtherPlayer();
	TargetCharacter->RollbackDebug->OnOpenRollbackTimelineUI(this);
	TargetCharacter->RollbackDebug->SetShouldUpdateTimelineSlider(true);
}

void URollbackTimelineWidget::OnTimelineSliderValueChanged(float Value)
{
	TargetCharacter->RollbackDebug->SetRollbackTimelineValue(Value);
}

void URollbackTimelineWidget::OnFudgeSliderValueChanged(float Value)
{
	TargetCharacter->RollbackDebug->DebugSetShapeDisplayOffset(Value);
	FudgeText->SetText(FText::FromString(FString::Printf(TEXT("%f s"), Value)));
}

void URollbackTimelineWidget::SetSliderMaxValue(float MaxValue)
{
	TimelineSlider->SetMaxValue(MaxValue);
	EndTimeText->SetText(FText::FromString(FString::Printf(TEXT("%f s"), MaxValue)));
}

void URollbackTimelineWidget::OnStartSliding()
{
	//TargetCharacter->SetShouldUpdateTimelineSlider(false);
}

void URollbackTimelineWidget::OnStopSliding()
{
	//TargetCharacter->SetShouldUpdateTimelineSlider(true);
}

void URollbackTimelineWidget::OnInterpolateCheckboxChanged(bool CheckState)
{
	TargetCharacter->RollbackDebug->SetShouldInterpolateDebugPoses(CheckState);
}

void URollbackTimelineWidget::OnRefreshButtonPress()
{
	TargetCharacter->RollbackDebug->SetShouldUpdateTimelineSlider(true);
}
