// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/Character.h"
#include "EngineUtils.h"
#include "PulseSurface.h"
#include "PulseFocusTarget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Classes/Components/SplineComponent.h"
#include "PulseCharacter.generated.h"

UCLASS(config=Game)
class APulseCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* PlayerSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		class UPointLightComponent* PlayerLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		class UPhysicsHandleComponent* PlayerSphereHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		float PlayerSphere_MaxWanderDist;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		float PlayerSphere_HandleLinearStiffness_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = PlayerSphere, meta = (AllowPrivateAccess = "true"))
		float PlayerSphere_HandleLinearStiffness_Surge;

	// Camera Vars
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float PlayerIntensity_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float PlayerIntensity_Charge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float PlayerIntensity_Pulse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float PlayerIntensity_LerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FLinearColor PlayerColor_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FLinearColor PlayerColor_RED;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FLinearColor PlayerColor_GREEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		FLinearColor PlayerColor_BLUE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float PlayerColor_LerpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraOffset_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraOffset_Surge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraOffset_Focus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFocusTargetSpeed_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFocusTargetSpeed_Surge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFOV_Default;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFOV_Focus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFOV_Surge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		float CameraFOV_LerpSpeed;

	//VFX
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* VFX_Charging;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* VFX_Charged;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* VFX_Pulse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UParticleSystemComponent* VFX_PulsedTrail;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float VFX_Pulse_Duration;

	// Pulse Vars
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PlayerAirMoveForce_RED;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PlayerAirMoveForce_BLUE;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_RED_JumpImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_GREEN_SurgeSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_GREEN_SurgeLaunchScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_GREEN_OffsetFromSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_BLUE_ActivateImpulse;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_BLUE_BoostImpulse;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PulseCheck_SphereRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PulseCheck_LineDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PulseCheck_LineDistance_SurgeSpline;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_Cooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_TimeToCharge;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_Duration_RED;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float Pulse_Duration_GREEN;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float surfaceActivationCheckInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PulseSurfaceDynamicFloor_Radius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pulse, meta = (AllowPrivateAccess = "true"))
		float PulseSurfaceDynamicFloor_Radius_SolidCap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Debug, meta = (AllowPrivateAccess = "true"))
		bool bDebugPlayer;

	UMaterialInstanceDynamic* DynamicPlayerMaterial;

	class APulseSurface_Spline* PreviousPulsedSplineSurface;
	class APulseSurface* PreviousPulsedSurfaceAttachedToSpline;
	class USplineComponent* MySurgeSpline;
	class APulseSurface* MyBlueVolume;
	float currSurgeSplineDistance;

	APulseSurface* CurrentPlayerSurface;
	ESurfaceType PlayerSurfaceType;

	ESurfaceType CurrentPulseType;

	float PlayerIntensity_Current;
	FLinearColor PlayerColor_Current;
	
	APulseFocusTarget* MyFocusTarget;
	FVector CameraFocusTargetLoc;

	float lastPulseTime;
	float lastPulseChargeStartTime;
	float lastSurgeEndTime;
	float lastSurfaceActivationCheckTime;
	FVector lastPlayerWorldPosition;

	TArray <APulseSurface*> CurrentActiveSurfaces;

	float CameraFocusTargetSpeed_Current;
	float CameraOffset_Current;
	float CameraFOV_CurrentTarget;

	bool bWantsToPulse;

	bool bPulseChargePressed;
	bool bCameraFocusTargetPressed;
	bool bIsPulsing;

	bool bLerpingCameraFOV;
	bool bLerpingCameraOffset;
	bool bLerpingPlayerColor;
	bool bLerpingPlayerIntensity;

	//Functions
	void Camera_Tick(float DeltaTime);
	void PlayerSphere_Tick(float DeltaTime);
	APulseSurface* GetPulseSurfaceInDir(FVector CheckDir, bool checkIfValid = true);

	void PulseCharge_Pressed();
	void PulseCharge_Released();
	void PulseCharge_Tick(float DeltaTime);
	APulseSurface* GetClosestPulseSurface(APulseSurface* ignoreSurface = NULL, bool checkIfValid = true);
	APulseSurface* GetClosestPulseSurface(TArray<APulseSurface*> ignoreSurfaces, bool checkIfValid = true);
	void SetCurrentPlayerSurface(APulseSurface* newSurface);
	bool Pulse();
	void SetCurrentPulseType(ESurfaceType newPulseType);
	void Pulse_Tick(float DeltaTime);
	void ConnectToSurgeSpline(APulseSurface_Spline* newSurgeSpline);
	void DisconnectFromSurgeSpline(bool reachedEnd = false, bool canReconnect = false);

	void CameraFocusOnTarget_Pressed();
	void CameraFocusOnTarget_Released();
	void CameraFocusOnTarget_Tick(float DeltaTime);

	void SetLerpToPlayerColor(FLinearColor newTargetColor);
	void SetLerpToPlayerColor(FLinearColor newTargetColor, FLinearColor hardSetCurrColor);
	void SetLerpToPlayerIntensity(float newTargetIntensity);
	void SetLerpToPlayerIntensity(float newTargetIntensity, float hardSetCurrIntensity);

	void UpdatePlayerColor(FLinearColor PlayerColor);
	void UpdatePlayerIntensity(float PlayerIntensity);
	void UpdatePlayerQuiverSpeed_Master(float MasterQuiverSpeed);
	void UpdatePlayerQuiverSpeed_Movement(float MovementQuiverSpeed);
	void UpdatePlayerQuiverDistance(float QuiverDistance);

	void ActivateNearbySurfaces_Tick();

	void PrintToViewport(FString toPrint, FColor printColor = FColor::Yellow, float printDuration = 0.f);

public:
	APulseCharacter();
	void SetBlueVolumeRef(APulseSurface* newBlueVolume);
	void SetMyFocusTarget(APulseFocusTarget* newTarget);

	UFUNCTION(BlueprintCallable, Category = Game)
		void QuitGame();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	void OnConstruction(const FTransform& Transform) override;

	void BeginPlay() override;
	void Tick(float DeltaTime) override;

	void AddControllerPitchInput(float input) override;
	void AddControllerYawInput(float input) override;

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Jump() override;
	void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE class APulseSurface* GetBlueVolumeRef() const { return MyBlueVolume; }
	FORCEINLINE FVector GetPlayerSphereLocation() const { return PlayerSphere->GetComponentLocation(); }
	FORCEINLINE float GetDynamicFloorRadius() const { return PulseSurfaceDynamicFloor_Radius; }
	FORCEINLINE float GetDynamicFloorCapRadius() const { return PulseSurfaceDynamicFloor_Radius_SolidCap; }
};

