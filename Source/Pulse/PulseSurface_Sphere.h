// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PulseSurface.h"
#include "PulseSurface_Sphere.generated.h"

/**
 * 
 */
UCLASS()
class PULSE_API APulseSurface_Sphere : public APulseSurface
{
	GENERATED_BODY()

public:
	APulseSurface_Sphere();
	
protected:
	void OnConstruction(const FTransform& Transform) override;
	bool CreateSurface() override;
	void UpdateSurface() override;

public:

};
