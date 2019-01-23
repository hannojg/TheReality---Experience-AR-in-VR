#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/TimelineComponent.h"
#include "ARComponent.generated.h"


UCLASS(Blueprintable)
class REALITYONE_API UARComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UARComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public: // external functions

	// blueprint messages C++ (internal) that object start now with AR appearing effect 
	UFUNCTION(BlueprintCallable, Category = "AREffects")
		void StartAppearing();

	// Let the volume disappear. Could also destroy actor once finished
	UFUNCTION(BlueprintCallable, Category = "AREffects")
		void Disappear(bool bAlsoDestroyActor = false);

	UFUNCTION(BlueprintCallable, Category = "AREffects")
		void SetupAR(UPrimitiveComponent* Volume, bool bOverrideCollision = true);

public: // external properties relations
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AREffects")
		class UPrimitiveComponent* Volume = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "AREffects")
		class UMaterialInterface* BlendMaterial;

private: // internal functions
	UFUNCTION()
		void TimelineFloatReturn(float val);

	UFUNCTION()
		void TimelineFinishedCallback();

	void CollectMaterialInfo(); 
	void SetupDynamicMaterial();
	void PlayTimelineCurve();

	// Depends whether SetupAR function was called with OverrideCollision parameter 
	void ToggleCollision();

private: // internal properties
	UPROPERTY()
		class UTimelineComponent* BlenableTimeline = nullptr;

	UPROPERTY()
		class UCurveFloat* fCurve = nullptr;

	UPROPERTY()
		class UMaterialInstanceDynamic* BlendMaterialDynamic = nullptr;

	int32 NumMaterials;
	TArray<UMaterialInterface*> OriginalMaterials;
	bool bIsFirstExecution = true;
	bool bOverrideCollision = true;
	bool bDisappear = false;
	bool bDisappearDestroyActor = false;
	TOptional<ECollisionEnabled::Type> OriginalCollisionType;
};
