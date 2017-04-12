// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PostProcessComponent.h"
#include <EngineGlobals.h>
#include "DrawDebugHelpers.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "PulseSurface.generated.h"

UENUM(BlueprintType)
enum class ESurfaceType : uint8
{
	NONE	UMETA(DisplayName = "None"),
	RED 	UMETA(DisplayName = "Red"),
	GREEN 	UMETA(DisplayName = "Green"),
	BLUE	UMETA(DisplayName = "Blue"),
	CUSTOM	UMETA(DisplayName = "Custom")
};

UCLASS()
class PULSE_API APulseSurface : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this actor's properties
	APulseSurface();
	void SetSurfaceActive(bool newActive);
	virtual void PulseSurface(FVector PulseLocation);
	void SetIndicesActiveInRadiusFromPoint(FVector point, float radius);
	virtual bool CanPulseSurfaceFromLocation(FVector location);

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Root, meta = (AllowPrivateAccess = "true"))
		USceneComponent* StartPoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		UPostProcessComponent* BlueVolume_PostProcess;

	UInstancedStaticMeshComponent* MyInstance;
	UMaterialInstanceDynamic* DynamicBlueVolumeMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		bool bForceUpdateSurface;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		ESurfaceType SurfaceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		bool bIsSurfaceActive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceSectionScale_XY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceSectionScale_Z;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceOscillationLength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceOscillationSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceOscillationWaveIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceSpread;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float RequiredMinUpDotToPulse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		bool bIsDynamicFloor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		bool bDisableSurfaceCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		bool bShouldUpdateSurfaceCollision;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		UStaticMesh* SurfaceMeshOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceColor_PulseIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		float SurfaceColor_PulseSpeedScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface, meta = (AllowPrivateAccess = "true"))
		FLinearColor CustomSurfaceColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		float SurfaceUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		float MaxLowQualityUpdateInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		float CheckForPlayerInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		float HighQualityDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		bool bOverrideHybernationDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Optimization, meta = (AllowPrivateAccess = "true"))
		float HybernateDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug)
		bool bDebugSurface;

	ACharacter* MyPlayer;
	UMaterialInstanceDynamic* DynamicSurfaceMaterial;
	TArray <int> ActiveIndices;

	FVector PlayerCheckOrigin;

	float lastSurfaceUpdateTime;
	float lastBlueVolumeActivationTime;
	float lastCheckForPlayerTime;

	float updateInterval_current;

	bool bIsDynamicFloorActive;
	bool bIsHybernating;

	//Instanced  Static Mesh Functions
	virtual void CheckForPlayer();
	virtual bool CanCreateSurface();
	virtual bool CreateSurface();
	virtual void UpdateSurface();
	virtual void SetSurfaceInstanceTransform(int index, FTransform newTransform, bool bForceUpdateCollision);
	virtual float GetPlayerDistanceFromSurfaceSquared();
	virtual float GetPlayerDistanceFromSurfaceSquared(int32 index);
	bool IsIndexActive(int32 index);
	virtual void SetBlueVolumeActive(bool active);

	void PrintToViewport(FString toPrint, FColor printColor = FColor::Yellow, float printDuration = 0.f);

public:
	FORCEINLINE virtual float GetMaxInstanceCount() const { return 0; }
	FORCEINLINE UInstancedStaticMeshComponent* GetStaticMeshInstanceComponent() const { return MyInstance; }
	FORCEINLINE ESurfaceType GetSurfaceType() const { return SurfaceType; }
	FORCEINLINE UStaticMesh* GetSurfaceMesh() const { return MyInstance->GetStaticMesh(); }
	FORCEINLINE bool GetIsSurfaceActive() const { return bIsSurfaceActive; }
	FORCEINLINE FLinearColor GetSurfaceCustomColor() const { return SurfaceType == ESurfaceType::CUSTOM ? CustomSurfaceColor : FLinearColor(0.f, 0.f, 0.f, 0.f); }
};
