#include "HealthCommand.h"

void HealthChatCommand(AShooterPlayerController* aShooterPlayerController, FString* msg, EChatSendMode::Type mode)
{
	auto result = HealthInternal(aShooterPlayerController, ArkLibrary::GetCommand(CommandName_Health_Chat));
	if (result.size() == 0) return;

	for (auto str : result) ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", str.c_str());
}

void HealthConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog)
{
	auto aShooterPlayerController = static_cast<AShooterPlayerController*>(aPlayerController);

	auto result = HealthInternal(aShooterPlayerController, ArkLibrary::GetCommand(CommandName_Health_Console));
	if (result.size() == 0) return;

	std::wstringstream ss;
	for (auto str : result) ss << ArkApi::Tools::ConvertToWideStr(str) << std::endl;
	
	ArkApi::GetApiUtils().SendNotification(aShooterPlayerController, { 1, 0, 1, 1 }, 0.8f, 30.0f, nullptr, ss.str().c_str());
}

std::list<std::string> HealthInternal(AShooterPlayerController* aShooterPlayerController, ArkLibrary::CommandDefinition *cmd)
{
	std::list<std::string> result;
	
	UWorld* world = ArkApi::GetApiUtils().GetWorld();
	if (!world) return result;
	if (!aShooterPlayerController) return result;
	
	ACharacter* character = aShooterPlayerController->CharacterField()();
	if (!character || !character->IsA(APrimalCharacter::GetPrivateStaticClass())) return result;
	
	APrimalCharacter* primalCharacter = static_cast<APrimalCharacter*>(character);
	
	std::map<std::string, std::list<std::tuple<float, float>>> statuses;
	FVector* pos = new FVector();
	int teamId = aShooterPlayerController->TargetingTeamField()();
	FVector playerPos = aShooterPlayerController->DefaultActorLocationField()();
	
	TArray<AActor*>* FoundActors = new TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(reinterpret_cast<UObject *>(world), APrimalDinoCharacter::GetPrivateStaticClass(), FoundActors);
	
	for (uint32_t i = 0; i < FoundActors->Num(); i++)
	{
		AActor* actor = (*FoundActors)[i];
	
		APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);
	
		int dinoTeam = dino->TargetingTeamField()();
	
		dino->RootComponentField()()->GetCustomLocation(pos);
		if (dinoTeam == teamId && ArkLibrary::IsPointInside2dCircle(*pos, playerPos.X, playerPos.Y, 5000))
		{
			FString className;
			dino->DinoNameTagField()().ToString(&className); //species name
			//std::string name = dino->GetTamedNameField().ToString(); //tamed name
	
			UPrimalCharacterStatusComponent* status = dino->MyCharacterStatusComponentField()();
			if (status)
			{
				float* currentStatValues = status->CurrentStatusValuesField()();
				float* maxStatsValues = status->MaxStatusValuesField()();

				std::string classNameStr = className.ToString();
	
				if (statuses.find(classNameStr) == statuses.end())
				{
					auto list = std::list<std::tuple<float, float>>();
					list.push_back(std::make_tuple(currentStatValues[0], maxStatsValues[0]));
					statuses[classNameStr] = list;
				}
				else
				{
					statuses[classNameStr].push_back(std::make_tuple(currentStatValues[0], maxStatsValues[0]));
				}
			}
		}
	}
	delete FoundActors;
	delete pos;
	
	for (std::map<std::string, std::list<std::tuple<float, float>>>::const_iterator it = statuses.begin(), end = statuses.end(); it != end; ++it)
	{
		std::string name = it->first;
		std::list<float> percentages;
		double totalHealth = 0.0;
		double totalRemaining = 0.0;
		double totalDiff = 0.0;
	
		for (std::list<std::tuple<float, float>>::const_iterator it2 = it->second.begin(), end2 = it->second.end(); it2 != end2; ++it2)
		{
			float currentHealth = std::get<0>(*it2);
			float maxHealth = std::get<1>(*it2);
			float healthPercentage = currentHealth / maxHealth;
			float healthDiff = maxHealth - currentHealth;
	
			totalHealth += maxHealth;
			totalRemaining += currentHealth;
			totalDiff += healthDiff;
	
			percentages.push_back(healthPercentage);
		}
	
		percentages.sort([](const float & a, const float & b) { return a < b; });
	
		std::stringstream ss;
		ss << name << " (" << percentages.size() << "): ";
	
		int n = 0;
		for (std::list<float>::const_iterator it2 = percentages.begin(), end2 = percentages.end(); it2 != end2; ++it2)
		{
			if (n > 0) ss << ", ";
			ss << std::round(*it2 * 100.0);
	
			n++;
		}
	
		result.push_back(ss.str());
		ss.str(std::string());
	
		//avoid divide-by-zero exception
		if (totalHealth > 0.0)
		{
			ss << "Remaining: " << std::round((totalRemaining / totalHealth) * 100.0) << "%, Diff: " << std::round(-totalDiff);
		}
	
		result.push_back(ss.str());
	}
	
	return result;
}