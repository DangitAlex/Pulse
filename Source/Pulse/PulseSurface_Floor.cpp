// Fill out your copyright notice in the Description page of Project Settings.

#include "Pulse.h"
#include "PulseCharacter.h"
#include "PulseSurface_Floor.h"

APulseSurface_Floor::APulseSurface_Floor()
{
	Surface_X = 10;
	Surface_Y = 10;
	SurfaceType = ESurfaceType::RED;

	BlueVolume_Z = 500.f;
	BlueVolume_LerpSpeed = 0.8f;
	BlueVolume_LifeTime = 0.f;

	bBlueVolume_StartActive = false;
	bBlueVolume_isActive = false;
	bBlueVolume_isLerping = false;

	BlueVolume_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("BlueVolume"));
	BlueVolume_Box->SetupAttachment(MyInstance);
	BlueVolume_Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BlueVolume_Box->SetBoxExtent(FVector(0.01f));

	BlueVolume_PostProcess->SetupAttachment(BlueVolume_Box);

	BlueVolume_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlueBox"));
	BlueVolume_Mesh->SetupAttachment(RootComponent);
	BlueVolume_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BlueVolume_Mesh->SetWorldScale3D(FVector(0.01f));

	struct ConstructorHelpers::FObjectFinder<UStaticMesh> findMesh(TEXT("StaticMesh'/Game/CreatedAssets/Meshes/BlueBox.BlueBox'"));
	BlueVolumeMeshRef = findMesh.Object;

	BlueVolume_Box->OnComponentBeginOverlap.AddDynamic(this, &APulseSurface_Floor::OnActorEnterBlueVolume);
	BlueVolume_Box->OnComponentEndOverlap.AddDynamic(this, &APulseSurface_Floor::OnActorLeaveBlueVolume);
}

void APulseSurface_Floor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!bIsSurfaceActive) bIsDynamicFloor = false;

	if (SurfaceType == ESurfaceType::BLUE)
	{
		if (BlueVolume_Mesh)
		{
			BlueVolume_Mesh->SetStaticMesh(BlueVolumeMeshRef);

			if (BlueVolume_Box && BlueVolume_Mesh->GetStaticMesh())
			{
				SetBlueVolumeZRatio(1.f);

				BlueVolume_Mesh->SetRelativeLocationAndRotation(BlueVolume_Box->GetRelativeTransform().GetLocation(), BlueVolume_Box->GetRelativeTransform().GetRotation());
				BlueVolume_Mesh->SetWorldScale3D(FVector(BlueVolume_Box->GetScaledBoxExtent().X / (BlueVolume_Mesh->GetStaticMesh()->GetBounds().BoxExtent.X),
					BlueVolume_Box->GetScaledBoxExtent().Y / (BlueVolume_Mesh->GetStaticMesh()->GetBounds().BoxExtent.Y),
					BlueVolume_Box->GetScaledBoxExtent().Z / (BlueVolume_Mesh->GetStaticMesh()->GetBounds().BoxExtent.Z)));
			}
		}
	}
	else
	{ 
		bBlueVolume_StartActive = false;

		if (BlueVolume_PostProcess) BlueVolume_PostProcess->bEnabled = false;

		if (BlueVolume_Box)
		{
			BlueVolume_Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BlueVolume_Box->SetBoxExtent(FVector(0.01f));
		}
		
		if (BlueVolume_Mesh)
		{
			BlueVolume_Mesh->SetStaticMesh(NULL);
			BlueVolume_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			BlueVolume_Mesh->SetVisibility(false);
		}
	}
}

void APulseSurface_Floor::BeginPlay()
{
	Super::BeginPlay();

	if (SurfaceType == ESurfaceType::BLUE && BlueVolume_Mesh)
	{
		if (BlueVolume_Mesh->GetStaticMesh())
		{
			DynamicBlueVolumeMaterial = UMaterialInstanceDynamic::Create(BlueVolume_Mesh->GetStaticMesh()->GetMaterial(0), this);
			BlueVolume_Mesh->SetMaterial(0, DynamicBlueVolumeMaterial);
		}

		SetBlueVolumeZRatio(0.f);

		if (bBlueVolume_StartActive)
			SetBlueVolumeActive(true);
	}
}

void APulseSurface_Floor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (SurfaceType == ESurfaceType::BLUE)
	{

		if (bBlueVolume_isActive && BlueVolume_LifeTime > 0.f && (UGameplayStatics::GetTimeSeconds(GetWorld()) - lastBlueVolumeActivationTime >= BlueVolume_LifeTime))
			SetBlueVolumeActive(false);

		if (bBlueVolume_isLerping)
			UpdateBlueVolume_Tick(DeltaTime);
	}
}

