#include "VRCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/PostProcessComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "InputCore/Classes/InputCoreTypes.h"
#include "XRMotionControllerBase.h"
#include "HeadMountedDisplay/Public/MotionControllerComponent.h"
#include "HandController.h"
#include "Engine/Classes/Components/SceneComponent.h"

AVRCharacter::AVRCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(VRRoot);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(VRRoot);

	LeftTracker = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftTracker"));
	LeftTracker->SetTrackingMotionSource(FXRMotionControllerBase::LeftHandSourceId);
	LeftTracker->MotionSource = FXRMotionControllerBase::LeftHandSourceId;
	LeftTracker->SetTrackingSource(EControllerHand::Left);
	LeftTracker->SetupAttachment(VRRoot);

	RightTracker = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightTracker"));
	RightTracker->SetTrackingMotionSource(FXRMotionControllerBase::RightHandSourceId);
	RightTracker->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	RightTracker->SetTrackingSource(EControllerHand::Right);
	RightTracker->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	DestinationMarkerInner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarkerInner"));
	DestinationMarkerInner->SetupAttachment(DestinationMarker);

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("UPostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Correct hight tracking //
	GetCapsuleComponent()->AddLocalOffset(FVector(0.0, 0.0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

	ToggleVisibilityTeleportMarker(false);

	// Spawn Controller //

	if (BlinkerMaterialBase != nullptr
		&& DebugShowBlinker)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
	}

	LeftController = AHandController::Create(this, LeftTracker, FXRMotionControllerBase::LeftHandSourceId, HandControllerClass);
	RightController = AHandController::Create(this, RightTracker, FXRMotionControllerBase::RightHandSourceId, HandControllerClass);
	OnControllerCreated();

	// Spawn Hands //
	LeftHand = AVRHand::Create(this, LeftTracker, EControllerHand::Left, VRHandClass);
	RightHand = AVRHand::Create(this, RightTracker, EControllerHand::Right, VRHandClass);
	RightHand->OnPointingWidgetEvent.AddUniqueDynamic(this, &AVRCharacter::HandWidgetPointing);
	LeftHand->PairHands(RightHand);
	OnHandsCreated();
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector CameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	CameraOffset.Z = 0;
	AddActorWorldOffset(CameraOffset);
	VRRoot->AddWorldOffset(-CameraOffset);

	if (bIsTeleportActivated)
	{
		UpdateTeleporter();
	}
	else
	{
		DisableDestinationMarker();
		DisableTeleportSpline();
	}
	
	
	if (DebugShowBlinker) {
		UpdateBlinkers();
	}

	CheckIfArGlassGetsMounted();
}

void AVRCharacter::CheckIfArGlassGetsMounted()
{
	FHitResult Scan = GetFirstPhysicsBodyInReach(FName(TEXT("ARGlass")));
	AActor* HitActor = Scan.GetActor();
	if (HitActor)
	{
		if (!bIsGlassMounted) 
		{
			// call event to blueprint
			OnARGlassMounted();
		}
		bIsGlassMounted = true;
	}
}

// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("SkipAroundOwnAxis"), this, &AVRCharacter::SkipAroundOwnAxis);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Pressed, this, &AVRCharacter::EnableTeleporter);
	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRCharacter::GripPressedLeft);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRCharacter::GripReleaseLeft);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRCharacter::GripReleasedRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRCharacter::GripReleaseRight);
	PlayerInputComponent->BindAction(TEXT("TriggerLeft"), IE_Pressed, this, &AVRCharacter::TriggerPressedLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerLeft"), IE_Released, this, &AVRCharacter::TriggerReleasedLeft);
	PlayerInputComponent->BindAction(TEXT("TriggerRight"), IE_Pressed, this, &AVRCharacter::TriggerPressedRight);
	PlayerInputComponent->BindAction(TEXT("TriggerRight"), IE_Released, this, &AVRCharacter::TriggerReleasedRight);

}

void AVRCharacter::HandWidgetPointing(FHitResult HitResult, bool bHitWidget)
{
	if (bHitWidget)
	{
		bIsPointingCurrently = true;
		TArray<FVector> Path;
		Path.Add(HitResult.TraceStart);
		Path.Add(HitResult.ImpactPoint);
		HitResult.GetActor()->GetActorLocation();
		DrawTeleportPath(Path);
	}
	else
	{
		bIsPointingCurrently = false;
		DisableTeleportSpline();
	}
}

void AVRCharacter::DisableDestinationMarker()
{
	ToggleVisibilityTeleportMarker(false);
	return;
}

void AVRCharacter::DisableTeleportSpline()
{
	if (!bIsPointingCurrently) 
	{
		TArray<FVector> EmptyPath;
		DrawTeleportPath(EmptyPath);
	}
}

void AVRCharacter::UpdateTeleporter()
{
	// We don't want to update draws on the teleporter stuff.
	// This also modifies our spline. The spline is also used for pointing
	// so we don't want to interfere these two.
	if (bIsPointingCurrently)
	{
		DisableDestinationMarker();
		return;
	}

	FVector Location;
	TArray<FVector> Path;
	bool bHasDestination = FindTeleportDestination(Path, Location);

	if (bHasDestination)
	{
		ToggleVisibilityTeleportMarker(true);

		DestinationMarker->SetWorldLocation(Location);

		DrawTeleportPath(Path);
	}
	else
	{
		ToggleVisibilityTeleportMarker(false);
		DisableTeleportSpline();
	}
}

