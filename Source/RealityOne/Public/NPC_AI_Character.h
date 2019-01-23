#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AIHelperBotComponent.h"
#include "NPC_AI_Character.generated.h"

UCLASS()
class REALITYONE_API ANPC_AI_Character : public ACharacter
{
	GENERATED_BODY()

public:
	ANPC_AI_Character();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
