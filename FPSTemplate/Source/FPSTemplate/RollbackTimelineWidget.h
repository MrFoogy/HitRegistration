// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "FPSTemplateCharacter.h"
#include "RollbackTimelineWidget.generated.h"

/**
 * 
 */
UCLASS()
class FPSTEMPLATE_API URollbackTimelineWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class USlider* TimelineSlider;

	UPROPERTY(meta = (BindWidget))
	class USlider* FudgeSlider;

	UPROPERTY(meta = (BindWidget))
	class UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* EndTimeText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* FudgeText;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* InterpolateCheckbox;

	UFUNCTION(BlueprintCallable)
	void AssignTargetCharacter(AFPSTemplateCharacter* Character);

	virtual bool Initialize() override;

	UFUNCTION()
	void OnTimelineSliderValueChanged(float Value);

	UFUNCTION()
	void OnFudgeSliderValueChanged(float Value);

	UFUNCTION()
	void OnInterpolateCheckboxChanged(bool CheckState);

	UFUNCTION()
	void OnRefreshButtonPress();

	UFUNCTION()
	void OnStartSliding();

	UFUNCTION()
	void OnStopSliding();

	void SetSliderMaxValue(float MaxValue);

protected:
	AFPSTemplateCharacter* TargetCharacter;
};
