#include "OpenableDoor.h"
#include "Engine/TriggerBox.h"
#include "CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Components/TimelineComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"

UOpenableDoor::UOpenableDoor()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Setup everything that is necessary in order to play a given curve 

	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curvy(TEXT("CurveFloat'/Game/Assets/AnimationCurves/CF_OpenDoor.CF_OpenDoor'"));
	check(Curvy.Succeeded());

	fCurve = Curvy.Object;
}

void UOpenableDoor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


void UOpenableDoor::BeginPlay()
{
	Super::BeginPlay();

	Door = GetOwner();

	RegisterDelegate();
	SetupTimelineCurve();
}

void UOpenableDoor::RegisterDelegate()
{
	if (TriggerBox)
	{
		TriggerBox->OnActorBeginOverlap.AddDynamic(this, &UOpenableDoor::OnBeginTriggerOverlap);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s can't get reference to BoxTrigger!"), *GetOwner()->GetName())
	}
}

void UOpenableDoor::SetupTimelineCurve()
{
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;
	if (fCurve != NULL)
	{
		BlenableTimeline = NewObject<UTimelineComponent>(this, FName("TimelineAnimation"));

		BlenableTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
		BlenableTimeline->SetNetAddressable();	// This component has a stable name that can be referenced for replication

		BlenableTimeline->SetPropertySetObject(this); // Set which object the timeline should drive properties on
		//BlenableTimeline->SetDirectionPropertyName(FName("TimelineFinishedCallback"));

		BlenableTimeline->SetLooping(false);
		BlenableTimeline->SetTimelineLength(1.6f);
		BlenableTimeline->SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

		BlenableTimeline->SetPlaybackPosition(0.0f, false);

		// Add the float curve to the timeline and connect it to your timelines's interpolation function
		onTimelineCallback.BindUFunction(this, FName{ TEXT("TimelineFloatReturn") });
		onTimelineFinishedCallback.BindUFunction(this, FName{ TEXT("TimelineFinishedCallback") });
		BlenableTimeline->AddInterpFloat(fCurve, onTimelineCallback);
		BlenableTimeline->SetTimelineFinishedFunc(onTimelineFinishedCallback);

		BlenableTimeline->RegisterComponent();
	}
}

void UOpenableDoor::OpenDoor()
{
	if (BlenableTimeline != nullptr
		&& fCurve != nullptr)
	{
		if (ConditionalParameter
			&& !bIsOpen)
		{
			IsReversing = false;
			bIsOpen = true;
			UGameplayStatics::PlaySoundAtLocation(this, OpenDoorSound, Door->GetActorLocation());
			BlenableTimeline->Play();
		}
		
	}
}

void UOpenableDoor::CloseDoor()
{
	if (BlenableTimeline != nullptr
		&& fCurve != nullptr)
	{
		if (ConditionalParameter
			&& bIsOpen)
		{
			IsReversing = true;
			bIsOpen = false;
			UGameplayStatics::PlaySoundAtLocation(this, CloseDoorSound, Door->GetActorLocation());
			BlenableTimeline->Play();
		}
	}
}

void UOpenableDoor::ChangeCondition(bool NewValue)
{
	ConditionalParameter = NewValue;
}

void UOpenableDoor::TimelineFloatReturn(float val)
{
	if (Door != nullptr)
	{
		float UpdatedZ = Door->GetActorLocation().Z;
		if (IsReversing)
		{
			UpdatedZ -= val;
		}
		else
		{
			UpdatedZ += val;
		}

		FVector UpdatedLocation = FVector(Door->GetActorLocation().X, Door->GetActorLocation().Y, UpdatedZ);
		Door->SetActorLocation(UpdatedLocation);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't open door, actor was null!"))
	}
}

void UOpenableDoor::TimelineFinishedCallback()
{
}

void UOpenableDoor::OnBeginTriggerOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	if (OtherActor->GetName().Contains("VRCharacter"))
	{
		OpenDoor();
	}
}


