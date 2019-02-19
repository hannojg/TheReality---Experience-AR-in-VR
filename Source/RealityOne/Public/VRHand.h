#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InputCore/Classes/InputCoreTypes.h"
#include "VRHand.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPointingWidgetEvent, FHitResult, HitResult, bool, bHitWidget);

UCLASS()
class REALITYONE_API AVRHand : public AActor
{
	GENERATED_BODY()

public:
	FPointingWidgetEvent OnPointingWidgetEvent;
	
public:	
	AVRHand();

	static AVRHand* Create(UObject* WorldContextObject, USceneComponent* AttachTo, const EControllerHand Hand, UClass* Class);
	void SetHandPosition(const EControllerHand Hand);

	UFUNCTION(BlueprintCallable, Category = "VRHandController")
		void PlayerIsPointing(bool bIsPointing);
	
	void GripPressed();
	void GripReleased();

	void PairHands(AVRHand* VRHand);

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "VRHandController")
		class UHapticFeedbackEffect_Base* HapticEffectClimbable;

	UPROPERTY(BlueprintReadOnly, Category = "VRHandController")
		class UWidgetInteractionComponent* WidgetInteraction;

private:
	// Callbacks
	UFUNCTION()
		void ActorBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
		void ActorEndOverlap(AActor* OverlappedActor, AActor* OtherActor);

private:
	// Helpers
	bool CanClimb() const;

private:
	// States
	bool bCanClimb = false;
	AActor* GrabActorCache = nullptr;

	bool bIsClimbing = false;
	FVector ClimbingStartLocation;
	EControllerHand HandPosition;

	AVRHand* OtherVRHand;

};
