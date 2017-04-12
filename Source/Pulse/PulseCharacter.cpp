// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "Pulse.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Classes/PhysicsEngine/PhysicsHandleComponent.h"
#include "PulseSurface_Spline.h"
#include "PulseCharacter.h"

//////////////////////////////////////////////////////////////////////////
// APulseCharacter

APulseCharacter::APulseCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(100.f, 110.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PlayerAirMoveForce_RED = 50000.f;
	PlayerAirMoveForce_BLUE = 25000.f;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->AirControl = 2.f;
	GetCharacterMovement()->AirControlBoostMultiplier = 0.f;
	GetCharacterMovement()->AirControlBoostVelocityThreshold = 0.f;
	GetCharacterMovement()->JumpZVelocity = 1500.f;
	GetCharacterMovement()->GravityScale = 3.f;
	GetCharacterMovement()->SetWalkableFloorAngle(35.f);
	GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	GetCharacterMovement()->MaxFlySpeed = 1800.f;
	GetCharacterMovement()->BrakingDecelerationFlying = 100.f;
	GetCharacterMovement()->MaxAcceleration = 1280.f;
	GetCharacterMovement()->GroundFriction = 1.f;
	GetCharacterMovement()->BrakingFrictionFactor = 0.05f;
	
	// Create Player Sphere
	PlayerSphere = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerSphere"));
	PlayerSphere->SetupAttachment(RootComponent);
	PlayerSphere->SetSimulatePhysics(true);

	// Create Player Light
	PlayerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PlayerLight"));
	PlayerLight->SetupAttachment(PlayerSphere);
	PlayerLight->SetIntensity(50000.f);
	
	// Player Sphere Color
	PlayerColor_Default = FLinearColor(1.f, 1.f, 0.f, 1.f);
	PlayerLight->SetLightColor(PlayerColor_Default.ToFColor(true)), true;
	PlayerColor_Current = PlayerColor_Default;
	PlayerColor_RED = FLinearColor(1.f, 0.f, 0.f, 1.f);
	PlayerColor_GREEN = FLinearColor(0.f, 1.f, 0.f, 1.f);
	PlayerColor_BLUE = FLinearColor(0.f, 0.2f, 0.8f, 1.f);
	PlayerColor_LerpSpeed = 6.f;

	// Player Sphere Emmissive Intensity
	PlayerIntensity_Default = 15.f;
	PlayerIntensity_Current = PlayerIntensity_Default;
	PlayerIntensity_Charge = 80.f;
	PlayerIntensity_Pulse = 3000.f;
	PlayerIntensity_LerpSpeed = 8.f;

	PlayerSphere_HandleLinearStiffness_Default = 200.f;
	PlayerSphere_HandleLinearStiffness_Surge = 400.f;

	// PlayerSphere Physics Handle
	PlayerSphereHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PlayerSphereHandle"));
	PlayerSphereHandle->SetLinearDamping(8.f);
	PlayerSphereHandle->SetLinearStiffness(PlayerSphere_HandleLinearStiffness_Default);
	PlayerSphereHandle->SetAngularDamping(10.f);
	PlayerSphereHandle->SetAngularStiffness(12.f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector(-200.f, 0.f, 150.f);
	CameraBoom->bEnableCameraLag = true;

	// Camera Focus Vars
	CameraFocusTargetSpeed_Default = 3.f;
	CameraFocusTargetSpeed_Surge = 4.5f;
	CameraFocusTargetSpeed_Current = CameraFocusTargetSpeed_Default;
	CameraFOV_Default = 100.f;
	CameraFOV_Focus = 70.f;
	CameraFOV_Surge = 120.f;
	CameraFOV_CurrentTarget = CameraFOV_Default;
	CameraFOV_LerpSpeed = 3.f;
	

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetFieldOfView(CameraFOV_Default);

	// VFX
	VFX_Charging = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFX_Charging"));
	struct ConstructorHelpers::FObjectFinder<UParticleSystem> findPS_Charging(TEXT("ParticleSystem'/Game/CreatedAssets/VFX/Player_Charging.Player_Charging'"));
	VFX_Charging->SetTemplate(findPS_Charging.Object);
	VFX_Charging->SetupAttachment(RootComponent);
	VFX_Charging->SetRelativeLocation({0.f, 0.f, -40.f});
	VFX_Charging->SetAutoActivate(false);

	VFX_Charged = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFX_Charged"));
	struct ConstructorHelpers::FObjectFinder<UParticleSystem> findPS_Charged(TEXT("ParticleSystem'/Game/CreatedAssets/VFX/Player_Charged.Player_Charged'"));
	VFX_Charged->SetTemplate(findPS_Charged.Object);
	VFX_Charged->SetupAttachment(PlayerSphere);
	VFX_Charged->SetAutoActivate(false);

	VFX_Pulse = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFX_Pulse"));
	struct ConstructorHelpers::FObjectFinder<UParticleSystem> findPS_Pulse(TEXT("ParticleSystem'/Game/CreatedAssets/VFX/Player_Pulse.Player_Pulse'"));
	VFX_Pulse->SetTemplate(findPS_Pulse.Object);
	VFX_Pulse->SetupAttachment(PlayerSphere);
	VFX_Pulse->SetAutoActivate(false);
	VFX_Pulse->SecondsBeforeInactive = 0.2f;

	VFX_PulsedTrail = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VFX_PulsedTrail"));
	struct ConstructorHelpers::FObjectFinder<UParticleSystem> findPS_PulsedTrail(TEXT("ParticleSystem'/Game/CreatedAssets/VFX/Player_Pulsing.Player_Pulsing'"));
	VFX_PulsedTrail->SetTemplate(findPS_PulsedTrail.Object);
	VFX_PulsedTrail->SetupAttachment(PlayerSphere);
	VFX_PulsedTrail->SetAutoActivate(false);

	VFX_Pulse_Duration = 0.3f;

	// Vars
	CurrentPlayerSurface = nullptr;
	PlayerSurfaceType = ESurfaceType::NONE;

	bWantsToPulse = false;
	bPulseChargePressed = false;
	bCameraFocusTargetPressed = false;
	bLerpingCameraFOV = false;

	currSurgeSplineDistance = 0.f;
	PlayerSphere_MaxWanderDist = 130.f;
	CameraFocusTargetLoc = FVector(0.f, 0.f, 0.f);
	PulseCheck_SphereRadius = 400.f;
	PulseCheck_LineDistance = 60.f;
	PulseCheck_LineDistance_SurgeSpline = 300.f;
	Pulse_TimeToCharge = 0.6f;
	Pulse_Cooldown = 1.f;

	Pulse_Duration_RED = 3.f;
	Pulse_Duration_GREEN = 0.8f;

	Pulse_RED_JumpImpulse = 3500.f;
	Pulse_GREEN_SurgeSpeed = 3000.f;
	Pulse_GREEN_SurgeLaunchScale = 14.f;
	Pulse_GREEN_OffsetFromSpline = 120.f;
	Pulse_BLUE_ActivateImpulse = 900.f;
	Pulse_BLUE_BoostImpulse = 2000.f;

	surfaceActivationCheckInterval = 0.1f;
	PulseSurfaceDynamicFloor_Radius = 900.f;
	PulseSurfaceDynamicFloor_Radius_SolidCap = 120.f;

	bDebugPlayer = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void APulseCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APulseCharacter::BeginPlay()
{
	Super::BeginPlay();

	VFX_Charging->SetActive(false, true);
	VFX_Charged->SetActive(false, true);
	VFX_Pulse->SetActive(false, true);
	VFX_PulsedTrail->SetActive(false, true);

	// Create and assign Dynamic Player Sphere Material
	if (PlayerSphere->GetStaticMesh())
	{
		DynamicPlayerMaterial = UMaterialInstanceDynamic::Create(PlayerSphere->GetMaterial(0), this);
		PlayerSphere->SetMaterial(0, DynamicPlayerMaterial);
	}

	PlayerSphereHandle->GrabComponentAtLocationWithRotation(PlayerSphere, TEXT("NONE"), GetActorLocation(), GetActorRotation());

	for (TActorIterator<APulseFocusTarget> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		APulseFocusTarget* currTarget = *ActorItr;

		if (currTarget->IsDefaultFocusTarget())
		{
			MyFocusTarget = currTarget;
			CameraFocusTargetLoc = MyFocusTarget->GetActorLocation();

			break;
		}
	}

	SetCurrentPulseType(ESurfaceType::NONE);
	lastPulseTime = UGameplayStatics::GetRealTimeSeconds(GetWorld()) - Pulse_Cooldown;
	lastSurfaceActivationCheckTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());

	lastPlayerWorldPosition = GetActorLocation();
}

void APulseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PrintToViewport("Velocity = " + FString::SanitizeFloat(GetVelocity().Size()));

	ActivateNearbySurfaces_Tick();
	PlayerSphere_Tick(DeltaTime);
	Pulse_Tick(DeltaTime);
	Camera_Tick(DeltaTime);

	if (bPulseChargePressed)
		PulseCharge_Tick(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
// Input

void APulseCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &APulseCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APulseCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &APulseCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APulseCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("Pulse", IE_Pressed, this, &APulseCharacter::PulseCharge_Pressed);
	PlayerInputComponent->BindAction("Pulse", IE_Released, this, &APulseCharacter::PulseCharge_Released);

	PlayerInputComponent->BindAction("FocusTarget", IE_Pressed, this, &APulseCharacter::CameraFocusOnTarget_Pressed);
	PlayerInputComponent->BindAction("FocusTarget", IE_Released, this, &APulseCharacter::CameraFocusOnTarget_Released);

	PlayerInputComponent->BindAction("Exit", IE_Released, this, &APulseCharacter::QuitGame);
}

void APulseCharacter::AddControllerPitchInput(float input)
{
	if (bCameraFocusTargetPressed)
		return;

	Super::AddControllerPitchInput(input);
}

void APulseCharacter::AddControllerYawInput(float input)
{
	if (bCameraFocusTargetPressed)
		return;

	Super::AddControllerYawInput(input);
}

void APulseCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void APulseCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void APulseCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		FVector Direction;
		float currAirForce = 0.f;
		
		if (CurrentPulseType > ESurfaceType::NONE)
		{
			switch (CurrentPulseType)
			{
			case ESurfaceType::RED:
				currAirForce = PlayerAirMoveForce_RED;
				break;

			case ESurfaceType::BLUE:
				currAirForce = PlayerAirMoveForce_BLUE;
				break;
			}
		}

		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking || GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
		{
			// find out which way is forward
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// get forward vector
			Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

			PrintToViewport(FString::SanitizeFloat(FMath::Clamp(FVector::DotProduct((Direction * Value), -GetVelocity().GetSafeNormal()), 0.f, 1.f)));

			currAirForce += ((2.f * currAirForce) * FMath::Clamp(FVector::DotProduct((Direction * Value), -GetVelocity().GetSafeNormal()), 0.f, 1.f));
		}
		else if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
		{
			Direction = UKismetMathLibrary::GetForwardVector(GetControlRotation());
			currAirForce *= FMath::Clamp(FVector::DotProduct((Direction * Value), -GetVelocity().GetSafeNormal()), 0.f, 1.f);
		}

		AddMovementInput(Direction, Value);

		if (CurrentPulseType > ESurfaceType::NONE)
		{
			GetCharacterMovement()->AddForce((Value * currAirForce) * Direction);
			PrintToViewport("Forward Air: " + ((Value * currAirForce) * Direction).ToString());
		}
	}
}

void APulseCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(Direction, Value);

		if (CurrentPulseType > ESurfaceType::NONE  && (GetCharacterMovement()->MovementMode == MOVE_Falling || GetCharacterMovement()->MovementMode == MOVE_Flying))
		{
			float currAirForce = 0.f;

			switch (CurrentPulseType)
			{
			case ESurfaceType::RED:
				currAirForce = PlayerAirMoveForce_RED;
				break;

			case ESurfaceType::BLUE:
				currAirForce = PlayerAirMoveForce_BLUE;
				break;
			}

			if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Flying)
				currAirForce *= FMath::Clamp(FVector::DotProduct((Direction * Value), -GetVelocity().GetSafeNormal()), 0.f, 1.f);
			else
				currAirForce += ((2.f * currAirForce) * FMath::Clamp(FVector::DotProduct((Direction * Value), -GetVelocity().GetSafeNormal()), 0.f, 1.f));

			GetCharacterMovement()->AddForce((Value * currAirForce) * Direction);
			PrintToViewport("Right Air: " + ((Value * currAirForce) * Direction).ToString());
		}
	}
}

void APulseCharacter::Jump()
{
	if (bPulseChargePressed && (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseChargeStartTime) >= Pulse_TimeToCharge && (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) >= Pulse_Cooldown)
	{
		if(!Pulse())
			Super::Jump();
	}
	else
		Super::Jump();
}

void APulseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	//On Land
	if (!PreviousPulsedSplineSurface && CurrentPulseType > ESurfaceType::NONE && (GetCharacterMovement()->MovementMode == MOVE_Walking && PrevMovementMode == MOVE_Falling))
		SetCurrentPulseType(ESurfaceType::NONE);
}

void APulseCharacter::PulseCharge_Pressed()
{
	if ((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) >= Pulse_Cooldown)
	{
		bPulseChargePressed = true;
		lastPulseChargeStartTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
		SetLerpToPlayerIntensity(PlayerIntensity_Charge * 0.5f);
		bWantsToPulse = false;
	}
	else
		bWantsToPulse = true;
}

void APulseCharacter::PulseCharge_Released()
{
	bPulseChargePressed = false;
	bWantsToPulse = false;
	SetLerpToPlayerIntensity(PlayerIntensity_Default);
	SetLerpToPlayerColor(PlayerColor_Default);
	VFX_Charging->Deactivate();
	VFX_Charged->Deactivate();

	if (!MySurgeSpline->IsValidLowLevel()) SetCurrentPlayerSurface(nullptr);
}

void APulseCharacter::PulseCharge_Tick(float DeltaTime)
{
	APulseSurface* newSurface;

	if (MySurgeSpline)
	{
		newSurface = GetPulseSurfaceInDir(-GetActorUpVector());

		if(newSurface == PreviousPulsedSplineSurface || !newSurface)
			SetLerpToPlayerColor(PlayerColor_GREEN);
		else
			SetCurrentPlayerSurface(newSurface);

		VFX_Charging->Activate(false);
	}
	else if (MyBlueVolume)
	{
		newSurface = GetClosestPulseSurface(MyBlueVolume);

		if (!newSurface)
			SetLerpToPlayerColor(PlayerColor_BLUE);
		else if(newSurface != CurrentPlayerSurface)
			SetCurrentPlayerSurface(newSurface);

		VFX_Charging->Deactivate();

		FLinearColor newColor = PlayerColor_BLUE;
		VFX_Charging->SetColorParameter(TEXT("Pulse_Color"), newColor);
		VFX_Charged->SetColorParameter(TEXT("Pulse_Color"), newColor);
		VFX_Pulse->SetColorParameter(TEXT("Pulse_Color"), newColor);
		VFX_PulsedTrail->SetColorParameter(TEXT("Pulse_Color"), newColor);
		
	}
	else
	{
		newSurface = GetPulseSurfaceInDir(-GetActorUpVector());
		
		if (!newSurface)
		{
			newSurface = GetClosestPulseSurface();
			
			if (!newSurface)
				SetLerpToPlayerColor(PlayerColor_Default);
		}

		SetCurrentPlayerSurface(newSurface);

		if (PlayerSurfaceType > ESurfaceType::NONE && PlayerSurfaceType != ESurfaceType::CUSTOM)
			VFX_Charging->Activate(false);
		else
			VFX_Charging->Deactivate();
	}

	if (CurrentPlayerSurface && PlayerSurfaceType > ESurfaceType::NONE && PlayerSurfaceType != ESurfaceType::CUSTOM && (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseChargeStartTime) >= Pulse_TimeToCharge && (UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) >= Pulse_Cooldown)
	{
		if(!MyBlueVolume)
			VFX_Charged->Activate(false);

		SetLerpToPlayerIntensity(PlayerIntensity_Charge);
	}
	else
		VFX_Charged->Deactivate();
}

