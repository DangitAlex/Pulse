// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseSurface_Sphere.h"

APulseSurface_Sphere::APulseSurface_Sphere()
{

}

void APulseSurface_Sphere::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);


}

bool APulseSurface_Sphere::CreateSurface()
{
	if(!Super::CreateSurface())
		return false;


	return false;
}

void APulseSurface_Sphere::UpdateSurface()
{

}