bool APulseSurface_Floor::CreateSurface()
{
	if (!Super::CreateSurface())
		return false;

	for (int i = 0; i < Surface_Y; i++)
	{
		for (int j = 0; j < Surface_X; j++)
		{
			FVector ForwardOffset = FVector(1.f, 0.f, 0.f) * (i * ((MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().X * SurfaceSectionScale_XY) + SurfaceSpread)),
					RightOffset = FVector(0.f, 1.f, 0.f) * (j * ((MyInstance->GetStaticMesh()->GetBoundingBox().GetExtent().Y * SurfaceSectionScale_XY) + SurfaceSpread)),
					currLoc = ForwardOffset + RightOffset;

			currLoc.Z += SurfaceHeight;
			
			FVector currScale = FVector(SurfaceSectionScale_XY, SurfaceSectionScale_XY, SurfaceSectionScale_Z);

			MyInstance->AddInstance(FTransform(FRotator(0.f, 0.f, 0.f), currLoc, currScale));
		}
	}

	if(SurfaceType == ESurfaceType::BLUE) SetBlueVolumeZRatio((SurfaceType == ESurfaceType::BLUE) ? 1.f : 0.f);

	return true;
}

void APulseSurface_Floor::UpdateSurface()
{
	for (int i = 0; i < Surface_Y; i++)
	{
		for (int j = 0; j < Surface_X; j++)
		{
			int index = (i*Surface_X) + j;

			if (index >= 0 && index < GetMaxInstanceCount())
			{
				FTransform newInstanceTransform;
				MyInstance->GetInstanceTransform(index, newInstanceTransform);

				float currWaveZOffset = (SurfaceHeight + (FMath::Sin((UGameplayStatics::GetRealTimeSeconds(GetWorld()) - (index * (SurfaceOscillationWaveIntensity / 10.f))) * SurfaceOscillationSpeed) * SurfaceOscillationLength) - SurfaceOscillationLength);
				float newInstanceZ = 0.f;

				if (bIsDynamicFloor && bIsDynamicFloorActive && IsIndexActive(index))
				{
					APulseCharacter* MyPulseCharacter = Cast<APulseCharacter>(MyPlayer);

					if (MyPulseCharacter)
					{
						float playerDistanceSquared = GetPlayerDistanceFromSurfaceSquared(index);

						if (playerDistanceSquared <= FMath::Pow(MyPulseCharacter->GetDynamicFloorCapRadius(), 2.f))
						{
							newInstanceZ = 0.f;
							if (bDebugSurface) DrawDebugLine(GetWorld(), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()) + (UKismetMathLibrary::GetUpVector(MyInstance->GetComponentRotation()) * 500.f), FColor::Green, false, -1.f, (uint8)'\000', 10.f);
						}
						else
						{
							newInstanceZ = FMath::Lerp(0.f, currWaveZOffset, FMath::Clamp((playerDistanceSquared - FMath::Pow(MyPulseCharacter->GetDynamicFloorCapRadius(), 2.f)) / FMath::Pow(MyPulseCharacter->GetDynamicFloorRadius(), 2.f), 0.f, 1.f));
							if (bDebugSurface) DrawDebugLine(GetWorld(), GetActorTransform().TransformPosition(newInstanceTransform.GetLocation()), MyPlayer->GetActorLocation(), FColor::Yellow, false, -1.f, (uint8)'\000', 2.f);
						}
					}
				}
				else
					newInstanceZ = currWaveZOffset;

				FVector newInstanceLoc = newInstanceTransform.GetLocation();
				newInstanceLoc.Z = newInstanceZ;

				newInstanceTransform.SetLocation(newInstanceLoc);

				SetSurfaceInstanceTransform(index, newInstanceTransform, false);
			}
			else if (bDebugSurface)
			{
				PrintToViewport("Out of bounds Index: " + FString::FromInt(index), FColor::Red, 0.f);
			}
		}
	}
}

void APulseSurface_Floor::PulseSurface(FVector PulseLocation)
{
	if (SurfaceType == ESurfaceType::BLUE) SetBlueVolumeActive(true);
}

