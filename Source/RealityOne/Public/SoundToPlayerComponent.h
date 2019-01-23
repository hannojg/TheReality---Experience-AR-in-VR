#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoundToPlayerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REALITYONE_API USoundToPlayerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USoundToPlayerComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		class USoundBase* SoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		class ATriggerBox* TriggerBox = nullptr;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void RegisterDelegate();

	UFUNCTION()
		void OnBeginTriggerOverlap(AActor * OverlappedActor, class AActor* OtherActor);

	bool bHasPlayed = false;

		
};
