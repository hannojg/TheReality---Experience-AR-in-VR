#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AIHelperBotComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REALITYONE_API UAIHelperBotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AI_Behavior")
		TArray<AActor*> GetNavWayPoints() const;


	// Variables and states 
private:
	UPROPERTY(EditInstanceOnly, Category = "AI_Behavior")
		TArray<AActor*> NavWayPoints;
		
};
