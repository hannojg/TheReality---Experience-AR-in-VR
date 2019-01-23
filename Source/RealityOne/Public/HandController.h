#pragma once

#include "CoreMinimal.h"
#include "InputCore/Classes/InputCoreTypes.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Engine/Classes/Engine/SkeletalMesh.h"
#include "HandController.generated.h"

UCLASS()
class REALITYONE_API AHandController : public AActor
{
	GENERATED_BODY()
	
public:	
	AHandController();

	static AHandController* Create(UObject* WorldContextObject, USceneComponent* AttachTo, const FName TrackingSource, UClass* Class);

	void GripPressed();
	void GripReleased();
	void TriggerPressed();
	void TriggerReleased();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "VRHand")
		EControllerHand HandPosition;

	UFUNCTION(BlueprintImplementableEvent, Category = "VRInput")
		void GripButtonPressed();

	UFUNCTION(BlueprintImplementableEvent, Category = "VRInput")
		void GripButtonReleased();

	UFUNCTION(BlueprintImplementableEvent, Category = "VRInput")
		void TriggerButtonPressed();

	UFUNCTION(BlueprintImplementableEvent, Category = "VRInput")
		void TriggerButtonReleased();


protected:
	virtual void BeginPlay() override;
	void SetHandPosition(FName TrackingSource);

public:	
	virtual void Tick(float DeltaTime) override;

};