void APulseSurface_Floor::SetBlueVolumeActive(bool active)
{
	if (SurfaceType != ESurfaceType::BLUE)
		return;

	if (active)
	{
		bBlueVolume_isActive = true;
		bBlueVolume_isLerping = true;

		lastBlueVolumeActivationTime = UGameplayStatics::GetTimeSeconds(GetWorld());
	}
	else
	{
		bBlueVolume_isActive = false;
		bBlueVolume_isLerping = true;
	}
}

void APulseSurface_Floor::SetBlueVolumeZRatio(float ratio)
{
	if (SurfaceType != ESurfaceType::BLUE || !BlueVolume_Box || !BlueVolume_Mesh || !BlueVolume_PostProcess || !MyInstance || !MyInstance->GetStaticMesh())
		return;

	PrintToViewport("Lerping Blue Volume:: " + FString::SanitizeFloat(ratio), FColor::Yellow, .05f );

	FBoxSphereBounds InstanceBounds = MyInstance->CalcBounds(MyInstance->GetRelativeTransform());
	FVector topOfSurface = InstanceBounds.Origin + FVector(0.f, 0.f, (((BlueVolume_Z * ratio) + 15.f) + InstanceBounds.BoxExtent.Z));

	BlueVolume_Box->SetRelativeLocation(topOfSurface);

	FVector InstanceBoundsBox = InstanceBounds.BoxExtent;
	BlueVolume_Box->SetBoxExtent(FVector(InstanceBoundsBox.X, InstanceBoundsBox.Y, BlueVolume_Z * ratio));

	if (ratio < 0.15f)
	{
		BlueVolume_Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BlueVolume_PostProcess->bEnabled = false;

		BlueVolume_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BlueVolume_Mesh->SetVisibility(false);

	}
	else
	{
		BlueVolume_Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BlueVolume_PostProcess->bEnabled = true;

		BlueVolume_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BlueVolume_Mesh->SetVisibility(true);
	}

	if(DynamicBlueVolumeMaterial) DynamicBlueVolumeMaterial->SetScalarParameterValue("BlueVolumeZRatio", ratio);
}

void APulseSurface_Floor::UpdateBlueVolume_Tick(float DeltaTime)
{
	if(SurfaceType != ESurfaceType::BLUE || !BlueVolume_Box->IsValidLowLevel() || !BlueVolume_Mesh->IsValidLowLevel())
		return;

	float newZRatio = 0.f;

	if (bBlueVolume_isActive)
	{
		if (!FMath::IsNearlyEqual(FMath::Clamp(BlueVolume_Box->GetScaledBoxExtent().Z / BlueVolume_Z, 0.f, 1.f), 1.f, 0.01f))
			newZRatio = FMath::Lerp(FMath::Clamp(BlueVolume_Box->GetScaledBoxExtent().Z / BlueVolume_Z, 0.f, 1.f), 1.f, DeltaTime * BlueVolume_LerpSpeed);
		else
		{
			newZRatio = 1.f;
			bBlueVolume_isLerping = false;
		}
	}
	else
	{
		if (!FMath::IsNearlyEqual(BlueVolume_Box->GetScaledBoxExtent().Z, 0.f, 0.05f))
			newZRatio = FMath::Lerp(BlueVolume_Box->GetScaledBoxExtent().Z, 0.f, DeltaTime * BlueVolume_LerpSpeed) / BlueVolume_Z;
		else
		{
			newZRatio = 0.f;
			bBlueVolume_isLerping = false;
		}
	}

	SetBlueVolumeZRatio(newZRatio);
}

void APulseSurface_Floor::OnActorEnterBlueVolume(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndexint, bool bFromSweep, const FHitResult& SweepResult)
{
	if (SurfaceType != ESurfaceType::BLUE)
		return;

	if (!Cast<APulseCharacter>(OtherActor))
		return;

	if (OtherActor == MyPlayer)
	{
		PrintToViewport("Player Entered...", FColor::Blue, 2.f);

		MyPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		Cast<APulseCharacter>(MyPlayer)->SetBlueVolumeRef(this);
	}
}

void APulseSurface_Floor::OnActorLeaveBlueVolume(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (SurfaceType != ESurfaceType::BLUE)
		return;

	if (!Cast<APulseCharacter>(OtherActor))
		return;

	if (OtherActor == MyPlayer)
	{
		PrintToViewport("...and Player Exited.", FColor::Blue, 2.f);

		MyPlayer->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		if(Cast<APulseCharacter>(MyPlayer)->GetBlueVolumeRef() == this) Cast<APulseCharacter>(MyPlayer)->SetBlueVolumeRef(NULL);
	}
}

