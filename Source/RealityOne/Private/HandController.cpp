#include "HandController.h"
#include "XRMotionControllerBase.h"
#include "MotionControllerComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

AHandController::AHandController()
{
	PrimaryActorTick.bCanEverTick = true;


	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
}

AHandController * AHandController::Create(UObject * WorldContextObject, USceneComponent * AttachTo, const FName TrackingSource, UClass * Class)
{
	UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject);
	if (!Class) Class = AHandController::StaticClass();
	auto* HandController = World->SpawnActor<AHandController>(Class);
	if (!ensure(HandController)) return nullptr;

	HandController->SetHandPosition(TrackingSource);
	HandController->AttachToComponent(AttachTo, FAttachmentTransformRules::KeepRelativeTransform);

	return HandController;
}


void AHandController::BeginPlay()
{
	Super::BeginPlay();
}

void AHandController::SetHandPosition(FName TrackingSource)
{
	HandPosition = (TrackingSource.IsEqual(FXRMotionControllerBase::RightHandSourceId)) ? EControllerHand::Right : EControllerHand::Left;
}


void AHandController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHandController::GripPressed()
{
	GripButtonPressed();
}

void AHandController::GripReleased()
{
	GripButtonReleased();
}

void AHandController::TriggerPressed()
{
	TriggerButtonPressed();
}

void AHandController::TriggerReleased()
{
	TriggerButtonReleased();
}
