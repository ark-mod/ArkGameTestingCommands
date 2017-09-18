#define _USE_MATH_DEFINES

#include <windows.h>
#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <list>
#include "API/Base.h"
#include "API/UE.h"
#include "Tools.h"
#include "ArkGameTestingCommands.h"
#include "SpawnCustomDino.h"
#include "GiveCustomItems.h"
#include "GiveItemsTemplate.h"
#include "json.hpp"

#pragma comment(lib, "ArkApi.lib")

typedef void(*callback_function)(void);

void ReloadTestConfig(APlayerController* playerController, FString* cmd, bool shouldLog);

nlohmann::json json;
nlohmann::json configDumpJson;

UPrimalGameData* uPrimalGameData;
UShooterGameInstance* uShooterGameInstance;

DECLARE_HOOK(UPrimalGameData_Initialize, void, UPrimalGameData*, int, UClass **, callback_function);
//DECLARE_HOOK(APrimalWorldSettings_GetNPCRandomSpawnClass, UClass*, APrimalWorldSettings*, TArray<FClassNameReplacement>*, TArray<FClassRemappingWeight>*, UClass*, float, bool);
//DECLARE_HOOK(AShooterGameMode_CheckIsOfficialServer, void, AShooterGameMode *);
DECLARE_HOOK(FConfigCacheIni_GetString, char, FConfigCacheIni*, const wchar_t*, const wchar_t*, FString*, FString*);

DECLARE_HOOK(FConfigCacheIni_GetArray, __int64, FConfigCacheIni*, const wchar_t*, const wchar_t*, TArray<FString> *, FString*);
//__int64 __fastcall FConfigCacheIni::GetArray(FConfigCacheIni *this, const wchar_t *Section, const wchar_t *Key, TArray<FString,FDefaultAllocator> *out_Arr, FString *Filename)

DECLARE_HOOK(UShooterGameInstance_Init, void, UShooterGameInstance*);
//void __fastcall UShooterGameInstance::Init(UShooterGameInstance *this)

void CustomGiveItem(APlayerController* aPlayerController, FString* cmd, bool bWriteToLog)
{
	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(aPlayerController);

	TArray<FString> Parsed;
	cmd->ParseIntoArray(&Parsed, L" ", true);

	if (Parsed.IsValidIndex(6))
	{
		unsigned __int64 steamId;
		float quality, rating;
		int qualityIndex, armor;

		try
		{
			steamId = std::stoull(*Parsed[1]);
			quality = std::stof(*Parsed[3]);
			rating = std::stof(*Parsed[4]);
			qualityIndex = std::stoi(*Parsed[5]);
			armor = std::stoi(*Parsed[6]);
		}
		catch (const std::exception&)
		{
			return;
		}

		FString bpPath = Parsed[2];

		AShooterPlayerController* aShooterPC = FindPlayerControllerFromSteamId(steamId);
		if (aShooterPC)
		{
			std::string bpPathStr = bpPath.ToString();
			if (!bpPathStr.empty())
			{
				wchar_t* bpPathWStr = ConvertToWideStr(bpPath.ToString());

				UObject* object = Globals::StaticLoadObject(UObject::StaticClass(), nullptr, bpPathWStr, nullptr, 0, 0, true);
				if (object)
				{
					TSubclassOf<UPrimalItem> archetype;
					archetype.uClass = reinterpret_cast<UClass*>(object);

					AShooterCharacter* aShooterCharacter = static_cast<AShooterCharacter*>(aShooterPC->GetCharacterField());

					UPrimalItem* saddle = UPrimalItem::AddNewItem(archetype, aShooterCharacter->GetMyInventoryComponentField(), false, false, quality >= 0.0 ? quality : 0.0, false, 0, false, 0.0, false, TSubclassOf<UPrimalItem>());
					
					//0: Effectiveness
					//1: Armor
					//2: Max Durability
					//3: Weapon Damage
					//4: Weapon Clip Ammo
					//5: Hypothermic Insulation
					//6: Weight
					//7: Hyperthermic Insulation

					//armor: 0 = 25.0 armor
					//armor: 1000 = 30.0 armor
					//armor: 15000 = 100.0 armor
					//armor: 35000 = 200.0 armor
					//I.E. +1000 = +5.0 armor

					if (armor >= 0)
					{
						unsigned short* statValues = saddle->GetItemStatValuesField();
						if (statValues == nullptr) SendDirectMessage(aShooterController, L"ItemStatValues is null");
						else statValues[1] = armor;
					}
					
					if (rating >= 0.0) saddle->SetItemRatingField(rating);
					if (qualityIndex > 5) saddle->SetItemQualityIndexField(5);
					else if (qualityIndex >= 0) saddle->SetItemQualityIndexField(qualityIndex);

					saddle->UpdatedItem();

					// Send a reply
					SendDirectMessage(aShooterController, L"Successfully spawned item");
				}

				delete[] bpPathWStr;
			}
		}
	}
}

