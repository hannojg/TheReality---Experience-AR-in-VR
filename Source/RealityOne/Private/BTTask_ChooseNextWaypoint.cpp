#include "BTTask_ChooseNextWaypoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPC_AI_Character.h"
#include "AIHelperBotComponent.h"

EBTNodeResult::Type UBTTask_ChooseNextWaypoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{	
	// Get the way points
	auto AIController = OwnerComp.GetAIOwner();
	auto AIComponent = AIController->GetPawn()->FindComponentByClass<UAIHelperBotComponent>();
	auto Waypoints = AIComponent->GetNavWayPoints();
	if (!ensure(AIComponent)) { return EBTNodeResult::Failed; }

	// Prepare for setting next waypoint by getting index
	auto BlackboardComp = OwnerComp.GetBlackboardComponent();
	auto Index = BlackboardComp->GetValueAsInt(IndexKey.SelectedKeyName);

	// check if waypoints have been set
	if (Waypoints.Num() <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No way points have been set for AI"));
		return EBTNodeResult::Failed;
	}

	// Set actual waypoint object to be refered in behavior tree
	BlackboardComp->SetValueAsObject(WaypointKey.SelectedKeyName, Waypoints[Index]);

	// Cycle the index
	auto NextIndex = (Index + 1) % Waypoints.Num();
	BlackboardComp->SetValueAsInt(IndexKey.SelectedKeyName, NextIndex);

	return EBTNodeResult::Succeeded;
}