void APulseCharacter::Camera_Tick(float DeltaTime)
{
	if (bLerpingCameraFOV)
	{
		if (!bCameraFocusTargetPressed && FMath::IsNearlyEqual(FollowCamera->FieldOfView, CameraFOV_CurrentTarget, 0.05f))
		{
			FollowCamera->SetFieldOfView(CameraFOV_CurrentTarget);
			bLerpingCameraFOV = false;
		}
		else
			FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, CameraFOV_CurrentTarget, DeltaTime, CameraFOV_LerpSpeed));
	}

	if (bCameraFocusTargetPressed)
		CameraFocusOnTarget_Tick(DeltaTime);
}

void APulseCharacter::ActivateNearbySurfaces_Tick()
{
	if ((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastSurfaceActivationCheckTime) >= surfaceActivationCheckInterval)
	{
		if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking || GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Falling)
		{
			for (APulseSurface* surface : CurrentActiveSurfaces)
			{
				surface->SetSurfaceActive(false);
			}

			CurrentActiveSurfaces.Empty();

			TArray<FHitResult> hits;
			if (UKismetSystemLibrary::SphereTraceMulti(GetWorld(), PlayerSphere->GetComponentLocation(), GetActorLocation(), PulseSurfaceDynamicFloor_Radius, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::None, hits, true))
			{
				for (FHitResult hit : hits)
				{
					if (hit.Actor.Get()->IsA(APulseSurface::StaticClass()) && (CurrentActiveSurfaces.Find(Cast<APulseSurface>(hit.Actor.Get())) == INDEX_NONE))
					{
						Cast<APulseSurface>(hit.Actor.Get())->SetSurfaceActive(true);
						CurrentActiveSurfaces.AddUnique(Cast<APulseSurface>(hit.Actor.Get()));

						Cast<APulseSurface>(hit.Actor.Get())->SetIndicesActiveInRadiusFromPoint(GetActorLocation(), PulseSurfaceDynamicFloor_Radius);
					}
				}
			}
		}

		lastSurfaceActivationCheckTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	}
}

void APulseCharacter::PlayerSphere_Tick(float DeltaTime)
{
	FVector sphereTargetLoc = GetActorLocation();

	if(!MySurgeSpline)
		sphereTargetLoc = GetActorLocation() + (FMath::Lerp(FVector(0.f, 0.f, 0.f), (GetActorLocation() - lastPlayerWorldPosition).GetSafeNormal() * PlayerSphere_MaxWanderDist, FMath::Clamp(GetVelocity().Size() / GetCharacterMovement()->MaxWalkSpeed, 0.f, 1.f)));

	PlayerSphereHandle->SetTargetLocationAndRotation(sphereTargetLoc, GetActorRotation());
	if (bDebugPlayer) DrawDebugLine(GetWorld(), GetActorLocation(), sphereTargetLoc, FColor::Purple);

	if (bLerpingPlayerColor)
	{
		FLinearColor currColor;
		DynamicPlayerMaterial->GetVectorParameterValue("PlayerColor", currColor);

		currColor = FMath::Lerp(currColor, PlayerColor_Current, DeltaTime * PlayerColor_LerpSpeed);

		if ((FVector(currColor) - FVector(PlayerColor_Current)).Size() < 0.08f)
		{
			bLerpingPlayerColor = false;
			currColor = PlayerColor_Current;
		}

		UpdatePlayerColor(currColor);
		PlayerLight->SetLightColor(currColor.ToFColor(true), true);
	}

	if (bLerpingPlayerIntensity)
	{
		float currIntensity;
		DynamicPlayerMaterial->GetScalarParameterValue("PlayerIntensity", currIntensity);

		currIntensity = FMath::Lerp(currIntensity, PlayerIntensity_Current, DeltaTime * PlayerIntensity_LerpSpeed);

		if (FMath::IsNearlyEqual(currIntensity, PlayerIntensity_Current, 0.1f))
		{
			bLerpingPlayerIntensity = false;
			currIntensity = PlayerIntensity_Current;
		}

		UpdatePlayerIntensity(currIntensity);
	}

	if ((GetActorLocation() - GetPlayerSphereLocation()).SizeSquared() > FMath::Pow(PlayerSphere_MaxWanderDist + 20.f, 2.f))
	{
		PlayerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
		PlayerSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
	}
	else if(PlayerSphere->GetCollisionResponseToChannel(ECC_WorldDynamic) == ECR_Ignore)
	{
		PlayerSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
		PlayerSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	}

	lastPlayerWorldPosition = GetActorLocation();
}