void SpawnTemplate(APlayerController* aPlayerController, FString* cmd, bool bWriteToLog)
{
	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(aPlayerController);

	TArray<FString> Parsed;
	cmd->ParseIntoArray(&Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		std::string templateName = Parsed[1].ToString();

		auto templatesList = json["Templates"];

		auto templateEntryIter = templatesList.find(templateName);
		if (templateEntryIter == templatesList.end())
		{
			SendDirectMessage(aShooterController, L"The template does not exist! Maybe you forgot to reload..?");
			return;
		}

		auto templateEntry = templateEntryIter.value();

		__int64 steamId = GetSteamId(aShooterController);

		int dinoBaseLevelHealth, dinoBaseLevelStamina, dinoBaseLevelOxygen, dinoBaseLevelFood, dinoBaseLevelWeight, dinoBaseLevelMeleeDamage, dinoBaseLevelMovementSpeed;
		int dinoTamedLevelHealth, dinoTamedLevelStamina, dinoTamedLevelOxygen, dinoTamedLevelFood, dinoTamedLevelWeight, dinoTamedLevelMeleeDamage, dinoTamedLevelMovementSpeed;
		float saddleArmor;
		int count;
		float radius;
		std::list<GiveItemDefinition> items;

		std::string bpPath = templateEntry["blueprint"];
		std::string bpPathSaddle = templateEntry["saddleBlueprint"];

		try
		{
			saddleArmor = templateEntry["saddleArmor"];
			dinoBaseLevelHealth = templateEntry["baseLevelHealth"];
			dinoBaseLevelStamina = templateEntry["baseLevelStamina"];
			dinoBaseLevelOxygen = templateEntry["baseLevelOxygen"];
			dinoBaseLevelFood = templateEntry["baseLevelFood"];
			dinoBaseLevelWeight = templateEntry["baseLevelWeight"];
			dinoBaseLevelMeleeDamage = templateEntry["baseLevelMeleeDamage"];
			dinoBaseLevelMovementSpeed = templateEntry["baseLevelMovementSpeed"];

			dinoTamedLevelHealth = templateEntry["tamedLevelHealth"];
			dinoTamedLevelStamina = templateEntry["tamedLevelStamina"];
			dinoTamedLevelOxygen = templateEntry["tamedLevelOxygen"];
			dinoTamedLevelFood = templateEntry["tamedLevelFood"];
			dinoTamedLevelWeight = templateEntry["tamedLevelWeight"];
			dinoTamedLevelMeleeDamage = templateEntry["tamedLevelMeleeDamage"];
			dinoTamedLevelMovementSpeed = templateEntry["tamedLevelMovementSpeed"];

			count = templateEntry["count"];
			radius = templateEntry["radius"];

			auto itemsMap = templateEntry.value("items", nlohmann::json::array());
			for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter)
			{
				auto item = iter.value();

				std::string blueprint = item["blueprint"];
				int quantity = item["quantity"];
				int count = item["count"];

				GiveItemDefinition giveItemDef;
				giveItemDef.blueprint = blueprint;
				giveItemDef.quantity = quantity;
				giveItemDef.count = count;

				items.push_back(giveItemDef);
			}
		}
		catch (const std::exception&)
		{
			SendDirectMessage(aShooterController, L"Failed to read template...");
			return;
		}

		bool success = true;
		for (int i = 0; i < count; i++)
		{
			float a = i * 2 * M_PI / count;
			float x = radius * std::cos(a);
			float y = radius * std::sin(a);

			bool innerSuccess = SpawnCustomDino(
				steamId,
				bpPath, bpPathSaddle,
				dinoBaseLevelHealth, dinoBaseLevelStamina, dinoBaseLevelOxygen, dinoBaseLevelFood, dinoBaseLevelWeight, dinoBaseLevelMeleeDamage, dinoBaseLevelMovementSpeed,
				dinoTamedLevelHealth, dinoTamedLevelStamina, dinoTamedLevelOxygen, dinoTamedLevelFood, dinoTamedLevelWeight, dinoTamedLevelMeleeDamage, dinoTamedLevelMovementSpeed,
				saddleArmor, items, x, y);

			if (!innerSuccess)
			{
				success = false;
				break;
			}
		}
	}
}

void ItemTemplate(APlayerController* aPlayerController, FString* cmd, bool bWriteToLog)
{
	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(aPlayerController);

	TArray<FString> Parsed;
	cmd->ParseIntoArray(&Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		std::string templateName = Parsed[1].ToString();

		auto templatesList = json["ItemTemplates"];

		auto templateEntryIter = templatesList.find(templateName);
		if (templateEntryIter == templatesList.end())
		{
			SendDirectMessage(aShooterController, L"The template does not exist! Maybe you forgot to reload..?");
			return;
		}

		auto templateEntry = templateEntryIter.value();

		__int64 steamId = GetSteamId(aShooterController);

		int count;
		std::list<GiveItemDefinition> items;

		try
		{
			count = templateEntry.value("count", 1);

			auto itemsMap = templateEntry.value("items", nlohmann::json::array());
			for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter)
			{
				auto item = iter.value();

				std::string blueprint = item.value("blueprint", "");
				std::list<std::string> blueprints = item.find("blueprints") != item.end() ? item["blueprints"] : std::list<std::string>();
				int quantity = item.value("quantity", 0);
				int count = item.value("count", 1);
				float quality = item.value("quality", 0.0);

				GiveItemDefinition giveItemDef;
				giveItemDef.blueprint = blueprint;
				giveItemDef.blueprints = blueprints;
				giveItemDef.quantity = quantity;
				giveItemDef.count = count;
				giveItemDef.quality = quality;

				items.push_back(giveItemDef);
			}
		}
		catch (const std::exception&)
		{
			SendDirectMessage(aShooterController, L"Failed to read template...");
			return;
		}

		bool success = true;
		for (int i = 0; i < count; i++)
		{
			bool innerSuccess = GiveCustomItems(steamId, items);

			if (!innerSuccess)
			{
				success = false;
				break;
			}
		}
	}
}

void TeleportTo(APlayerController* aPlayerController, FString* cmd, bool bWriteToLog)
{
	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(aPlayerController);

	TArray<FString> Parsed;
	cmd->ParseIntoArray(&Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		std::string templateName = Parsed[1].ToString();

		auto templatesList = json["Locations"];

		auto templateEntryIter = templatesList.find(templateName);
		if (templateEntryIter == templatesList.end())
		{
			SendDirectMessage(aShooterController, L"The template does not exist! Maybe you forgot to reload..?");
			return;
		}

		auto templateEntry = templateEntryIter.value();

		__int64 steamId = GetSteamId(aShooterController);

		float x, y, z;

		try
		{
			x = templateEntry.value("x", 0.0);
			y = templateEntry.value("y", 0.0);
			z = templateEntry.value("z", 0.0);
		}
		catch (const std::exception&)
		{
			SendDirectMessage(aShooterController, L"Failed to read template...");
			return;
		}

		aShooterController->SetPlayerPos(x, y, z);
	}
}

void LoadConfig()
{
	try {
		std::ifstream file(GetCurrentDir() + "/BeyondApi/Plugins/ArkGameTestingCommands/config.json");
		if (!file.is_open())
		{
			std::cout << "Could not open file config.json" << std::endl;
			throw;
		}

		file >> json;
		file.close();
	}
	catch (const std::exception& ex) {
		std::cout << "Exception: " << ex.what() << std::endl;
		return;
	}
}

