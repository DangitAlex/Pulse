// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/TriggerBox.h"
#include "PulseFocusTarget.h"
#include "PulseFocusTargetTrigger.generated.h"

/**
 * 
 */
UCLASS()
class PULSE_API APulseFocusTargetTrigger : public ATriggerBox
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = FocusTarget, meta = (AllowPrivateAccess = "true"))
		APulseFocusTarget* SetFocusTargetTo;

protected:
	UFUNCTION(Category = Overlap)
		void OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	APulseFocusTargetTrigger();
	
};
