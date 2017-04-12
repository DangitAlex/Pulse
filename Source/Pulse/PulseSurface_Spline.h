// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PulseSurface.h"
#include "Components/SplineComponent.h"
#include "Components/ArrowComponent.h"
#include "PulseSurface_Spline.generated.h"

/**
 * 
 */
UCLASS()
class PULSE_API APulseSurface_Spline : public APulseSurface
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Root, meta = (AllowPrivateAccess = "true"))
		USplineComponent* SurfaceSpline;

	UPROPERTY(BlueprintReadOnly, Category = Spline, meta = (AllowPrivateAccess = "true"))
		UArrowComponent* EndLaunchArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spline, meta = (AllowPrivateAccess = "true"))
		float SplineSearchIncrementDist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Spline, meta = (AllowPrivateAccess = "true"))
		float SplineStartRadius;

	FTransform GetSplineSurfaceMeshInstanceTransform(int x, int);

	float forwardOffsetDist;
	float rightOffsetDist;

public:
	APulseSurface_Spline();
	bool CanPulseSurfaceFromLocation(FVector location) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface)
		int Surface_X;

	float GetClosestSplineDistanceToWorldLocation(FVector worldLocation);

protected:
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	bool CanCreateSurface() override;
	bool CreateSurface() override;
	void UpdateSurface() override;
	
public:
	FORCEINLINE float GetMaxInstanceCount() const override { if (!MyInstance->GetStaticMesh()) return -1.f; else return (Surface_X  * (SurfaceSpline->GetSplineLength() / (MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().X * SurfaceSectionScale_XY) + SurfaceSpread)); }
	FORCEINLINE USplineComponent* GetSurfaceSpline() const { return SurfaceSpline; }
	FORCEINLINE UArrowComponent* GetLaunchArrow() const { return EndLaunchArrow; }
};
