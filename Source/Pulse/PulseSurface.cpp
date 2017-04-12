// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseCharacter.h"
#include "PulseSurface.h"

// Sets default values
APulseSurface::APulseSurface()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	StartPoint = CreateDefaultSubobject<USceneComponent>(TEXT("StartPoint"));
	RootComponent = StartPoint;

	MyInstance = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MyInstance"));
	MyInstance->SetMobility(EComponentMobility::Movable);
	MyInstance->SetupAttachment(RootComponent);
	MyInstance->bCastDynamicShadow = false;
	this->AddInstanceComponent(MyInstance);

	struct ConstructorHelpers::FObjectFinder<UStaticMesh> findMesh(TEXT("StaticMesh'/Game/CreatedAssets/Meshes/PulseSurfaceMesh.PulseSurfaceMesh'"));
	SurfaceMeshOverride = findMesh.Object;

	BlueVolume_PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("BlueVolume_PostProcess"));
	BlueVolume_PostProcess->bUnbound = false;
	BlueVolume_PostProcess->bUseAttachParentBound = true;
	BlueVolume_PostProcess->bEnabled = false;

	SurfaceType = ESurfaceType::NONE;
	bIsSurfaceActive = true;
	SurfaceColor_PulseIntensity = 50.f;
	SurfaceColor_PulseSpeedScale = 0.f;
	CustomSurfaceColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	SurfaceSpread = 40.f;
	SurfaceSectionScale_XY = 0.4f;
	SurfaceSectionScale_Z = 1.f;
	SurfaceOscillationLength = 60.f;
	SurfaceOscillationSpeed = 1.f;
	SurfaceOscillationWaveIntensity = 1.f;
	SurfaceHeight = 0.f;
	RequiredMinUpDotToPulse = 0.f;
	SurfaceUpdateInterval = 0.f;
	
	bIsDynamicFloor = false;
	bForceUpdateSurface = false;
	bDisableSurfaceCollision = false;
	bShouldUpdateSurfaceCollision = false;

	CheckForPlayerInterval = 0.5f;
	HighQualityDistance = 10000.f;
	HybernateDistance = 35000.f;
	MaxLowQualityUpdateInterval = 0.5f;

	bIsHybernating = false;
	bOverrideHybernationDistance = false;
}

void APulseSurface::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bDisableSurfaceCollision) bShouldUpdateSurfaceCollision = false;

	if (SurfaceMeshOverride && (!MyInstance->GetStaticMesh() || MyInstance->GetStaticMesh() != SurfaceMeshOverride))
		MyInstance->SetStaticMesh(SurfaceMeshOverride);

	if (!MyInstance->GetStaticMesh())
	{
		bForceUpdateSurface = false;
		return;
	}

	FLinearColor newColor;

	switch (SurfaceType)
	{
		case ESurfaceType::NONE:
			newColor = FLinearColor::Black;
			break;
		case ESurfaceType::RED:
			newColor = FLinearColor::Red;
			break;

		case ESurfaceType::GREEN:
			newColor = FLinearColor::Green;
			break;

		case ESurfaceType::BLUE:
			newColor = FLinearColor(0.f, 0.2f, 1.f, 1.f);
			SurfaceColor_PulseIntensity = 100.f;
			break;

		case ESurfaceType::CUSTOM:
			newColor = CustomSurfaceColor;
			break;
	}

	DynamicSurfaceMaterial = UMaterialInstanceDynamic::Create(MyInstance->GetStaticMesh()->GetMaterial(1), this);
	DynamicSurfaceMaterial->SetVectorParameterValue(TEXT("PulseColor"), newColor);
	DynamicSurfaceMaterial->SetScalarParameterValue(TEXT("PulseIntensity"), FMath::FloatSelect(SurfaceType == ESurfaceType::NONE, SurfaceColor_PulseIntensity, 0.f));
	DynamicSurfaceMaterial->SetScalarParameterValue(TEXT("PulseSpeedScale"), SurfaceColor_PulseSpeedScale);
	MyInstance->SetMaterial(1, DynamicSurfaceMaterial);

	if (CreateSurface())
	{
		PrintToViewport(TEXT("Creating Surface..."), FColor::Yellow, 5.0f);

		HighQualityDistance = MyInstance->CalcBounds(MyInstance->GetRelativeTransform()).SphereRadius  + 1500.f;

		if (!bOverrideHybernationDistance)
			HybernateDistance = HighQualityDistance + 20000.f;
	}
}

// Called when the game starts or when spawned
void APulseSurface::BeginPlay()
{
	Super::BeginPlay();

	if (MyInstance)
	{
		//MyInstance->RegisterComponent();
		MyInstance->SetFlags(RF_Transactional);

		if(bDisableSurfaceCollision) MyInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	MyPlayer = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	lastSurfaceUpdateTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	FVector empty;
	GetActorBounds(false, PlayerCheckOrigin, empty);
	lastCheckForPlayerTime = UGameplayStatics::GetTimeSeconds(GetWorld());
}

void APulseSurface::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsSurfaceActive)
		return;

	if ((UGameplayStatics::GetTimeSeconds(GetWorld()) - lastCheckForPlayerTime) >= CheckForPlayerInterval)
	{
		CheckForPlayer();
	}

	if (bIsHybernating)
		return;

	if ((UGameplayStatics::GetTimeSeconds(GetWorld()) - lastSurfaceUpdateTime) >= updateInterval_current)
	{
		UpdateSurface();
		lastSurfaceUpdateTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	}
}

