// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseCharacter.h"
#include "PulseFocusTargetTrigger.h"


APulseFocusTargetTrigger::APulseFocusTargetTrigger() 
{
	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &APulseFocusTargetTrigger::OnActorEnterTrigger);
	Cast<UBoxComponent>(GetCollisionComponent())->SetBoxExtent(FVector(200.f));
}

void APulseFocusTargetTrigger::OnActorEnterTrigger(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA(APulseCharacter::StaticClass()))
		Cast<APulseCharacter>(OtherActor)->SetMyFocusTarget(SetFocusTargetTo);
}