void ReloadTestConfig(APlayerController* playerController, FString* cmd, bool shouldLog)
{
	LoadConfig();

	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(playerController);
}

void TestFunc(APlayerController* playerController, FString* cmd, bool shouldLog)
{
	AShooterPlayerController* aShooterController = static_cast<AShooterPlayerController*>(playerController);

	APlayerState* state = aShooterController->GetPlayerStateField();

	DWORD64 ptr = *reinterpret_cast<DWORD64 *>(*reinterpret_cast<DWORD64 *>(state) + static_cast<DWORD64>(2760));
	DWORD64 base = Ark::GetBaseAddress();

	std::cout << "[Test] Base address: " << std::hex << base << std::endl;
	std::cout << "[Test] Func: " << std::hex << ptr << std::endl;
	std::cout << "[Test] Calc: " << std::hex << (ptr - base) << " (" << std::dec << (ptr - base) << ")" << std::endl;
}

void LevelExperienceRamps(RCONClientConnection* rconClientConnection, RCONPacket* rconPacket, UWorld* uWorld)
{
	if (uPrimalGameData)
	{
		FLevelExperienceRamp* levelExperienceRamps = uPrimalGameData->GetLevelExperienceRampsField();
		if (levelExperienceRamps)
		{
			std::wstringstream ss;

			for (int n = 0; n < 4; n++)
			{
				FLevelExperienceRamp levelExperienceRamp = levelExperienceRamps[n];
				ss << "Ramp[" << n << "]: [";
				for (uint32_t i = 0; i < levelExperienceRamp.ExperiencePointsForLevel.Num(); i++)
				{
					if (i > 0) ss << ", ";
					ss << std::fixed << std::setprecision(0) << levelExperienceRamp.ExperiencePointsForLevel[i];
				}

				
				ss << "]\n";
			}

			FString reply(ss.str());
			rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);
		}
		else
		{
			FString reply = L"Failed to get level experience ramps\n";
			rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);
		}
	}
	else
	{
		FString reply = L"Failed to get primal game data\n";
		rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);
	}
}

void TestRandomBaseLevel(RCONClientConnection* rconClientConnection, RCONPacket* rconPacket, UWorld* uWorld)
{
	FString msg = rconPacket->Body;

	TArray<FString> Parsed;
	msg.ParseIntoArray(&Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		APlayerController* aPC = Ark::GetWorld()->GetFirstPlayerController();

		AShooterPlayerController* aShooterPC = static_cast<AShooterPlayerController*>(aPC);
		if (aShooterPC)
		{
			FString bpPath = Parsed[1];

			AActor* actor = aShooterPC->SpawnActor(&bpPath, 50, 0, 0, true);
			if (actor && actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass()))
			{
				APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);

				int baseLevels[30] = {};
				for (int i = 0; i < 10000000; i++) {
					int baseLevel = dino->GetRandomBaseLevel();
					baseLevels[baseLevel - 1] = baseLevels[baseLevel - 1] + 1;
				}

				dino->Suicide();

				// Send a reply
				std::stringstream ss;
				ss << bpPath.ToString() << "\n" << "[ ";
				for (int i = 0; i < 30; i++) {
					ss << baseLevels[i] << (i < 30 - 1 ? "," : "");
				}
				ss << " ]\n";
				wchar_t* wcstring = ConvertToWideStr(ss.str());

				FString reply(wcstring);
				rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);

				delete[] wcstring;
			}
		}
	}
}