void APulseSurface::CheckForPlayer()
{
	if (!MyPlayer)
		return;

	float playerDist = (MyPlayer->GetActorLocation() - PlayerCheckOrigin).SizeSquared();

	lastCheckForPlayerTime = UGameplayStatics::GetTimeSeconds(GetWorld());

	if (bIsDynamicFloorActive)
	{
		updateInterval_current = SurfaceUpdateInterval;
		bIsHybernating = false;
		PrintToViewport("Surface is Active", FColor::Yellow, CheckForPlayerInterval);
		return;
	}

	if (playerDist <= FMath::Pow(HighQualityDistance, 2.f))
	{
		bIsHybernating = false;
	} 
	else if (playerDist >= FMath::Pow(HybernateDistance, 2.f))
	{
		bIsHybernating = true;
	}
	else
	{
		updateInterval_current = FMath::Lerp(SurfaceUpdateInterval, MaxLowQualityUpdateInterval, FMath::Clamp(playerDist / FMath::Pow(HybernateDistance, 2.f), 0.f, 1.f));
		bIsHybernating = false;
	}

	PrintToViewport("Player Distance From " + GetName() + ":: " + FString::SanitizeFloat(FMath::Sqrt(playerDist)) + " Check Interval: " + (bIsHybernating ? "Hybernating" : (FString::SanitizeFloat(updateInterval_current)) + "s") , FColor::Yellow, CheckForPlayerInterval);
}

bool APulseSurface::CanCreateSurface() { return (MyInstance->GetInstanceCount() != GetMaxInstanceCount()); }

bool APulseSurface::CreateSurface()
{
	if (!MyInstance->GetStaticMesh())
		return false;

	if (!bForceUpdateSurface)
	{
		if (!CanCreateSurface())
			return false;
	}

	bForceUpdateSurface = false;
	MyInstance->ClearInstances();

	//Child Logic
	return true;
}

void APulseSurface::UpdateSurface() {}

void APulseSurface::SetSurfaceInstanceTransform(int index, FTransform newTransform, bool bForceUpdateCollision)
{
	if (!MyInstance)
		return;

	if (!bDisableSurfaceCollision && (bShouldUpdateSurfaceCollision || bForceUpdateCollision))
	{
		MyInstance->UpdateInstanceTransform(index, newTransform, false, true, false);
	}
	else 
	{
		FMatrix newMatrix = MyInstance->PerInstanceSMData[index].Transform;
		newMatrix.SetAxis(3, newTransform.GetLocation());
		MyInstance->PerInstanceSMData[index].Transform = newMatrix;
		MyInstance->MarkRenderStateDirty();
	}

	MyInstance->ReleasePerInstanceRenderData();
}

float APulseSurface::GetPlayerDistanceFromSurfaceSquared()
{
	return (MyPlayer->GetActorLocation() - MyInstance->Bounds.Origin).SizeSquared();
}

float APulseSurface::GetPlayerDistanceFromSurfaceSquared(int32 index) 
{ 
	FTransform instanceTrans;
	MyInstance->GetInstanceTransform(index, instanceTrans, true);

	return ((Cast<APulseCharacter>(MyPlayer)->GetPlayerSphereLocation() - (MyPlayer->GetActorUpVector() * MyPlayer->GetCapsuleComponent()->GetScaledCapsuleHalfHeight())) - instanceTrans.GetLocation()).SizeSquared();
}

void APulseSurface::SetSurfaceActive(bool newActive)
{
	bIsDynamicFloorActive = newActive;

	if (!newActive)
		ActiveIndices.Empty();
}

void APulseSurface::SetIndicesActiveInRadiusFromPoint(FVector point, float radius)
{
	ActiveIndices.Empty();

	ActiveIndices = MyInstance->GetInstancesOverlappingSphere(point, radius, true);
}

bool APulseSurface::IsIndexActive(int32 index)
{
	return (ActiveIndices.Find(index) != INDEX_NONE);
}

bool APulseSurface::CanPulseSurfaceFromLocation(FVector location)
{
	FBoxSphereBounds InstanceBounds = MyInstance->CalcBounds(MyInstance->GetRelativeTransform());
	FVector topOfSurface = InstanceBounds.Origin + (UKismetMathLibrary::GetUpVector(MyInstance->GetComponentRotation()) * InstanceBounds.BoxExtent.Z);
	topOfSurface = GetActorTransform().TransformPosition(topOfSurface);

	float currDot = FVector::DotProduct((location - topOfSurface).GetSafeNormal(), UKismetMathLibrary::GetUpVector(MyInstance->GetComponentRotation()));
	
	if(bDebugSurface) DrawDebugLine(GetWorld(), topOfSurface, MyPlayer->GetActorLocation(), FColor::White, false, -1.f, (uint8)'\000', 10.f);
	PrintToViewport("Current Player Dot to Surface Up: " + FString::SanitizeFloat(currDot) + " // Can Pulse = " + (currDot >= RequiredMinUpDotToPulse ? "true" : "false"), FColor::Yellow, 0.01f);
	
	return (currDot >= RequiredMinUpDotToPulse);
}

void APulseSurface::PulseSurface(FVector PulseLocation) {}

void APulseSurface::SetBlueVolumeActive(bool active) {}

void APulseSurface::PrintToViewport(FString toPrint, FColor printColor, float printDuration)
{
	if (GEngine && bDebugSurface)
		GEngine->AddOnScreenDebugMessage(-1, printDuration, printColor, toPrint);
}

