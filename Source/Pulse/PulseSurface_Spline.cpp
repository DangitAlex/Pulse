// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseCharacter.h"
#include "PulseSurface_Spline.h"

APulseSurface_Spline::APulseSurface_Spline()
{
	SurfaceSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SurfaceSpline"));
	//SurfaceSpline->bShouldVisualizeScale = true;
	SurfaceSpline->SetupAttachment(RootComponent);

	EndLaunchArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("EndLaunchArrow"));
	EndLaunchArrow->SetupAttachment(SurfaceSpline);
	EndLaunchArrow->ArrowSize = 5.f;

	SplineSearchIncrementDist = 100.f;
	SplineStartRadius = 500.f;
	Surface_X = 5;
	SurfaceType = ESurfaceType::GREEN;
}

void APulseSurface_Spline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (MyInstance->GetStaticMesh())
	{
		forwardOffsetDist = (MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().X * SurfaceSectionScale_XY) + SurfaceSpread;
		rightOffsetDist = (MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().Y * SurfaceSectionScale_XY) + SurfaceSpread;
	}

	EndLaunchArrow->SetWorldLocationAndRotation(SurfaceSpline->GetWorldLocationAtSplinePoint(SurfaceSpline->GetNumberOfSplinePoints()), SurfaceSpline->GetRotationAtSplinePoint(SurfaceSpline->GetNumberOfSplinePoints(), ESplineCoordinateSpace::World));
}

void APulseSurface_Spline::BeginPlay()
{
	Super::BeginPlay();
	
	if (MyInstance->GetStaticMesh())
	{
		forwardOffsetDist = (MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().X * SurfaceSectionScale_XY) + SurfaceSpread;
		rightOffsetDist = (MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().Y * SurfaceSectionScale_XY) + SurfaceSpread;
	}
}

bool APulseSurface_Spline::CanCreateSurface()
{
	return (MyInstance->GetInstanceCount() == 0);
}

bool APulseSurface_Spline::CreateSurface()
{
	if (!Super::CreateSurface())
		return false;

	int surfaceLength = FMath::FloorToInt(SurfaceSpline->GetSplineLength() / forwardOffsetDist);

	for (int i = 0; i < surfaceLength; i++)
	{
		for (int j = 0; j < Surface_X; j++)
		{
			MyInstance->AddInstance(GetSplineSurfaceMeshInstanceTransform(i, j));
		}
	}

	EndLaunchArrow->SetWorldLocationAndRotation(SurfaceSpline->GetWorldLocationAtSplinePoint(SurfaceSpline->GetNumberOfSplinePoints()), SurfaceSpline->GetRotationAtSplinePoint(SurfaceSpline->GetNumberOfSplinePoints(), ESplineCoordinateSpace::World));
	
	return true;
}

void APulseSurface_Spline::UpdateSurface()
{
	for (int i = 0; i < (FMath::FloorToInt(MyInstance->GetInstanceCount() / Surface_X)); i++)
	{
		for (int j = 0; j < Surface_X; j++)
		{
			int index = (i * Surface_X) + j;

			if (index >= 0 && index < GetMaxInstanceCount())
			{
				FTransform newInstanceTransform = GetSplineSurfaceMeshInstanceTransform(i, j);
				FVector InstanceUpVector = newInstanceTransform.GetRotation().GetUpVector();

				float currWaveZOffset = (SurfaceHeight + (FMath::Sin((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - (i * (SurfaceOscillationWaveIntensity / 10.f))) * SurfaceOscillationSpeed) * SurfaceOscillationLength) - SurfaceOscillationLength);
				FVector newInstanceOffset = FVector();

				if (bIsDynamicFloor && bIsDynamicFloorActive && IsIndexActive(index))
				{
					APulseCharacter* MyPulseCharacter = Cast<APulseCharacter>(MyPlayer);

					if (MyPulseCharacter)
					{
						float playerDistanceSquared = GetPlayerDistanceFromSurfaceSquared(index);

						PrintToViewport(FString::SanitizeFloat(playerDistanceSquared) + " :: " + FString::SanitizeFloat(FMath::Pow(MyPulseCharacter->GetDynamicFloorCapRadius(), 2.f)), FColor::Green, SurfaceUpdateInterval);

						if (playerDistanceSquared <= FMath::Pow(MyPulseCharacter->GetDynamicFloorCapRadius(), 2.f))
						{
							newInstanceOffset = FVector(0.f);
							if (bDebugSurface) DrawDebugLine(GetWorld(), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()) + (GetActorTransform().TransformVector((UKismetMathLibrary::GetUpVector(newInstanceTransform.GetRotation().Rotator()))) * 500.f), FColor::Green, false, -1.f, (uint8)'\000', 5.f);
						}
						else
						{
							newInstanceOffset = InstanceUpVector * (FMath::Lerp(0.f, currWaveZOffset, FMath::Clamp((playerDistanceSquared - FMath::Pow(MyPulseCharacter->GetDynamicFloorCapRadius(), 2.f)) / FMath::Pow(MyPulseCharacter->GetDynamicFloorRadius(), 2.f), 0.f, 1.f)));
							if (bDebugSurface) DrawDebugLine(GetWorld(), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()), MyPlayer->GetActorLocation(), FColor::Yellow, false, -1.f, (uint8)'\000', 2.f);
						}
					}
				}
				else
					newInstanceOffset = InstanceUpVector * currWaveZOffset;

				FVector newInstanceLoc = newInstanceTransform.GetLocation() += newInstanceOffset;
				newInstanceTransform.SetLocation(newInstanceLoc);

				SetSurfaceInstanceTransform(index, newInstanceTransform, false);
			}
		}

		MyInstance->ReleasePerInstanceRenderData();
	}
}