void SpawnZoneTest(RCONClientConnection* rconClientConnection, RCONPacket* rconPacket, UWorld* uWorld) {
	TArray<AActor*>* FoundActors = new TArray<AActor*>();

	//ANPCZoneManager

	UWorld* world = Ark::GetWorld();
	if (!world) return;

	UGameplayStatics::GetAllActorsOfClass(world, ANPCZoneManager::GetPrivateStaticClass(), FoundActors);

	std::stringstream ss;
	nlohmann::json ojson;

	int num = 0;
	for (uint32_t i = 0; i < FoundActors->Num(); i++)
	{
		AActor* actor = (*FoundActors)[i];

		ANPCZoneManager* zoneManager = static_cast<ANPCZoneManager*>(actor);

		TSubclassOf<UNPCSpawnEntriesContainer> spawnEntitiesContainer = zoneManager->GetNPCSpawnEntriesContainerObjectField();

		std::string key;

		if (spawnEntitiesContainer.uClass) {
			FString* className = new FString();
			FName bp = spawnEntitiesContainer.uClass->GetNameField();
			bp.ToString(className);

			//ss << "[ " << className->ToString() << " ]\n";
			key = className->ToString();
			delete className;

			auto iter = ojson.find(key);
			if (iter == ojson.end()) {
				ojson[key] = nlohmann::json::object();
			}
			else continue;
		}
		else
		{
			ss << "[ UNKNOWN ]\n";
			continue;
		}

		TArray<FNPCSpawnEntry> spawnEntries = zoneManager->GetNPCSpawnEntriesField();
		for (uint32_t j = 0; j < spawnEntries.Num(); j++)
		{
			FNPCSpawnEntry se = spawnEntries[j];
			FString name = se.AnEntryName;
			std::string nameStr = name.ToString();
			//ss << "Name: " << name.ToString() << "\n";

			std::list<std::string> npcsToSpawn;
			for (uint32_t k = 0; k < se.NPCsToSpawn.Num(); k++)
			{
				FString* className = new FString();
				FName bp = se.NPCsToSpawn[k].uClass->GetNameField();
				bp.ToString(className);
				npcsToSpawn.push_back(className->ToString());
				delete className;
			}

			std::list<std::string> npcsToSpawnStrings;
			for (uint32_t k = 0; k < se.NPCsToSpawnStrings.Num(); k++)
			{
				npcsToSpawnStrings.push_back(se.NPCsToSpawnStrings[k].ToString());
			}

			std::list<nlohmann::json> npcRandomSpawnClassWeights;
			for (uint32_t k = 0; k < se.NPCRandomSpawnClassWeights.Num(); k++)
			{
				std::string fromClass;
				{
					FString* className = new FString();
					FName bp = se.NPCRandomSpawnClassWeights[k].FromClass.uClass->GetNameField();
					bp.ToString(className);
					fromClass = className->ToString();
					delete className;
				}

				std::list<std::string> toClasses;
				for (uint32_t l = 0; l < se.NPCRandomSpawnClassWeights[k].ToClasses.Num(); l++)
				{
					FString* className = new FString();
					FName bp = se.NPCRandomSpawnClassWeights[k].ToClasses[l].uClass->GetNameField();
					bp.ToString(className);
					toClasses.push_back(className->ToString());
					delete className;
					
				}

				std::list<float> weights;
				for (uint32_t l = 0; l < se.NPCRandomSpawnClassWeights[k].Weights.Num(); l++)
				{
					weights.push_back(se.NPCRandomSpawnClassWeights[k].Weights[l]);
				}

				npcRandomSpawnClassWeights.push_back(nlohmann::json({ 
					{ "FromClass", fromClass },
					{ "ToClasses", toClasses },
					{ "Weights", weights } }));
			}

			std::list<nlohmann::json> npcsSpawnOffsets;
			for (uint32_t k = 0; k < se.NPCsSpawnOffsets.Num(); k++)
			{
				npcsSpawnOffsets.push_back(nlohmann::json({
					{ "X", se.NPCsSpawnOffsets[k].X },
					{ "Y", se.NPCsSpawnOffsets[k].Y },
					{ "Z", se.NPCsSpawnOffsets[k].Z } }));
			}

			std::list<float> npcsToSpawnPercentageChance;
			for (uint32_t k = 0; k < se.NPCsToSpawnPercentageChance.Num(); k++)
			{
				npcsToSpawnPercentageChance.push_back(se.NPCsToSpawnPercentageChance[k]);
			}

			std::list<float> npcMinLevelOffset;
			for (uint32_t k = 0; k < se.NPCMinLevelOffset.Num(); k++)
			{
				npcMinLevelOffset.push_back(se.NPCMinLevelOffset[k]);
			}

			std::list<float> npcMaxLevelOffset;
			for (uint32_t k = 0; k < se.NPCMaxLevelOffset.Num(); k++)
			{
				npcMaxLevelOffset.push_back(se.NPCMaxLevelOffset[k]);
			}

			std::list<float> npcMinLevelMultiplier;
			for (uint32_t k = 0; k < se.NPCMinLevelMultiplier.Num(); k++)
			{
				npcMinLevelMultiplier.push_back(se.NPCMinLevelMultiplier[k]);
			}

			std::list<float> npcMaxLevelMultiplier;
			for (uint32_t k = 0; k < se.NPCMaxLevelMultiplier.Num(); k++)
			{
				npcMaxLevelMultiplier.push_back(se.NPCMaxLevelMultiplier[k]);
			}

			std::list<int> npcOverrideLevel;
			for (uint32_t k = 0; k < se.NPCOverrideLevel.Num(); k++)
			{
				npcOverrideLevel.push_back((int)se.NPCOverrideLevel[k]);
			}

			std::list<nlohmann::json> npcDifficultyLevelRanges;
			for (uint32_t k = 0; k < se.NPCDifficultyLevelRanges.Num(); k++)
			{
				std::list<float> enemyLevelsMax;
				for (uint32_t l = 0; l < se.NPCDifficultyLevelRanges[k].EnemyLevelsMax.Num(); l++)
				{
					enemyLevelsMax.push_back(se.NPCDifficultyLevelRanges[k].EnemyLevelsMax[l]);
				}

				std::list<float> enemyLevelsMin;
				for (uint32_t l = 0; l < se.NPCDifficultyLevelRanges[k].EnemyLevelsMin.Num(); l++)
				{
					enemyLevelsMin.push_back(se.NPCDifficultyLevelRanges[k].EnemyLevelsMin[l]);
				}

				std::list<float> gameDifficulties;
				for (uint32_t l = 0; l < se.NPCDifficultyLevelRanges[k].GameDifficulties.Num(); l++)
				{
					gameDifficulties.push_back(se.NPCDifficultyLevelRanges[k].GameDifficulties[l]);
				}

				npcDifficultyLevelRanges.push_back(nlohmann::json({
					{ "EnemyLevelsMax", enemyLevelsMax },
					{ "EnemyLevelsMin", enemyLevelsMin },
					{ "GameDifficulties", gameDifficulties } }));
			}
			
			ojson[key]["spawnEntries"].push_back(nlohmann::json({
				{ "AnEntryName", nameStr },
				{ "NPCsToSpawn", npcsToSpawn },
				{ "NPCsToSpawnStrings", npcsToSpawnStrings },
				{ "NPCRandomSpawnClassWeights", npcRandomSpawnClassWeights },
				{ "NPCsSpawnOffsets", npcsSpawnOffsets },
				{ "NPCsToSpawnPercentageChance", npcsToSpawnPercentageChance },
				{ "NPCMinLevelOffset", npcMinLevelOffset },
				{ "NPCMaxLevelOffset", npcMaxLevelOffset },
				{ "NPCMinLevelMultiplier", npcMinLevelMultiplier },
				{ "NPCMaxLevelMultiplier", npcMaxLevelMultiplier },
				{ "bAddLevelOffsetBeforeMultiplier", (int)se.bAddLevelOffsetBeforeMultiplier },
				{ "NPCOverrideLevel", npcOverrideLevel },
				{ "ExtentCheck", {
					{ "X", se.ExtentCheck.X },
					{ "Y", se.ExtentCheck.Y },
					{ "Z", se.ExtentCheck.Z } } },
				{ "GroupSpawnOffset",{
					{ "X", se.GroupSpawnOffset.X },
					{ "Y", se.GroupSpawnOffset.Y },
					{ "Z", se.GroupSpawnOffset.Z } } },
				{ "EntryWeight", se.EntryWeight },
				{ "ManualSpawnPointSpreadRadius", se.ManualSpawnPointSpreadRadius },
				{ "WaterOnlySpawnMinimumWaterHeight", se.WaterOnlySpawnMinimumWaterHeight },
				{ "MaximumWaterHeight", se.MaximumWaterHeight },
				{ "NPCDifficultyLevelRanges", npcDifficultyLevelRanges },
				{ "LevelDifficultyTestOverride", se.LevelDifficultyTestOverride },
				{ "SpawnMinDistanceFromStructuresMultiplier", se.SpawnMinDistanceFromStructuresMultiplier },
				{ "SpawnMinDistanceFromPlayersMultiplier", se.SpawnMinDistanceFromPlayersMultiplier },
				{ "SpawnMinDistanceFromTamedDinosMultiplier", se.SpawnMinDistanceFromTamedDinosMultiplier },
				{ "RandGroupSpawnOffsetZMin", se.RandGroupSpawnOffsetZMin },
				{ "RandGroupSpawnOffsetZMax", se.RandGroupSpawnOffsetZMax } }));

			/*FString AnEntryName;
			TArray<TSubclassOf<APrimalDinoCharacter>> NPCsToSpawn;
			TArray<FString> NPCsToSpawnStrings;
			TArray<FClassRemappingWeight> NPCRandomSpawnClassWeights;
			TArray<FVector> NPCsSpawnOffsets;
			TArray<float> NPCsToSpawnPercentageChance;
			TArray<float> NPCMinLevelOffset;
			TArray<float> NPCMaxLevelOffset;
			TArray<float> NPCMinLevelMultiplier;
			TArray<float> NPCMaxLevelMultiplier;
			unsigned __int32 bAddLevelOffsetBeforeMultiplier : 1;
			TArray<unsigned char> NPCOverrideLevel;
			FVector ExtentCheck;
			FVector GroupSpawnOffset;
			float EntryWeight;
			float ManualSpawnPointSpreadRadius;
			float WaterOnlySpawnMinimumWaterHeight;
			float MaximumWaterHeight;
			TArray<FNPCDifficultyLevelRange> NPCDifficultyLevelRanges;
			float LevelDifficultyTestOverride;
			float SpawnMinDistanceFromStructuresMultiplier;
			float SpawnMinDistanceFromPlayersMultiplier;
			float SpawnMinDistanceFromTamedDinosMultiplier;
			float RandGroupSpawnOffsetZMin;
			float RandGroupSpawnOffsetZMax;*/
		}

		TArray<FNPCSpawnLimit> spawnLimits = zoneManager->GetNPCSpawnLimitsField();
		for (uint32_t j = 0; j < spawnLimits.Num(); j++)
		{
			FNPCSpawnLimit sl = spawnLimits[j];

			std::string npcClass;
			{
				FString* className = new FString();
				FName bp = sl.NPCClass.uClass->GetNameField();
				bp.ToString(className);
				npcClass = className->ToString();
				delete className;
			}

			ojson[key]["spawnLimits"].push_back(nlohmann::json({
				{ "NPCClass", npcClass },
				{ "NPCClassString", sl.NPCClassString.ToString() },
				{ "MaxPercentageOfDesiredNumToAllow", sl.MaxPercentageOfDesiredNumToAllow },
				{ "CurrentNumberOfNPCTouching", sl.CurrentNumberOfNPCTouching } }));

			/*TSubclassOf<APrimalDinoCharacter> NPCClass;
			FString NPCClassString;
			float MaxPercentageOfDesiredNumToAllow;
			int CurrentNumberOfNPCTouching;*/
		}

		ojson[key]["bIgnoreNPCRandomClassReplacements"] = zoneManager->GetbIgnoreNPCRandomClassReplacementsField();

		//FName name = zoneManager->GetNameField();

		num++;
	}

	std::ofstream o(GetCurrentDir() + "/BeyondApi/Plugins/ArkGameTestingCommands/SpawnZoneTest-output.json");
	o << std::setw(4) << ojson << std::endl;

	ss << "Found " << num << " spawn zones\n";

	wchar_t* wcstring = ConvertToWideStr(ss.str());

	FString reply(wcstring);
	rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);

	delete FoundActors;
	delete[] wcstring;
}

