// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "PulseFocusTarget.generated.h"

UCLASS()
class PULSE_API APulseFocusTarget : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FocusTarget, meta = (AllowPrivateAccess = "true"))
		FName TargetName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FocusTarget, meta = (AllowPrivateAccess = "true"))
		bool bIsDefaultFocusTarget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FocusTarget, meta = (AllowPrivateAccess = "true"))
		UTextRenderComponent* FocusTargetTitle;

protected:
	void OnConstruction(const FTransform& Transform) override;

public:	
	APulseFocusTarget();

	FORCEINLINE bool IsDefaultFocusTarget() const { return bIsDefaultFocusTarget; }
	FORCEINLINE FName GetFocusTargetName() const { return TargetName; }

};