APulseSurface* APulseCharacter::GetPulseSurfaceInDir(FVector CheckDir, bool checkIfValid)
{
	FHitResult hitSurface;
	if (UKismetSystemLibrary::SphereTraceSingle(GetWorld(), (PlayerSphere->GetComponentLocation() + (CheckDir * GetCapsuleComponent()->GetScaledCapsuleHalfHeight())), PlayerSphere->GetComponentLocation() + (CheckDir * (GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + (MySurgeSpline->IsValidLowLevel() ? PulseCheck_LineDistance_SurgeSpline : PulseCheck_LineDistance))), PlayerSphere->Bounds.SphereRadius, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), bDebugPlayer ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, hitSurface, true))
	{
		bool foundValidSurface = false;

		if (hitSurface.Actor.Get()->IsA(APulseSurface::StaticClass()))
		{
			if (!checkIfValid)
				foundValidSurface = true;
			else if (Cast<APulseSurface>(hitSurface.Actor.Get())->CanPulseSurfaceFromLocation(PlayerSphere->GetComponentLocation()))
				foundValidSurface = true;
		}

		if(foundValidSurface)
			return Cast<APulseSurface>(hitSurface.Actor.Get());
	}

	return nullptr;
}

APulseSurface* APulseCharacter::GetClosestPulseSurface(APulseSurface* ignoreSurface, bool checkIfValid)
{
	TArray<APulseSurface*> ignoreArray;
	if (ignoreSurface)
		ignoreArray.Add(ignoreSurface);

	return GetClosestPulseSurface(ignoreArray, checkIfValid);
}

APulseSurface* APulseCharacter::GetClosestPulseSurface(TArray<APulseSurface*> ignoreSurfaces, bool checkIfValid)
{
	TArray<FHitResult> results;
	APulseSurface* closest = nullptr;
	TArray<AActor*> ignoreActors;
	
	for (APulseSurface* surface : ignoreSurfaces)
	{
		ignoreActors.Add(surface);
	}

	if (UKismetSystemLibrary::SphereTraceMulti(GetWorld(), PlayerSphere->GetComponentLocation(), GetActorLocation(), PulseCheck_SphereRadius, ETraceTypeQuery::TraceTypeQuery1, false, ignoreActors, bDebugPlayer ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None, results, true))
	{
		float shortestDist = 1000000.f;
		bool currSurfaceIsValid = false;

		for (int i = 0; i < results.Num(); i++)
		{
			if (Cast<APulseSurface>(results[i].Actor.Get()))
			{
				if (!checkIfValid)
					currSurfaceIsValid = true;
				else if (Cast<APulseSurface>(results[i].Actor.Get())->CanPulseSurfaceFromLocation(PlayerSphere->GetComponentLocation()))
					currSurfaceIsValid = true;

				if (currSurfaceIsValid && (((results[i].ImpactPoint - GetActorLocation()).Size()) < shortestDist))
				{
					shortestDist = (results[i].ImpactPoint - GetActorLocation()).Size();
					closest = Cast<APulseSurface>(results[i].Actor.Get());
				}
			}
		}
		return closest;
	}

	return nullptr;
}

void APulseCharacter::SetCurrentPlayerSurface(APulseSurface* newSurface)
{
	if (!newSurface->IsValidLowLevel())
	{
		CurrentPlayerSurface = nullptr;
		PlayerSurfaceType = ESurfaceType::NONE;

		return;
	}

	if (CurrentPlayerSurface != newSurface)
	{
		CurrentPlayerSurface = newSurface;

		PrintToViewport(CurrentPlayerSurface->GetName(), FColor::Yellow, 1.f);

		if (PlayerSurfaceType != newSurface->GetSurfaceType())
		{
			PlayerSurfaceType = newSurface->GetSurfaceType();

			FLinearColor newColor;

			switch (PlayerSurfaceType)
			{
			case ESurfaceType::NONE:
				newColor = PlayerColor_Default;
				PrintToViewport("New Surface Type: NONE", FColor::White, 1.f);
				break;

			case ESurfaceType::RED:
				newColor = PlayerColor_RED;
				PrintToViewport("New Surface Type: RED", FColor::Red, 1.f);
				break;

			case ESurfaceType::GREEN:
				newColor = PlayerColor_GREEN;
				PrintToViewport("New Surface Type: GREEN", FColor::Green, 1.f);
				break;

			case ESurfaceType::BLUE:
				newColor = PlayerColor_BLUE;
				PrintToViewport("New Surface Type: BLUE", FColor::Blue, 1.f);
				break;

			case ESurfaceType::CUSTOM:
				newColor = PlayerColor_Default;
				PrintToViewport("New Surface Type: CUSTOM", newSurface->GetSurfaceCustomColor().ToFColor(true), 1.f);
				break;
			}

			SetLerpToPlayerColor(newColor);

			VFX_Charging->SetColorParameter(TEXT("Pulse_Color"), newColor * 10.f);
			VFX_Charged->SetColorParameter(TEXT("Pulse_Color"), newColor);
			VFX_Pulse->SetColorParameter(TEXT("Pulse_Color"), newColor);
			VFX_PulsedTrail->SetColorParameter(TEXT("Pulse_Color"), newColor);
		}
	}
}