bool APulseSurface_Spline::CanPulseSurfaceFromLocation(FVector location)
{
	float closestDist = GetClosestSplineDistanceToWorldLocation(location);
	FVector offset = SurfaceHeight * UKismetMathLibrary::GetUpVector(SurfaceSpline->GetRotationAtDistanceAlongSpline(closestDist, ESplineCoordinateSpace::World));


	return FVector::DotProduct((location - (offset + (SurfaceSpline->GetLocationAtDistanceAlongSpline(closestDist, ESplineCoordinateSpace::World)))).GetSafeNormal(), UKismetMathLibrary::GetUpVector(SurfaceSpline->GetRotationAtDistanceAlongSpline(closestDist, ESplineCoordinateSpace::World))) >= RequiredMinUpDotToPulse;
}

FTransform APulseSurface_Spline::GetSplineSurfaceMeshInstanceTransform(int x, int y)
{
	x *= forwardOffsetDist;

	FRotator currSplineRot = SurfaceSpline->GetRotationAtDistanceAlongSpline(x, ESplineCoordinateSpace::Local);
	FVector currSplineScale = SurfaceSpline->GetScaleAtDistanceAlongSpline(x);
	FVector currRightVector = UKismetMathLibrary::GetRightVector(currSplineRot);
	FVector currUpVector = UKismetMathLibrary::GetUpVector(currSplineRot);
	FVector currStartLoc = SurfaceSpline->GetLocationAtDistanceAlongSpline(x, ESplineCoordinateSpace::Local) + (currRightVector * -(rightOffsetDist * currSplineScale.Y * (Surface_X / 2)));


	FVector currInstanceLoc = currStartLoc + (currUpVector * SurfaceHeight) + (currRightVector * (y * (rightOffsetDist * currSplineScale.Y))),
		currScale = FVector(SurfaceSectionScale_XY, SurfaceSectionScale_XY, SurfaceSectionScale_Z) * currSplineScale;

	return FTransform(currSplineRot, currInstanceLoc, currScale);
}

float APulseSurface_Spline::GetClosestSplineDistanceToWorldLocation(FVector worldLocation)
{
	if ((worldLocation - GetActorLocation()).Size() < SplineStartRadius)
		return 0.f;

	float closestDist = 0.f,
		  closestDistanceToLocation = (SurfaceSpline->GetWorldLocationAtDistanceAlongSpline(closestDist) - worldLocation).Size(),
		  currDist = SplineSearchIncrementDist;

	while (currDist < SurfaceSpline->GetSplineLength())
	{
		if ((SurfaceSpline->GetWorldLocationAtDistanceAlongSpline(currDist) - worldLocation).Size() < closestDistanceToLocation)
		{
			closestDistanceToLocation = (SurfaceSpline->GetWorldLocationAtDistanceAlongSpline(currDist) - worldLocation).Size();
			closestDist = currDist;
		}

		currDist += SplineSearchIncrementDist;
	}

	return closestDist;
}