#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OpenableDoor.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REALITYONE_API UOpenableDoor : public UActorComponent
{
	GENERATED_BODY()

// Methods //
public:	
	UOpenableDoor();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
		void OpenDoor();

	UFUNCTION(BlueprintCallable)
		void CloseDoor();

	UFUNCTION(BlueprintCallable)
		void ChangeCondition(bool NewValue);

	// Internal methods //
private:
	void RegisterDelegate();
	void SetupTimelineCurve();

	UFUNCTION()
		void TimelineFloatReturn(float val);

	UFUNCTION()
		void TimelineFinishedCallback();

	UFUNCTION()
		void OnBeginTriggerOverlap(AActor * OverlappedActor, class AActor* OtherActor);


// Properties //
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		class ATriggerBox* TriggerBox = nullptr;

	// Use this one to set conditional at what point it is allowed to open the door
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		bool ConditionalParameter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		class USoundBase* OpenDoorSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
		class USoundBase* CloseDoorSound;

	// Internal properties //
private:
	bool IsReversing = false;
	bool bIsOpen = false;
	AActor* Door = nullptr;

	UPROPERTY()
		class UTimelineComponent* BlenableTimeline = nullptr;

	UPROPERTY()
		class UCurveFloat* fCurve = nullptr;

		
};
