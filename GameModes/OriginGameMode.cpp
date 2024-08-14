#include "OriginGameMode.h"
#include "Project_Taco/OriginCharacter.h"
#include "Project_Taco/PlayerController/OriginPlayerController.h"

void AOriginGameMode::PlayerEliminated(class AOriginCharacter* ElimmedCharacter, class AOriginPlayerController* VictimController, AOriginPlayerController* AttackerController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
	}
}