// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PulseSurface.h"
#include "PulseSurface_Floor.generated.h"

/**
 * 
 */
UCLASS()
class PULSE_API APulseSurface_Floor : public APulseSurface
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		UBoxComponent* BlueVolume_Box;

	UPROPERTY(BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* BlueVolume_Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		bool bBlueVolume_StartActive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BlueVolume , meta = (AllowPrivateAccess = "true"))
		float BlueVolume_Z;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		float BlueVolume_LerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = BlueVolume, meta = (AllowPrivateAccess = "true"))
		float BlueVolume_LifeTime;

	class UStaticMesh* BlueVolumeMeshRef;

	bool bBlueVolume_isActive;
	bool bBlueVolume_isLerping;

	void SetBlueVolumeZRatio(float ratio);
	void UpdateBlueVolume_Tick(float DeltaTime);

	UFUNCTION(Category = BlueVolume)
		void OnActorEnterBlueVolume(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Category = BlueVolume)
		void OnActorLeaveBlueVolume(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	APulseSurface_Floor();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface)
		int Surface_X;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Surface)
		int Surface_Y;

protected:
	void OnConstruction(const FTransform& Transform) override;
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	bool CreateSurface() override;
	void UpdateSurface() override;
	void PulseSurface(FVector PulseLocation) override;
	void SetBlueVolumeActive(bool active) override;

public:
	FORCEINLINE float GetMaxInstanceCount() const override { if (!MyInstance->GetStaticMesh()) return -1.f; else return (Surface_X * Surface_Y); }

};
