// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM()
enum MovementReplicationType
{
	Interpolation, Default, Extrapolation
};

/**
 * 
 */
class FPSTEMPLATE_API ReplicationUtil
{
public:
	ReplicationUtil();
	~ReplicationUtil();
};