void DumpConfigJson(RCONClientConnection* rconClientConnection, RCONPacket* rconPacket, UWorld* uWorld) {
	std::ofstream o(GetCurrentDir() + "/BeyondApi/Plugins/ArkGameTestingCommands/ark-settings-output.json");
	o << std::setw(4) << configDumpJson << std::endl;

	std::stringstream ss;

	ss << "Dumped " << configDumpJson.size() << " ini settings\n";

	wchar_t* wcstring = ConvertToWideStr(ss.str());

	FString reply(wcstring);
	rconClientConnection->SendMessageW(rconPacket->Id, 0, &reply);

	delete[] wcstring;
}

void MyDinoHealth(AShooterPlayerController* aShooterPlayerController, FString* message, int mode)
{
	UWorld* world = Ark::GetWorld();
	if (!world) return;
	if (!aShooterPlayerController) return;

	ACharacter* character = aShooterPlayerController->GetCharacterField();
	if (!character || !character->IsA(APrimalCharacter::GetPrivateStaticClass())) return;

	APrimalCharacter* primalCharacter = static_cast<APrimalCharacter*>(character);

	std::map<std::string, std::list<std::tuple<float, float>>> statuses;
	FVector* pos = new FVector();
	int teamId = aShooterPlayerController->GetTargetingTeamField();
	FVector playerPos = aShooterPlayerController->GetDefaultActorLocationField();

	TArray<AActor*>* FoundActors = new TArray<AActor*>();
	UGameplayStatics::GetAllActorsOfClass(world, APrimalDinoCharacter::GetPrivateStaticClass(), FoundActors);

	for (uint32_t i = 0; i < FoundActors->Num(); i++)
	{
		AActor* actor = (*FoundActors)[i];

		APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);

		int dinoTeam = dino->GetTargetingTeamField();

		dino->GetRootComponentField()->GetCustomLocation(pos);
		if (dinoTeam == teamId && IsPointInside2dCircle(*pos, playerPos.X, playerPos.Y, 5000))
		{
			FString* className = new FString();
			dino->GetDinoNameTagField().ToString(className); //species name
															 //std::string name = dino->GetTamedNameField().ToString(); //tamed name

			UPrimalCharacterStatusComponent* status = dino->GetMyCharacterStatusComponentField();
			if (status)
			{
				float* currentStatValues = status->GetCurrentStatusValuesField();
				float* maxStatsValues = status->GetMaxStatusValuesField();

				std::string classNameStr = className->ToString();

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

			//todo: delete memory alloc?
			//delete className;
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

		wchar_t* wcstring = ConvertToWideStr(ss.str());
		SendChatMessage(aShooterPlayerController, L"[system]", wcstring);
		delete[] wcstring;
		ss.str(std::string());

		//avoid divide-by-zero exception
		if (totalHealth > 0.0)
		{
			ss << "Remaining: " << std::round((totalRemaining / totalHealth) * 100.0) << "%%, Diff: " << std::round(-totalDiff);
		}

		wcstring = ConvertToWideStr(ss.str());
		SendChatMessage(aShooterPlayerController, L"[system]", wcstring);
		delete[] wcstring;
	}
}

void ComeToMe(AShooterPlayerController* aShooterPlayerController, FString* message, int mode)
{
	UWorld* world = Ark::GetWorld();
	if (!world) return;
	if (!aShooterPlayerController) return;

	ACharacter* character = aShooterPlayerController->GetCharacterField();
	if (!character || !character->IsA(APrimalCharacter::GetPrivateStaticClass())) return;

	APrimalCharacter* primalCharacter = static_cast<APrimalCharacter*>(character);
	AActor* actor = primalCharacter->GetAimedActor(ECC_GameTraceChannel2, 0i64, 10000.0f, 10.0, 0i64, 0i64, 0, 0);
	// APrimalCharacter::GetAimedActor(this, ECC_GameTraceChannel16, 0i64, v1 * 2.0, 240.0, 0i64, 0i64, 1, 1);
	if (!actor || !actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass())) return;

	APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);
	int dinoTeam = dino->GetTargetingTeamField();
	int playerTeam = aShooterPlayerController->GetTargetingTeamField();
	if (dinoTeam != playerTeam) return;

	std::stringstream ss;

	UPrimalCharacterStatusComponent* status = dino->GetCharacterStatusComponent();
	//dino->UseHighQualityMovement
	//dino->ServerCallMoveTo_Implementation
	//dino->TryCallMoveTo

	FVector pos = aShooterPlayerController->GetDefaultActorLocationField();

	AController* dinoController = dino->GetCharacterController();
	APrimalDinoAIController* dinoAiController = static_cast<APrimalDinoAIController*>(dinoController);

	UNavigationQueryFilter navFilter = UNavigationQueryFilter();
	TSubclassOf<UNavigationQueryFilter> archetype = TSubclassOf<UNavigationQueryFilter>();
	archetype.uClass = reinterpret_cast<UClass*>(&navFilter);

	//dinoAiController->MoveToLocation(&pos, -1.0f, true, true, true, true, archetype, true); //does not work
	dinoAiController->MoveToLocation(&pos, -1.0f, true, true, false, true, archetype, true);
	dino->UpdateNavAgent();
	dinoAiController->Tick(0.0f);
}