bool APulseCharacter::Pulse()
{
	PrintToViewport("Pulse", FColor::Magenta, 2.f);

	bool bPulseSuccess = false;
	FLinearColor pulseColor;
	FRotator playerRotation = GetActorRotation();

	if (MySurgeSpline)
	{
		APulseSurface* newSurface = GetClosestPulseSurface(PreviousPulsedSplineSurface);
		if (newSurface)
		{
			PreviousPulsedSurfaceAttachedToSpline = newSurface;
			DisconnectFromSurgeSpline(false, true);
			SetCurrentPlayerSurface(newSurface);
		}
		else
		{
			DisconnectFromSurgeSpline(false, true);
			GetCharacterMovement()->Velocity = ((UKismetMathLibrary::GetForwardVector(playerRotation) * GetVelocity().Size() *  0.8f) + (UKismetMathLibrary::GetUpVector(playerRotation) * Pulse_GREEN_SurgeSpeed));
			pulseColor = PlayerColor_GREEN;
			bPulseSuccess = true;
		}

	}
	else if(MyBlueVolume)
	{
		APulseSurface* newSurface = GetClosestPulseSurface(MyBlueVolume);

		if (newSurface)
			SetCurrentPlayerSurface(newSurface);
		else
		{
			GetCharacterMovement()->AddImpulse(UKismetMathLibrary::GetForwardVector(GetControlRotation()) * Pulse_BLUE_BoostImpulse, true);
			pulseColor = PlayerColor_BLUE;
			bPulseSuccess = true;
		}
	}
	else if (!CurrentPlayerSurface)
	{
		APulseSurface* closestSurface;
		closestSurface = GetPulseSurfaceInDir(-GetActorUpVector());
		if (closestSurface)
		{
			SetCurrentPlayerSurface(closestSurface);
		}
		else
			SetCurrentPlayerSurface(GetClosestPulseSurface());
	}

	if (!bPulseSuccess)
	{
		switch (PlayerSurfaceType)
		{
		case ESurfaceType::NONE:
			SetLerpToPlayerColor(PlayerColor_Default);
			return false;

		case ESurfaceType::RED:
			pulseColor = PlayerColor_RED;

			if (CurrentPlayerSurface->IsA(APulseSurface_Spline::StaticClass()))
				GetCharacterMovement()->Velocity = ((GetVelocity().Size() * 0.5f) * (UKismetMathLibrary::GetForwardVector(playerRotation))) + ((GetVelocity().Size() * 0.5f) * UKismetMathLibrary::GetUpVector(Cast<APulseSurface_Spline>(CurrentPlayerSurface)->GetSurfaceSpline()->GetRotationAtDistanceAlongSpline(Cast<APulseSurface_Spline>(CurrentPlayerSurface)->GetClosestSplineDistanceToWorldLocation(GetPlayerSphereLocation()), ESplineCoordinateSpace::World)));
			else
				GetCharacterMovement()->AddImpulse(UKismetMathLibrary::GetUpVector(CurrentPlayerSurface->GetStaticMeshInstanceComponent()->GetComponentRotation()) * ((MyBlueVolume) ? (Pulse_RED_JumpImpulse * 1.25f) : Pulse_RED_JumpImpulse), true);
			
			break;

		case ESurfaceType::GREEN:
			pulseColor = PlayerColor_GREEN;
			if (Cast<APulseSurface_Spline>(CurrentPlayerSurface))
				ConnectToSurgeSpline(Cast<APulseSurface_Spline>(CurrentPlayerSurface));
			break;

		case ESurfaceType::BLUE:
			pulseColor = PlayerColor_BLUE;

			if (!MyBlueVolume) GetCharacterMovement()->AddImpulse(CurrentPlayerSurface->GetActorUpVector() * Pulse_BLUE_ActivateImpulse, true);
			break;

		case ESurfaceType::CUSTOM:
			SetLerpToPlayerColor(PlayerColor_Default);
			return false;
			break;
		}

		SetCurrentPulseType(PlayerSurfaceType);
		CurrentPlayerSurface->PulseSurface(PlayerSphere->GetComponentLocation());
		bPulseSuccess = true;
	}

	if (bPulseSuccess)
	{
		SetLerpToPlayerColor(PlayerColor_Default, pulseColor * 5.f);
		SetLerpToPlayerIntensity(PlayerIntensity_Default, PlayerIntensity_Pulse);

		if(!MyBlueVolume) VFX_Pulse->Activate(false);
		VFX_PulsedTrail->Activate(false);

		lastPulseTime = UGameplayStatics::GetTimeSeconds(GetWorld());

		if (bPulseChargePressed)
		{
			bPulseChargePressed = false;
			bWantsToPulse = true;
		}
	}

	SetCurrentPlayerSurface(nullptr);

	return bPulseSuccess;
}

void APulseCharacter::SetCurrentPulseType(ESurfaceType newPulseType)
{
	CurrentPulseType = newPulseType;

	switch (newPulseType)
	{
		case ESurfaceType::NONE:
			PreviousPulsedSplineSurface = NULL;
			PreviousPulsedSurfaceAttachedToSpline = NULL;
			VFX_PulsedTrail->Deactivate();
			VFX_Pulse->Deactivate();
			break;

		case ESurfaceType::RED:
			break;

		case ESurfaceType::GREEN:
			break;

		case ESurfaceType::BLUE:
			break;
	}
}

