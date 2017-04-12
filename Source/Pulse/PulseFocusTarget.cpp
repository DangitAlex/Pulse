// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseFocusTarget.h"


// Sets default values
APulseFocusTarget::APulseFocusTarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SetActorHiddenInGame(true);

	FocusTargetTitle = CreateDefaultSubobject<UTextRenderComponent>(TEXT("FocusTargetTitle"));
	RootComponent = FocusTargetTitle;
	FocusTargetTitle->SetRelativeLocation({ 0.f, 0.f, 150.f });
	FocusTargetTitle->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	FocusTargetTitle->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextBottom);
	FocusTargetTitle->XScale = 4.f;
	FocusTargetTitle->YScale = 4.f;

	TargetName = "Important Thing";
	bIsDefaultFocusTarget = false;
}

void APulseFocusTarget::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if(FocusTargetTitle) FocusTargetTitle->SetText((FText::FromName(TargetName)));
}