bool AVRCharacter::FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation)
{
	FVector Start = RightController->GetActorLocation();
	FVector LookDirection = RightController->GetActorForwardVector();

	FPredictProjectilePathParams Params(
		TeleportProjectileRadius,
		Start,
		LookDirection * TeleportProjectileSpeed,
		TeleportSimulateTime,
		ECollisionChannel::ECC_Camera,
		this
	);
	Params.bTraceComplex = true;
	FPredictProjectilePathResult ProjectileHitResult;
	bool bHasHitResult = UGameplayStatics::PredictProjectilePath(
		this,
		Params,
		ProjectileHitResult
	);

	for (auto& point : ProjectileHitResult.PathData) {
		OutPath.Add(point.Location);
	}

	if (!bHasHitResult) { return false; }

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLocation;
	bool bOnNavMesh = NavSystem->ProjectPointToNavigation(ProjectileHitResult.HitResult.Location, NavLocation, TeleportProjectionExtent);

	if (!bOnNavMesh) { return false; }

	OutLocation = NavLocation.Location;

	return true;
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector>& Path)
{
	UpdateSpline(Path);

	if (!TeleportArchMesh || !TeleportArchMaterial) {
		UE_LOG(LogTemp, Error, TEXT("Can't draw teleport path as material and mesh haven't been set!"))
	}

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 SegmentNum = Path.Num() - 1;
	for (int32 i = 0; i < SegmentNum; i++) {
		if (TeleportPathMeshPool.Num() <= i) {
			USplineMeshComponent* DynamicSplineMesh = NewObject<USplineMeshComponent>(this);
			DynamicSplineMesh->SetMobility(EComponentMobility::Movable);
			DynamicSplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			DynamicSplineMesh->SetStaticMesh(TeleportArchMesh);
			DynamicSplineMesh->SetMaterial(0, TeleportArchMaterial);
			DynamicSplineMesh->RegisterComponent();

			TeleportPathMeshPool.Add(DynamicSplineMesh);
		}

		USplineMeshComponent* DynamicSplineMesh = TeleportPathMeshPool[i];
		DynamicSplineMesh->SetVisibility(true);

		FVector StartPos, StartTangent, EndPos, EndTangent;
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPos, EndTangent);
		DynamicSplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent);
	}
}

void AVRCharacter::UpdateSpline(const TArray<FVector> &Path)
{
	TeleportPath->ClearSplinePoints(false);
	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint Point(i, LocalPosition, ESplinePointType::Curve);
		TeleportPath->AddPoint(Point, false);
	}
	TeleportPath->UpdateSpline();
}

void AVRCharacter::UpdateBlinkers()
{
	if (!ensure(RadiusVsVelocity)) { return; }
	if (!ensure(BlinkerMaterialInstance)) { return; }

	float Speed = GetVelocity().Size();
	float RadiusVsSpeed = RadiusVsVelocity->GetFloatValue(Speed);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("ParamRadius"), RadiusVsSpeed);

	FVector2D Center = GetBlinkerCenter();
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("ParamCenter"), FLinearColor(Center.X, Center.Y, 0));
}

FVector2D AVRCharacter::GetBlinkerCenter()
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();
	if (MovementDirection.IsNearlyZero())
	{
		return FVector2D(0.5, 0.5);
	}

	FVector WorldStationaryLocation;

	if (FVector::DotProduct(Camera->GetForwardVector(), MovementDirection) > 0)
	{
		// Forward movement
		WorldStationaryLocation = Camera->GetComponentLocation() + MovementDirection * 100;
	}
	else
	{
		//Going backwards
		WorldStationaryLocation = Camera->GetComponentLocation() - MovementDirection * 100;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC == nullptr)
	{
		return FVector2D(0.5, 0.5);
	}

	FVector2D ScreenStationaryLocation;
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);

	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;

	return ScreenStationaryLocation;
}

/**
 * This call toggles the visibility of the teleportation marker.
 * It should be used instead of setting it directly because it
 * prevents the marker from being redrawn every frame!
*/
void AVRCharacter::ToggleVisibilityTeleportMarker(bool SetVisibility)
{
	if (!ensure(DestinationMarker)) { return; }

	if (DestinationMarker->IsVisible()
		&& !SetVisibility)
	{
		DestinationMarker->SetVisibility(false);
		DestinationMarkerInner->SetVisibility(false);
	}
	else if (!DestinationMarker->IsVisible()
		&& SetVisibility)
	{
		DestinationMarker->SetVisibility(true);
		DestinationMarkerInner->SetVisibility(true);
	}
}

void AVRCharacter::MoveForward(float throttle)
{
	if(throttle > 0.05 || throttle < -0.05)
	{
		AddMovementInput(Camera->GetForwardVector() * throttle);
	}
}