void APulseCharacter::Pulse_Tick(float DeltaTime)
{
	if (bWantsToPulse && ((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) > Pulse_Cooldown))
		PulseCharge_Pressed();

	if (MySurgeSpline && PreviousPulsedSplineSurface)
	{
		if (currSurgeSplineDistance >= MySurgeSpline->GetSplineLength())
		{
			DisconnectFromSurgeSpline(true, true);
		}
		else if (currSurgeSplineDistance >= 0 && currSurgeSplineDistance < MySurgeSpline->GetSplineLength())
		{
			SetActorLocation(MySurgeSpline->GetWorldLocationAtDistanceAlongSpline(currSurgeSplineDistance) +
				(UKismetMathLibrary::GetUpVector(MySurgeSpline->GetRotationAtDistanceAlongSpline(currSurgeSplineDistance, ESplineCoordinateSpace::World)) *
				((PreviousPulsedSplineSurface->GetSurfaceMesh()->GetBounds().BoxExtent.Z * 0.5f) +
					GetCapsuleComponent()->GetScaledCapsuleHalfHeight() +
					Pulse_GREEN_OffsetFromSpline) *
					MySurgeSpline->GetScaleAtDistanceAlongSpline(currSurgeSplineDistance)));

			SetActorRotation(MySurgeSpline->GetRotationAtDistanceAlongSpline(currSurgeSplineDistance, ESplineCoordinateSpace::World), ETeleportType::None);

			currSurgeSplineDistance += (DeltaTime * Pulse_GREEN_SurgeSpeed);
			CameraFocusTargetLoc = MySurgeSpline->GetWorldLocationAtDistanceAlongSpline(currSurgeSplineDistance + (Pulse_GREEN_SurgeSpeed * 0.3f));
			VFX_PulsedTrail->Activate(false);
		}
	}
	else
	{
		if (MyBlueVolume)
			VFX_PulsedTrail->Activate(false);
		else
			VFX_PulsedTrail->Deactivate();
	}

	if (!MySurgeSpline && PreviousPulsedSplineSurface)
	{
		bool stopTryingToSurge = false;

		if ((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) < Pulse_Duration_GREEN)
		{
			PrintToViewport("Searching for new Spline...", FColor::Orange);

			TArray<APulseSurface*> ignoreSurfaces;
			ignoreSurfaces.Add(PreviousPulsedSplineSurface);
			ignoreSurfaces.Add(PreviousPulsedSurfaceAttachedToSpline);

			if (APulseSurface* nearbySurface = GetClosestPulseSurface(ignoreSurfaces))
			{
				if (Cast<APulseSurface_Spline>(nearbySurface) && Cast<APulseSurface_Spline>(nearbySurface)->GetSurfaceType() == ESurfaceType::GREEN)
				{
					ConnectToSurgeSpline(Cast<APulseSurface_Spline>(nearbySurface));
					PrintToViewport("Found new Spline!");
				}
			}
		}
		else
		{
			SetCurrentPulseType(ESurfaceType::NONE);
			PlayerSphereHandle->SetLinearStiffness(PlayerSphere_HandleLinearStiffness_Default);
			PrintToViewport("...no Splines found.", FColor::Orange, 2.f);
		}

	}

	// Keep track of the Pulse Duration
	if (!MySurgeSpline && CurrentPulseType > ESurfaceType::NONE)
	{
		float duration = Pulse_Cooldown;

		switch (CurrentPulseType)
		{
		case ESurfaceType::RED:
			duration = Pulse_Duration_RED;
			break;
		case ESurfaceType::GREEN:
			duration = Pulse_Duration_GREEN;
			break;
		case ESurfaceType::BLUE:
			duration = Pulse_Duration_GREEN;
		}
			
		if ((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) >= duration)
			SetCurrentPulseType(ESurfaceType::NONE);
		else if (((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - lastPulseTime) >= VFX_Pulse_Duration))
			VFX_Pulse->Deactivate();
	}
}

void APulseCharacter::ConnectToSurgeSpline(APulseSurface_Spline* newSurgeSpline)
{
	CurrentPulseType = ESurfaceType::NONE;
	SetCurrentPlayerSurface(newSurgeSpline);

	SetLerpToPlayerColor(PlayerColor_Default, PlayerColor_GREEN * 5.f);
	SetLerpToPlayerIntensity(PlayerIntensity_Default, PlayerIntensity_Pulse);

	PreviousPulsedSplineSurface = newSurgeSpline;
	MySurgeSpline = PreviousPulsedSplineSurface->GetSurfaceSpline();
	currSurgeSplineDistance = newSurgeSpline->GetClosestSplineDistanceToWorldLocation(GetActorLocation());
	bCameraFocusTargetPressed = true;
	bLerpingCameraFOV = true;
	CameraFOV_CurrentTarget = CameraFOV_Surge;
	CameraFocusTargetSpeed_Current = CameraFocusTargetSpeed_Surge;
	PlayerSphereHandle->SetLinearStiffness(PlayerSphere_HandleLinearStiffness_Surge);
}

