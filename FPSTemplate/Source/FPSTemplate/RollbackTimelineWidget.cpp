// Fill out your copyright notice in the Description page of Project Settings.


#include "RollbackTimelineWidget.h"

bool URollbackTimelineWidget::Initialize()
{
	bool Res = UUserWidget::Initialize();

	TimelineSlider->OnValueChanged.AddDynamic(this, &URollbackTimelineWidget::OnSliderValueChanged);
	TimelineSlider->OnMouseCaptureBegin.AddDynamic(this, &URollbackTimelineWidget::OnStartSliding);
	TimelineSlider->OnMouseCaptureEnd.AddDynamic(this, &URollbackTimelineWidget::OnStopSliding);

	InterpolateCheckbox->OnCheckStateChanged.AddDynamic(this, &URollbackTimelineWidget::OnInterpolateCheckboxChanged);

	RefreshButton->OnClicked.AddDynamic(this, &URollbackTimelineWidget::OnRefreshButtonPress);

	return Res;
}

void URollbackTimelineWidget::AssignTargetCharacter(AFPSTemplateCharacter* ControlledCharacter)
{
	//UE_LOG(LogTemp, Warning, TEXT("We're here!"));
	TargetCharacter = ControlledCharacter->DebugFindOtherPlayer();
	TargetCharacter->OnOpenRollbackTimelineUI(this);
	TargetCharacter->SetShouldUpdateTimelineSlider(true);
}

void URollbackTimelineWidget::OnSliderValueChanged(float Value)
{
	TargetCharacter->SetRollbackTimelineValue(Value);
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
	TargetCharacter->SetShouldInterpolateDebugPoses(CheckState);
}

void URollbackTimelineWidget::OnRefreshButtonPress()
{
	TargetCharacter->SetShouldUpdateTimelineSlider(true);
}