void AVRCharacter::MoveRight(float throttle)
{
	if (throttle > 0.05 || throttle < -0.05)
	{
		AddMovementInput(Camera->GetRightVector() * throttle);
	}
}

void AVRCharacter::SkipAroundOwnAxis(float throttle)
{
	float RotationDegree = 30;

	throttle = FMath::Clamp(throttle, -0.8f, 0.8f);

	if (!bIsRotating
			&& (throttle <= -0.8 || throttle >= 0.8))
	{
		bIsRotating = true;
		NewRotation = Camera->GetComponentRotation();
		NewRotation.Roll = 0.0;
		NewRotation.Pitch = 0.0;

		NewRotation.Yaw = (throttle < 0) ? -30: 30;
			
		StartFade(0, 1, OwnAxisRotationFadeTime);

		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishSkipAroundOwnAxis, OwnAxisRotationFadeTime - 0.05);
	
	} 
	else if (bIsRotating
				&& (throttle > -0.2 && throttle < 0.2)) 
	{
		bIsRotating = false;
	}

}

void AVRCharacter::FinishSkipAroundOwnAxis()
{
	VRRoot->AddLocalRotation(NewRotation);

	StartFade(1, 0, OwnAxisRotationFadeTime);
}

void AVRCharacter::GripPressedLeft()
{
	LeftController->GripPressed();
	LeftHand->GripPressed();
}

void AVRCharacter::GripReleaseLeft()
{
	LeftController->GripReleased();
	LeftHand->GripReleased();
}

void AVRCharacter::GripReleasedRight()
{
	RightController->GripPressed();
	RightHand->GripPressed();
}

void AVRCharacter::GripReleaseRight()
{
	RightController->GripReleased();
	RightHand->GripReleased();
}

void AVRCharacter::TriggerPressedLeft()
{
	LeftController->TriggerPressed();
}

void AVRCharacter::TriggerReleasedLeft()
{
	LeftController->TriggerReleased();
}

void AVRCharacter::TriggerPressedRight()
{
	RightController->TriggerPressed();
}

void AVRCharacter::TriggerReleasedRight()
{
	RightController->TriggerReleased();
}

void AVRCharacter::BeginTeleport()
{
	if (!DestinationMarker->IsVisible()) return;

	TeleportLocation = DestinationMarker->GetComponentLocation();
	
	StartFade(0, 1, TeleportFadeTime);

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, TeleportFadeTime - 0.1);
	DisableTeleporter();
}

void AVRCharacter::FinishTeleport()
{
	TeleportLocation += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetActorUpVector();

	SetActorLocation(TeleportLocation);
	StartFade(1, 0, TeleportFadeTime);
	
}

void AVRCharacter::DoBlackScreenTransition(float TimeToStayBlack, float TransitionTime)
{
	TimeCache = TransitionTime;
	StartFade(0, 1, TransitionTime, true);
	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishDoBlackscreenTransition, TimeToStayBlack - 0.01);
}

void AVRCharacter::FinishDoBlackscreenTransition()
{
	StartFade(1, 0, TimeCache);
}


void AVRCharacter::StartFade(float FromAlpha, float ToAlpha, float duration, bool bHoldIfFinished)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC != nullptr)
	{
		PC->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, duration, FLinearColor::Black, false, bHoldIfFinished);
	}
}

// Line Tracing //
TPair<FVector, FRotator> AVRCharacter::GetPlayerViewPoint() const
{
	FVector PlayerViewPointLocation;
	FRotator PlayerViewPointRotation;
	GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(
		OUT PlayerViewPointLocation,
		OUT PlayerViewPointRotation
	);

	return TPair<FVector, FRotator>(PlayerViewPointLocation, PlayerViewPointRotation);
}

FVector AVRCharacter::GetPlayerViewPointLocation() const
{
	return GetPlayerViewPoint().Key;
}

FRotator AVRCharacter::GetPlayerViewPointRotator() const
{
	return GetPlayerViewPoint().Value;
}

FVector AVRCharacter::GetPlayerViewPointRotation() const
{
	return GetPlayerViewPointRotator().Vector();
}

// Toggles the teleporter vor VR movement on or off. (By default the teleporter is activated)
void AVRCharacter::ToggleTeleporter()
{
	bIsTeleportActivated = !bIsTeleportActivated;
}

void AVRCharacter::DisableTeleporter()
{
	bIsTeleportActivated = false;
}

void AVRCharacter::EnableTeleporter()
{
	bIsTeleportActivated = true;
}

FVector AVRCharacter::PlayerViewLineTrace() const
{
	return GetPlayerViewPointLocation() + GetPlayerViewPointRotation() * Reach;
}

FHitResult AVRCharacter::GetFirstPhysicsBodyInReach(FName TAG) const
{
	/// Setup query parameters
	FCollisionQueryParams TraceParameters(TAG, false, this);

	FHitResult LineTraceHit;

	GetWorld()->LineTraceSingleByObjectType(
		LineTraceHit,
		GetPlayerViewPointLocation(),
		PlayerViewLineTrace(),
		FCollisionObjectQueryParams(ECollisionChannel::ECC_PhysicsBody),
		TraceParameters
	);

	return LineTraceHit;
}