void ForceLand(AShooterPlayerController* aShooterPlayerController, FString* message, int mode)
{
	UWorld* world = Ark::GetWorld();
	if (!world) return;
	if (!aShooterPlayerController) return;

	ACharacter* character = aShooterPlayerController->GetCharacterField();
	if (!character || !character->IsA(APrimalCharacter::GetPrivateStaticClass())) return;

	APrimalCharacter* primalCharacter = static_cast<APrimalCharacter*>(character);
	AActor* actor = primalCharacter->GetAimedActor(ECC_GameTraceChannel16, 0i64, 0.0, 0.0, 0i64, 0i64, 0, 0);
	// APrimalCharacter::GetAimedActor(this, ECC_GameTraceChannel16, 0i64, v1 * 2.0, 240.0, 0i64, 0i64, 1, 1);
	if (!actor || !actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass())) return;

	APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);
	int dinoTeam = dino->GetTargetingTeamField();
	int playerTeam = aShooterPlayerController->GetTargetingTeamField();
	if (dinoTeam != playerTeam) return;

	std::stringstream ss;

	UPrimalCharacterStatusComponent* status = dino->GetCharacterStatusComponent();
	//dino->UseHighQualityMovement
	//dino->ServerCallMoveTo_Implementation
	//dino->TryCallMoveTo

	FVector pos = aShooterPlayerController->GetDefaultActorLocationField();

	AController* dinoController = dino->GetCharacterController();
	APrimalDinoAIController* dinoAiController = static_cast<APrimalDinoAIController*>(dinoController);

	if (dinoAiController->CanLand())
	{
		dinoAiController->ForceLand(0, 0);
	}
}

void SpeedyTest(AShooterPlayerController* aShooterPlayerController, FString* message, int mode)
{
	UWorld* world = Ark::GetWorld();
	if (!world) return;
	if (!aShooterPlayerController) return;

	ACharacter* character = aShooterPlayerController->GetCharacterField();
	if (!character || !character->IsA(APrimalCharacter::GetPrivateStaticClass())) return;

	APrimalCharacter* primalCharacter = static_cast<APrimalCharacter*>(character);
	AActor* actor = primalCharacter->GetAimedActor(ECC_GameTraceChannel16, 0i64, 0.0, 0.0, 0i64, 0i64, 0, 0);
	// APrimalCharacter::GetAimedActor(this, ECC_GameTraceChannel16, 0i64, v1 * 2.0, 240.0, 0i64, 0i64, 1, 1);
	if (!actor || !actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass())) return;

	APrimalDinoCharacter* dino = static_cast<APrimalDinoCharacter*>(actor);
	int dinoTeam = dino->GetTargetingTeamField();
	int playerTeam = aShooterPlayerController->GetTargetingTeamField();
	if (dinoTeam != playerTeam) return;

	std::stringstream ss;

	//dino->UseHighQualityMovement
	//dino->ServerCallMoveTo_Implementation
	//dino->TryCallMoveTo

	FVector pos = aShooterPlayerController->GetDefaultActorLocationField();

	AController* dinoController = dino->GetCharacterController();
	APrimalDinoAIController* dinoAiController = static_cast<APrimalDinoAIController*>(dinoController);

	UPrimalCharacterStatusComponent* status = dino->GetMyCharacterStatusComponentField();
	if (status)
	{
		// 0: health
		// 1: stamina
		// 2: torpor
		// 3: oxygen
		// 4: food
		// 5: water
		// 6: temperature
		// 7: weight
		// 8: melee damage
		// 9: movement speed
		// 10: fortitude
		// 11: crafting speed

		char* statsTamed = status->GetNumberOfLevelUpPointsAppliedTamedField();
		statsTamed[9] = 100;
	}

	dino->SetRunningSpeedModifierField(10.0f);

	dino->UpdateStatusComponent(0.0);
	dino->ForceNetUpdate(false);
}

