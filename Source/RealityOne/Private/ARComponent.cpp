#include "ARComponent.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstance.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "CoreUObject/Public/UObject/ConstructorHelpers.h"
#include "CoreUObject/Public/UObject/UObjectGlobals.h"
#include "Components/PrimitiveComponent.h"

UARComponent::UARComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Setup everything that is necessary in order to play a given curve 

	static ConstructorHelpers::FObjectFinder<UCurveFloat> Curvy(TEXT("CurveFloat'/Game/VirtualReality/Framework/AREffects/CF_MaterialTransition.CF_MaterialTransition'"));
	check(Curvy.Succeeded());

	fCurve = Curvy.Object;
}


void UARComponent::BeginPlay()
{
	FOnTimelineFloat onTimelineCallback;
	FOnTimelineEventStatic onTimelineFinishedCallback;

	Super::BeginPlay();
	if (fCurve != NULL)
	{
		BlenableTimeline = NewObject<UTimelineComponent>(this, FName("TimelineAnimation"));

		BlenableTimeline->CreationMethod = EComponentCreationMethod::UserConstructionScript; // Indicate it comes from a blueprint so it gets cleared when we rerun construction scripts
		BlenableTimeline->SetNetAddressable();	// This component has a stable name that can be referenced for replication

		BlenableTimeline->SetPropertySetObject(this); // Set which object the timeline should drive properties on
		BlenableTimeline->SetDirectionPropertyName(FName("TimelineDirection"));

		BlenableTimeline->SetLooping(false);
		BlenableTimeline->SetTimelineLength(3.0f);
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


void UARComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// This function must be called in order to use AR function and effects.
// You need to specify a volume and whether you want to also toggle
// the collision of the proided volume.
void UARComponent::SetupAR(UPrimitiveComponent* Volume, bool bOverrideCollision)
{
	this->bOverrideCollision = bOverrideCollision;
	this->Volume = Volume;
	if (ensure(Volume)) // is a validity check
	{
		// set volume by default inivisible
		Volume->SetVisibility(false, true);
		ToggleCollision();
	}
}


// Appearing effect and helper functions //

void UARComponent::StartAppearing()
{
	if (Volume == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could't initialize AR effects as ObjectRef has not been set yet on %s"), *GetOwner()->GetName())
		return;
	}
	bDisappear = false;
	CollectMaterialInfo();
	SetupDynamicMaterial();
	PlayTimelineCurve();
	ToggleCollision();
}

void UARComponent::Disappear(bool bAlsoDestroyActor)
{
	bDisappear = true;
	bDisappearDestroyActor = bAlsoDestroyActor;

	if (Volume->GetNumMaterials() <= 0)
	{
		CollectMaterialInfo();
	}

	SetupDynamicMaterial();

	if (BlenableTimeline != nullptr
		&& fCurve != nullptr)
	{
		BlenableTimeline->Reverse();
	}
}

void UARComponent::CollectMaterialInfo()
{
	if (Volume == nullptr) { return; }

	// Set Visibility of objects [it has been set to invisible (in BP)] //
	Volume->SetVisibility(true, true);

	NumMaterials = Volume->GetNumMaterials();


	for (int32 i = 0; i < NumMaterials; i++)
	{
		OriginalMaterials.Add(Volume->GetMaterial(i));
	}
}

void UARComponent::SetupDynamicMaterial()
{
	// Set dynamic material instance in order to manipulate params of material instance //
	if (BlendMaterial != nullptr)
	{
		BlendMaterialDynamic = Volume->CreateAndSetMaterialInstanceDynamicFromMaterial(0, BlendMaterial);

		// Apply dynamic material to all material slots of object
		for (int32 i = 0; i < NumMaterials; i++)
		{
			Volume->SetMaterial(i, BlendMaterialDynamic);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could't initialize AR effects as BlendMaterial was not set. Check BP setup of childed BP component!"))
		return;
	}
}

void UARComponent::PlayTimelineCurve()
{
	if (BlenableTimeline != nullptr
		&& fCurve != nullptr)
	{
		BlenableTimeline->Play();
	}
}

void UARComponent::TimelineFloatReturn(float val)
{
	if (BlendMaterialDynamic != nullptr)
	{
		BlendMaterialDynamic->SetScalarParameterValue(FName{ TEXT("BlendValue") }, val);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could't UPDATE AR appearing effect. BlendMaterialDynamic instance was null. Check code!"))
	}
}

void UARComponent::TimelineFinishedCallback()
{
	if (Volume == nullptr) { return; }

	if (!bDisappear)
	{
		// Reapply previous materials //
		for (int32 i = 0; i < NumMaterials; i++)
		{
			Volume->SetMaterial(i, OriginalMaterials[i]);
		}
	}
	else
	{
		ToggleCollision();
		Volume->SetVisibility(false, true);
		if (bDisappearDestroyActor)
		{
			if(Volume->GetOwner())
			{
				Volume->GetOwner()->Destroy();
			}
		}
	}
}

void UARComponent::ToggleCollision()
{
	if (bOverrideCollision)
	{
		if (!OriginalCollisionType.IsSet())
		{
			OriginalCollisionType = Volume->GetCollisionEnabled();

			// if there hasn't been set a OriginalCollisionType yet we can safly assume that 
			// there hasn't been yet any modifications to the collision.
			// So, as a default behavior in this case we would like to remove/disable the
			// collision by changing the CollicionType
			Volume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		} 
		else
		{
			Volume->SetCollisionEnabled(OriginalCollisionType.GetValue());
			OriginalCollisionType.Reset();
		}
		
	}
}