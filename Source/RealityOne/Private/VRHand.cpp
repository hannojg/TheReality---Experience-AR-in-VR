#include "VRHand.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Runtime/UMG/Public/Components/WidgetInteractionComponent.h"
#include "Runtime/SlateCore/Public/Layout/WidgetPath.h"
#include "Engine/Engine.h"

AVRHand::AVRHand()
{
	PrimaryActorTick.bCanEverTick = true;

	WidgetInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("WidgetInteractionComp"));
	WidgetInteraction->Activate();
}

AVRHand * AVRHand::Create(UObject * WorldContextObject, USceneComponent * AttachTo, const EControllerHand Hand, UClass * Class)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!Class) Class = AVRHand::StaticClass();
	auto* VRHand = World->SpawnActor<AVRHand>(Class);
	if (!ensure(VRHand)) return nullptr;
	VRHand->SetHandPosition(Hand);
	FAttachmentTransformRules AttachmentRules = {
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		EAttachmentRule::SnapToTarget,
		true
	};
	VRHand->AttachToComponent(AttachTo, AttachmentRules);

	// "rotate" hand mesh for left hand (it's the same, the inverted scale leads to the opposite hand model)
	if (Hand == EControllerHand::Left)
	{
		VRHand->SetActorScale3D(FVector{ 1.0, -1.0, 1.0 });
	}


	return VRHand;
}

void AVRHand::SetHandPosition(const EControllerHand Hand)
{
	HandPosition = Hand;
}

/// This one is obsolote. It is refered to a pointing gesture.
void AVRHand::PlayerIsPointing(bool bIsPointing)
{
	UE_LOG(LogTemp, Warning, TEXT("Calling playerIsPointing. This is an obsolete method!"))
}

void AVRHand::GripPressed()
{
	if (!bCanClimb) return;

	if (!bIsClimbing)
	{
		bIsClimbing = true;
		ClimbingStartLocation = GetActorLocation();

		OtherVRHand->bIsClimbing = false;

		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
		if (Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
		}
	}
}

void AVRHand::GripReleased()
{
	if (bIsClimbing)
	{
		bIsClimbing = false;

		ACharacter* Character = Cast<ACharacter>(GetAttachParentActor());
		if (Character != nullptr)
		{
			Character->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
		}
	}
}

void AVRHand::BeginPlay()
{
	Super::BeginPlay();
	

	OnActorBeginOverlap.AddDynamic(this, &AVRHand::ActorBeginOverlap);
	OnActorEndOverlap.AddDynamic(this, &AVRHand::ActorEndOverlap);
}

void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsClimbing)
	{
		FVector HandControllerDelta = GetActorLocation() - ClimbingStartLocation;
		GetAttachParentActor()->AddActorWorldOffset(-HandControllerDelta);
	}

	// check whether hand is pointing at a dialogue widget
	FHitResult HitResult = WidgetInteraction->GetLastHitResult();

	OnPointingWidgetEvent.Broadcast(
							HitResult,
							WidgetInteraction->GetHoveredWidgetPath().IsValid()
						);
}

void AVRHand::ActorBeginOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	// Climbing //

	bool bNewCanClimb = CanClimb();

	if (!ensure(HapticEffectClimbable)) { return; }

	if (!bCanClimb && bNewCanClimb)
	{
		GetWorld()->GetFirstPlayerController()->PlayHapticEffect(
			HapticEffectClimbable,
			HandPosition
		);
	}
	bCanClimb = bNewCanClimb;
}

void AVRHand::ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	bCanClimb = CanClimb();
}

bool AVRHand::CanClimb() const
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);


	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (OverlappingActor->ActorHasTag(TEXT("Climbable")))
		{
			return true;
		}
	}

	return false;
}

void AVRHand::PairHands(AVRHand* VRHand)
{
	OtherVRHand = VRHand;
	OtherVRHand->OtherVRHand = this;
}