////void __fastcall UPrimalGameData::Initialize(UPrimalGameData *this, int a2, UClass **a3, void (__cdecl *a4)())
void _cdecl Hook_UPrimalGameData_Initialize(UPrimalGameData* _UPrimalGameData, int a2, UClass ** a3, callback_function a4)
{
	std::cout << "[API] UPrimalGameData::Initialize was called" << std::endl;

	uPrimalGameData = _UPrimalGameData;
	UPrimalGameData_Initialize_original(_UPrimalGameData, a2, a3, a4);
}

void _cdecl Hook_UShooterGameInstance_Init(UShooterGameInstance* _UShooterGameInstance)
{
	std::cout << "[API] UShooterGameInstance::Init was called" << std::endl;
	
	DWORD64 ptr = *reinterpret_cast<DWORD64 *>(*reinterpret_cast<DWORD64 *>(_UShooterGameInstance) + static_cast<DWORD64>(0x28));
	DWORD64 base = Ark::GetBaseAddress();

	std::cout << "Base address: " << std::hex << base << std::endl;
	std::cout << "Func: " << std::hex << ptr << std::endl;
	std::cout << "Calc: " << std::hex << (ptr-base) << std::endl;

	uShooterGameInstance = _UShooterGameInstance;
	UShooterGameInstance_Init_original(_UShooterGameInstance);
}
//void __fastcall UShooterGameInstance::Init(UShooterGameInstance *this)

//some interesting functions
//Ark::GetGameMode()->RemoveTribe() //probably removes the entire tribe, or atleast tribe-profile and tribe name, while kicking all players out from said tribe
//UShooterCheatManager* cheatManager = rconClientConnection->GetCheatManagerField(); //get the cheat manager for rcon connections

