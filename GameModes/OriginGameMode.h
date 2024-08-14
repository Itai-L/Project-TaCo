#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "OriginGameMode.generated.h"

/**
 *
 */
UCLASS()
class PROJECT_TACO_API AOriginGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class AOriginCharacter* ElimmedCharacter, class AOriginPlayerController* VictimController, AOriginPlayerController* AttackerController);
};