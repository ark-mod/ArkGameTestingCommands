#define _USE_MATH_DEFINES
#include <cmath>
#include "SpawnTemplateCommand.h"

void SpawnTemplateConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog)
{
	auto aShooterPlayerController = static_cast<AShooterPlayerController*>(aPlayerController);

	auto &plugin = Plugin::Get();
	auto cmd = ArkLibrary::GetCommand(CommandName_SpawnTemplate_Console);

	TArray<FString> Parsed;
	msg->ParseIntoArray(Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		std::string templateName = Parsed[1].ToString();

		auto templatesList = cmd->Json["Templates"];

		auto templateEntryIter = templatesList.find(templateName);
		if (templateEntryIter == templatesList.end())
		{
			ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"The template does not exist! Maybe you forgot to reload..?");
			return;
		}

		auto templateEntry = templateEntryIter.value();

		__int64 steamId = ArkApi::GetApiUtils().GetSteamIdFromController(aShooterPlayerController);

		int dinoBaseLevelHealth, dinoBaseLevelStamina, dinoBaseLevelOxygen, dinoBaseLevelFood, dinoBaseLevelWeight, dinoBaseLevelMeleeDamage, dinoBaseLevelMovementSpeed;
		int dinoTamedLevelHealth, dinoTamedLevelStamina, dinoTamedLevelOxygen, dinoTamedLevelFood, dinoTamedLevelWeight, dinoTamedLevelMeleeDamage, dinoTamedLevelMovementSpeed;
		float saddleArmor;
		float imprint;
		int count;
		float radius;
		bool follow;
		bool ignoreAllWhistles;
		bool ignoreAllyLook;
		std::list<ArkLibrary::GiveItemDefinition> items;

		std::string bpPath = templateEntry["blueprint"];
		std::string bpPathSaddle = templateEntry.value("saddleBlueprint", std::string());

		std::string aggressionLevelStr = ArkLibrary::str_tolower(templateEntry.value("aggressionLevel", std::string()));
		ArkLibrary::AggressionLevel aggressionLevel = ArkLibrary::AggressionLevel::Passive;
		if (aggressionLevelStr.compare("passive") == 0) aggressionLevel = ArkLibrary::AggressionLevel::Passive;
		else if (aggressionLevelStr.compare("neutral") == 0) aggressionLevel = ArkLibrary::AggressionLevel::Neutral;
		else if (aggressionLevelStr.compare("aggressive") == 0) aggressionLevel = ArkLibrary::AggressionLevel::Aggressive;
		else if (aggressionLevelStr.compare("attackmytarget") == 0) aggressionLevel = ArkLibrary::AggressionLevel::AttackMyTarget;

		std::string facingStr = ArkLibrary::str_tolower(templateEntry.value("facing", std::string()));
		ArkLibrary::FacingDirection facing = ArkLibrary::FacingDirection::Forward;
		if (facingStr.compare("forward") == 0) facing = ArkLibrary::FacingDirection::Forward;
		else if (facingStr.compare("outwards") == 0) facing = ArkLibrary::FacingDirection::Outwards;
		else if (facingStr.compare("inwards") == 0) facing = ArkLibrary::FacingDirection::Inwards;

		try
		{
			saddleArmor = templateEntry.value("saddleArmor", 25.0);
			imprint = templateEntry.value("imprint", 0.0);
			dinoBaseLevelHealth = templateEntry.value("baseLevelHealth", 0);
			dinoBaseLevelStamina = templateEntry.value("baseLevelStamina", 0);
			dinoBaseLevelOxygen = templateEntry.value("baseLevelOxygen", 0);
			dinoBaseLevelFood = templateEntry.value("baseLevelFood", 0);
			dinoBaseLevelWeight = templateEntry.value("baseLevelWeight", 0);
			dinoBaseLevelMeleeDamage = templateEntry.value("baseLevelMeleeDamage", 0);
			dinoBaseLevelMovementSpeed = templateEntry.value("baseLevelMovementSpeed", 0);

			dinoTamedLevelHealth = templateEntry.value("tamedLevelHealth", 0);
			dinoTamedLevelStamina = templateEntry.value("tamedLevelStamina", 0);
			dinoTamedLevelOxygen = templateEntry.value("tamedLevelOxygen", 0);
			dinoTamedLevelFood = templateEntry.value("tamedLevelFood", 0);
			dinoTamedLevelWeight = templateEntry.value("tamedLevelWeight", 0);
			dinoTamedLevelMeleeDamage = templateEntry.value("tamedLevelMeleeDamage", 0);
			dinoTamedLevelMovementSpeed = templateEntry.value("tamedLevelMovementSpeed", 0);

			count = templateEntry.value("count", 1);
			radius = templateEntry.value("radius", 1000.0);
			follow = templateEntry.value("follow", false);
			ignoreAllWhistles = templateEntry.value("ignoreAllWhistles", false);
			ignoreAllyLook = templateEntry.value("ignoreAllyLook", false);

			auto itemsMap = templateEntry.value("items", nlohmann::json::array());
			for (auto iter = itemsMap.begin(); iter != itemsMap.end(); ++iter)
			{
				auto item = iter.value();

				std::string blueprint = item["blueprint"];
				int quantity = item.value("quantity", 1);
				int count = item.value("count", 1);

				ArkLibrary::GiveItemDefinition giveItemDef;
				giveItemDef.blueprint = blueprint;
				giveItemDef.quantity = quantity;
				giveItemDef.count = count;

				items.push_back(giveItemDef);
			}
		}
		catch (const std::exception&)
		{
			ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"Failed to read template...");
			return;
		}

		bool success = true;
		for (int i = 0; i < count; i++)
		{
			float a = i * 2 * M_PI / count; //in radians
			float x = radius * std::cos(a);
			float y = radius * std::sin(a);

			float f = 0.0; // in degrees
			if (facing == ArkLibrary::FacingDirection::Outwards) f = i * 360.0 / count;
			else if (facing == ArkLibrary::FacingDirection::Inwards) f = i * 360.0 / count + 180.0;

			bool innerSuccess = SpawnCustomDino(
				steamId,
				bpPath, bpPathSaddle,
				dinoBaseLevelHealth, dinoBaseLevelStamina, dinoBaseLevelOxygen, dinoBaseLevelFood, dinoBaseLevelWeight, dinoBaseLevelMeleeDamage, dinoBaseLevelMovementSpeed,
				dinoTamedLevelHealth, dinoTamedLevelStamina, dinoTamedLevelOxygen, dinoTamedLevelFood, dinoTamedLevelWeight, dinoTamedLevelMeleeDamage, dinoTamedLevelMovementSpeed,
				saddleArmor, imprint, items, x, y, 300,
				follow, aggressionLevel, ignoreAllWhistles, ignoreAllyLook, f);

			if (!innerSuccess)
			{
				success = false;
				break;
			}
		}
	}
}