void APulseCharacter::DisconnectFromSurgeSpline(bool reachedEnd, bool canReconnect)
{
	MySurgeSpline = nullptr;
	SetCurrentPlayerSurface(nullptr);
	bCameraFocusTargetPressed = false;
	CameraFOV_CurrentTarget = CameraFOV_Default;
	CameraFocusTargetSpeed_Current = CameraFocusTargetSpeed_Default;
	SetMyFocusTarget(MyFocusTarget);

	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f), ETeleportType::TeleportPhysics);

	if (reachedEnd)
	{
		FVector surgeLaunchDir = UKismetMathLibrary::GetForwardVector(PreviousPulsedSplineSurface->GetLaunchArrow()->GetComponentRotation());
		float launchImpulse = GetVelocity().Size(),
			dotLaunchToDown = FVector::DotProduct(FVector(0.f, 0.f, -1.f), surgeLaunchDir);

		if (dotLaunchToDown > 0.f)
			launchImpulse *= FMath::Clamp((1.f - dotLaunchToDown), 0.08f, 1.f);
		
		currSurgeSplineDistance = 0.f;
		GetCharacterMovement()->Velocity = surgeLaunchDir * launchImpulse;
		SetLerpToPlayerIntensity(PlayerIntensity_Default, PlayerIntensity_Pulse);
		SetLerpToPlayerColor(PlayerColor_Default, PlayerColor_GREEN);
		
		if(canReconnect) SetCurrentPulseType(ESurfaceType::GREEN);
	}

	if (canReconnect)
		lastPulseTime = UGameplayStatics::GetRealTimeSeconds(GetWorld());
	else
		SetCurrentPulseType(ESurfaceType::NONE);
}

void APulseCharacter::CameraFocusOnTarget_Pressed()
{
	if (MySurgeSpline)
		return;

	bCameraFocusTargetPressed = true;
	bLerpingCameraFOV = true;
	CameraFOV_CurrentTarget = CameraFOV_Focus;
}

void APulseCharacter::CameraFocusOnTarget_Released()
{
	if (MySurgeSpline)
		return;

	bCameraFocusTargetPressed = false;
	CameraFOV_CurrentTarget = CameraFOV_Default;
}

void APulseCharacter::CameraFocusOnTarget_Tick(float DeltaTime)
{
	if (!GetController())
		return;

	FRotator targetLookDir = (CameraFocusTargetLoc - (FollowCamera->GetComponentLocation())).Rotation(),
			currentLookDir = GetController()->GetControlRotation(),
			newLookDir = FMath::Lerp(currentLookDir, targetLookDir, DeltaTime * CameraFocusTargetSpeed_Current);
		
	GetController()->SetControlRotation(newLookDir);
}

void APulseCharacter::SetMyFocusTarget(APulseFocusTarget* newTarget)
{
	if (!newTarget)
		return;

	MyFocusTarget = newTarget;
	CameraFocusTargetLoc = MyFocusTarget->GetActorLocation();
}

void APulseCharacter::SetLerpToPlayerColor(FLinearColor newTargetColor)
{
	PlayerColor_Current = newTargetColor;
	bLerpingPlayerColor = true;
}

void APulseCharacter::SetLerpToPlayerColor(FLinearColor newTargetColor, FLinearColor hardSetCurrColor)
{
	UpdatePlayerColor(hardSetCurrColor);
	SetLerpToPlayerColor(newTargetColor);
}

void APulseCharacter::SetLerpToPlayerIntensity(float newTargetIntensity)
{
	PlayerIntensity_Current = newTargetIntensity;
	bLerpingPlayerIntensity = true;
}

void APulseCharacter::SetLerpToPlayerIntensity(float newTargetIntensity, float hardSetCurrIntensity)
{
	UpdatePlayerIntensity(hardSetCurrIntensity);
	SetLerpToPlayerIntensity(newTargetIntensity);
}

void APulseCharacter::UpdatePlayerColor(FLinearColor PlayerColor) { DynamicPlayerMaterial->SetVectorParameterValue(TEXT("PlayerColor"), PlayerColor); }

void APulseCharacter::UpdatePlayerIntensity(float PlayerIntensity) { DynamicPlayerMaterial->SetScalarParameterValue(TEXT("PlayerIntensity"), PlayerIntensity); }

void APulseCharacter::UpdatePlayerQuiverSpeed_Master(float MasterQuiverSpeed) { DynamicPlayerMaterial->SetScalarParameterValue(TEXT("MasterQuiverSpeed"), MasterQuiverSpeed); }

void APulseCharacter::UpdatePlayerQuiverSpeed_Movement(float MovementQuiverSpeed) { DynamicPlayerMaterial->SetScalarParameterValue(TEXT("MovementQuiverSpeed"), MovementQuiverSpeed); }

void APulseCharacter::UpdatePlayerQuiverDistance(float QuiverDistance) { DynamicPlayerMaterial->SetScalarParameterValue(TEXT("QuiverDistance"), QuiverDistance); }

void APulseCharacter::SetBlueVolumeRef(APulseSurface* newBlueVolume) { MyBlueVolume = newBlueVolume; }

void APulseCharacter::QuitGame()
{
	PrintToViewport("Quitting game....", FColor::Red, 4.f);
	FGenericPlatformMisc::RequestExit(false);
}

void APulseCharacter::PrintToViewport(FString toPrint, FColor printColor, float printDuration)
{
	if (GEngine && bDebugPlayer)
		GEngine->AddOnScreenDebugMessage(-1, printDuration, printColor, toPrint);
}