void Init()
{
	configDumpJson = nlohmann::json::object();
	LoadConfig();

	Ark::AddChatCommand(L"/health", &MyDinoHealth);

	//Ark::AddChatCommand(L"/cometome", &ComeToMe);
	//Ark::AddChatCommand(L"/forceland", &ForceLand);
	//Ark::AddChatCommand(L"/speedy", &SpeedyTest);
	//Ark::AddConsoleCommand(L"SpawnCustomTamed", &SpawnCustomTamed);
	//Ark::AddConsoleCommand(L"MultipleSpawnCustomTamed", &MultipleSpawnCustomTamed);
	//Ark::AddConsoleCommand(L"CustomGiveItem", &CustomGiveItem);
	//Ark::AddConsoleCommand(L"TestFunc", &TestFunc);
	//Ark::AddRconCommand(L"TestRandomBaseLevel", &TestRandomBaseLevel);
	
	Ark::AddConsoleCommand(L"SpawnTemplate", &SpawnTemplate);
	Ark::AddConsoleCommand(L"ItemTemplate", &ItemTemplate);
	Ark::AddConsoleCommand(L"TeleportTo", &TeleportTo);
	Ark::AddConsoleCommand(L"ReloadTestConfig", &ReloadTestConfig);

	//Ark::AddRconCommand(L"DumpConfigJson", &DumpConfigJson);
	//Ark::AddRconCommand(L"SpawnZoneTest", &SpawnZoneTest);
	//Ark::AddRconCommand(L"LevelExperienceRamps", &LevelExperienceRamps);

	//Needed for: LevelExperienceRamps
	//Ark::SetHook("UPrimalGameData", "Initialize", &Hook_UPrimalGameData_Initialize, reinterpret_cast<LPVOID*>(&UPrimalGameData_Initialize_original));

	//Ark::SetHook("UShooterGameInstance", "Init", &Hook_UShooterGameInstance_Init, reinterpret_cast<LPVOID*>(&UShooterGameInstance_Init_original));

	//Ark::SetHook("FConfigCacheIni", "GetString", &Hook_FConfigCacheIni_GetString, reinterpret_cast<LPVOID*>(&FConfigCacheIni_GetString_original));
	//Ark::SetHook("FConfigCacheIni", "GetArray", &Hook_FConfigCacheIni_GetArray, reinterpret_cast<LPVOID*>(&FConfigCacheIni_GetArray_original));

	//Ark::SetHook("APrimalWorldSettings", "GetNPCRandomSpawnClass", &Hook_APrimalWorldSettings_GetNPCRandomSpawnClass, reinterpret_cast<LPVOID*>(&APrimalWorldSettings_GetNPCRandomSpawnClass_original));
	//Ark::SetHook("AShooterGameMode", "CheckIsOfficialServer", &Hook_AShooterGameMode_CheckIsOfficialServer, reinterpret_cast<LPVOID*>(&AShooterGameMode_CheckIsOfficialServer_original));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

////void __fastcall UPrimalGameData::Initialize(UPrimalGameData *this, int a2, UClass **a3, void (__cdecl *a4)())
//UClass* _cdecl Hook_APrimalWorldSettings_GetNPCRandomSpawnClass(APrimalWorldSettings *_APrimalWorldSettings, TArray<FClassNameReplacement> *ClassNameReplacements, TArray<FClassRemappingWeight> *TheNPCRandomSpawnClassWeights, UClass *forClass, float a5, bool bIgnoreNPCRandomClassReplacements)
//{
//	UClass* result = APrimalWorldSettings_GetNPCRandomSpawnClass_original(_APrimalWorldSettings, ClassNameReplacements, TheNPCRandomSpawnClassWeights, forClass, a5, bIgnoreNPCRandomClassReplacements);
//
//	std::stringstream ss;
//	UClass* settings = _APrimalWorldSettings->GetClassField();
//	if (settings != nullptr) {
//		FString* configName = new FString();
//		FString* configName2 = new FString();
//		settings->GetConfigName(configName);
//		unsigned int flags = settings->GetClassFlagsField();
//
//		FName name = settings->GetClassConfigNameField();
//		name.ToString(configName2);
//		ss << "APrimalWorldSettings: " << configName->ToString() << ", " << configName2->ToString() << ", " << flags << std::endl;
//
//		/*TArray<FClassRemappingWeight, FDefaultAllocator> NPCRandomSpawnClassWeights;
//		TArray<FClassRemappingWeight, FDefaultAllocator> SinglePlayerNPCRandomSpawnClassWeights;*/
//		TArray<FClassRemappingWeight> npcRandomSpawnClassWeights = _APrimalWorldSettings->GetNPCRandomSpawnClassWeightsField();
//		for (uint32_t i = 0; i < npcRandomSpawnClassWeights.Num(); i++)
//		{
//			for (uint32_t j = 0; j < npcRandomSpawnClassWeights.Num(); j++)
//			{
//				npcRandomSpawnClassWeights[i].Weights[j] = 1.0;
//			}
//		}
//
//		settings->SaveConfig(0, nullptr, nullptr);
//
//		delete configName;
//		delete configName2;
//	}
//	ss << "[API] APrimalWorldSettings::GetNPCRandomSpawnClass(" << GetUClassName(forClass) << ", ";
//	if (ClassNameReplacements == nullptr) {
//		ss << "[null]";
//	}
//	else
//	{
//		for (uint32_t i = 0; i < ClassNameReplacements->Num(); i++)
//		{
//			if (i != 0) ss << "|";
//			ss << (*ClassNameReplacements)[i].FromClassName.ToString() << " - " << (*ClassNameReplacements)[i].ToClassName.ToString();
//		}
//	}
//	ss << ", ";
//	if (TheNPCRandomSpawnClassWeights == nullptr) {
//		ss << "[null]";
//	}
//	else
//	{
//		for (uint32_t i = 0; i < TheNPCRandomSpawnClassWeights->Num(); i++)
//		{
//			if (i != 0) ss << "|";
//			ss << GetUClassName((*TheNPCRandomSpawnClassWeights)[i].FromClass.uClass) << " - ";
//			for (uint32_t j = 0; j < (*TheNPCRandomSpawnClassWeights)[i].ToClasses.Num(); j++)
//			{
//				if (j != 0) ss << "/";
//				ss << GetUClassName((*TheNPCRandomSpawnClassWeights)[i].ToClasses[j].uClass) << "=" << (*TheNPCRandomSpawnClassWeights)[i].Weights[j];
//			}
//		}
//	}
//	ss << ", " << a5 << ", " << bIgnoreNPCRandomClassReplacements;
//	ss << ") -> " << GetUClassName(result) << std::endl;
//
//	std::cout << ss.str();
//
//	return result;
//}

//__int64 __fastcall FConfigCacheIni::GetArray(FConfigCacheIni *this, const wchar_t *Section, const wchar_t *Key, TArray<FString,FDefaultAllocator> *out_Arr, FString *Filename)
__int64 _cdecl Hook_FConfigCacheIni_GetArray(FConfigCacheIni *_FConfigCacheIni, const wchar_t* Section, const wchar_t* Key, TArray<FString> *out_Arr, FString* Filename)
{
	std::string section = FromWStringToString(Section);
	std::string key = FromWStringToString(Key);
	std::string filename = Filename == nullptr ? "" : Filename->ToString();

	__int64 result = FConfigCacheIni_GetArray_original(_FConfigCacheIni, Section, Key, out_Arr, Filename);
	std::list<std::string> values;
	if (out_Arr != nullptr) {
		for (uint32_t i = 0; i < out_Arr->Num(); i++) {
			std::string value = (*out_Arr)[i].ToString();
			values.push_back(value);
		}
	}

	std::string ckey = section + "_" + key;
	auto iter = configDumpJson.find(ckey);
	if (iter == configDumpJson.end()) {
		configDumpJson[ckey] = nlohmann::json({
			{ "Section", section },
			{ "Key", key },
			{ "Filename", filename },
			{ "Values", values },
			{ "IsSet", (bool)result },
			{ "IsArray", true } });
	}

	return result;
}

//char __fastcall FConfigCacheIni::GetString(FConfigCacheIni *this, const wchar_t *Section, const wchar_t *Key, FString *Value, FString *Filename)
char _cdecl Hook_FConfigCacheIni_GetString(FConfigCacheIni *_FConfigCacheIni, const wchar_t* Section, const wchar_t* Key, FString* Value, FString* Filename)
{
	std::string section = FromWStringToString(Section);
	std::string key = FromWStringToString(Key);
	std::string filename = Filename == nullptr ? "" : Filename->ToString();
	
	char result = FConfigCacheIni_GetString_original(_FConfigCacheIni, Section, Key, Value, Filename);
	//std::cout << "[API] FConfigCacheIni::GetString[" << (int)result << "]: " << section << ", " << key << ", " << (Filename == nullptr ? "[null]" : Filename->ToString()) << " (" << (Value == nullptr ? "[null]" : Value->ToString()) << ")" << std::endl;
	std::string value = Value == nullptr ? "" : Value->ToString();

	std::string ckey = section + "_" + key;
	auto iter = configDumpJson.find(ckey);
	if (iter == configDumpJson.end()) {
		configDumpJson[ckey] = nlohmann::json({
			{ "Section", section },
			{ "Key", key },
			{ "Filename", filename },
			{ "Value", value },
			{ "IsSet", (bool)result } });
	}

	return result;
}


//void _cdecl Hook_AShooterGameMode_CheckIsOfficialServer(AShooterGameMode* _AShooterGameMode)
//{
//	std::cout << "[API] AShooterGameMode::CheckIsOfficialServer was called" << std::endl;
//
//	AShooterGameMode_CheckIsOfficialServer_original(_AShooterGameMode);
//	_AShooterGameMode->SetbIsOfficialServerField(true);
//}