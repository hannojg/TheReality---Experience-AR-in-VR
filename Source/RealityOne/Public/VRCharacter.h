#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "HandController.h"
#include "VRHand.h"
#include "VRCharacter.generated.h"


UCLASS()
class REALITYONE_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVRCharacter();

	virtual void Tick(float DeltaTime) override;

	void CheckIfArGlassGetsMounted();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UFUNCTION()
		void IsHandPointing(FHitResult HitResult, bool bIsPointing);

	FPointingEvent OnPointingEvent;

	UFUNCTION(BlueprintImplementableEvent, Category = "VRHands")
		void OnControllerCreated();

	UFUNCTION(BlueprintImplementableEvent, Category = "VRHands")
		void OnHandsCreated();

	// This function gets called as soon as the user pulls the Actor with the TAG "ARGlass" towards his/her face
	UFUNCTION(BlueprintImplementableEvent, Category = "VRAREvents")
		void OnARGlassMounted();

	UFUNCTION(BlueprintCallable, Category = "VRAREvents")
		void DoBlackScreenTransition(float TimeToStayBlack, float TransitionTime);

	UFUNCTION(BLueprintPure, Category = "VRCharacter")
		FVector GetPlayerViewPointLocation() const;
	
	UFUNCTION(BLueprintPure, Category = "VRCharacter")
		FVector GetPlayerViewPointRotation() const;

	UFUNCTION(BlueprintCallable, Category = "VRCharacter")
		void ToggleTeleporter();

	UFUNCTION(BlueprintCallable, Category = "VRCharacter")
		void DisableTeleporter();

	UFUNCTION(BlueprintCallable, Category = "VRCharacter")
		void EnableTeleporter();

protected:
	virtual void BeginPlay() override;

protected: // PROPERTIES //

	// Camera and VRRoot //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		class USceneComponent* VRRoot;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Input")
		float OwnAxisRotationFadeTime = 0.3;

	// Controller and Hands //

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		class UMotionControllerComponent* LeftTracker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		class UMotionControllerComponent* RightTracker;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		AHandController* LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		AHandController* RightController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		AVRHand* LeftHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VRCharacter")
		AVRHand* RightHand;

	UPROPERTY(BlueprintReadWrite, Category = "VRCharacter")
		bool bIsPointingCurrently = false;


	// Teleportation //

	UPROPERTY(VisibleAnywhere, Category = "VRTeleport")
		class USplineComponent* TeleportPath;

	UPROPERTY(VisibleAnywhere, Category = "VRTeleport")
		class UStaticMeshComponent* DestinationMarker;

	UPROPERTY(VisibleAnywhere, Category = "VRTeleport")
		class UStaticMeshComponent* DestinationMarkerInner;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		float TeleportFadeTime = 1;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		FVector TeleportProjectionExtent = FVector(100, 100, 100);

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		float TeleportProjectileRadius = 10;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		float TeleportProjectileSpeed = 800;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "VRTeleport")
		float TeleportSimulateTime = 2;

	// Blinkers (FOV) //

	// You can tick this option in order to turn on/off blinkers (FOV) for development
	UPROPERTY(EditDefaultsOnly, Category = "VRBlinker")
		bool DebugShowBlinker = true;

	UPROPERTY(EditDefaultsOnly, Category = "VRBlinker")
		class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditDefaultsOnly, Category = "VRBlinker")
		class UCurveFloat* RadiusVsVelocity;

	// AR Glass //
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "VRARGlasses")
		bool bIsGlassMounted = false;


private:
	// Movement and player input //
	void MoveForward(float throttle);
	void MoveRight(float throttle);
	void SkipAroundOwnAxis(float throttle);
	void FinishSkipAroundOwnAxis();

	void GripPressedLeft();
	void GripReleaseLeft();
	void GripReleasedRight(); 
	void GripReleaseRight();

	void TriggerPressedLeft();
	void TriggerReleasedLeft();
	void TriggerPressedRight();
	void TriggerReleasedRight();

	// internal functions //
	void DisableDestinationMarker();
	void DisableTeleportSpline();
	void UpdateTeleporter();
	bool FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation);
	void DrawTeleportPath(const TArray<FVector> &Path);
	void UpdateSpline(const TArray<FVector> &Path);
	void UpdateBlinkers();
	FVector2D GetBlinkerCenter();
	void ToggleVisibilityTeleportMarker(bool SetVisibility);

	void BeginTeleport();
	void FinishTeleport();
	void FinishDoBlackscreenTransition();
	void StartFade(float FromAlpha, float ToAlpha, float duration, bool bHoldIfFinished = false);

	TPair<FVector, FRotator> GetPlayerViewPoint() const;
	FRotator GetPlayerViewPointRotator() const;
	FVector PlayerViewLineTrace() const;
	FHitResult GetFirstPhysicsBodyInReach(FName Tag) const;

private: //Configuration Parameters
	UPROPERTY()
		class UPostProcessComponent* PostProcessComponent;

	UPROPERTY()
		class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY()
		TArray<class USplineMeshComponent*> TeleportPathMeshPool;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AHandController> HandControllerClass;

	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<AVRHand> VRHandClass;

private: // Local states
	FVector TeleportLocation = FVector();
	FRotator NewRotation = FRotator();
	bool bIsRotating = false;
	float Reach = 7.f;
	float TimeCache = 1.f; // in use when handling DoBlackScreenTransition
	bool bIsTeleportActivated = true;

};
