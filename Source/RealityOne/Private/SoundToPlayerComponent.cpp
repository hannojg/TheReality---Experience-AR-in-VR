#include "SoundToPlayerComponent.h"
#include "Engine/TriggerBox.h"
#include "Kismet/GameplayStatics.h"

USoundToPlayerComponent::USoundToPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USoundToPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	RegisterDelegate();
}

void USoundToPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void USoundToPlayerComponent::RegisterDelegate()
{
	TriggerBox->OnActorBeginOverlap.AddDynamic(this, &USoundToPlayerComponent::OnBeginTriggerOverlap);
}

void USoundToPlayerComponent::OnBeginTriggerOverlap(AActor * OverlappedActor, AActor * OtherActor)
{
	if (OtherActor->GetName().Contains("BP_VRCharacter")
		&& !bHasPlayed)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SoundToPlay, OtherActor->GetActorLocation());
		bHasPlayed = true;
	}
}

