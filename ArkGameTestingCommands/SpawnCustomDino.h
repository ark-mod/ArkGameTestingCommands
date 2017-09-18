#pragma once
#include "API/Base.h"
#include "API/UE.h"
#include "Tools.h"
#include "GiveItemsTemplate.h"
#include <list>
#include <iostream>

bool SpawnCustomDino(
	unsigned long long steamId,
	std::string bpPath, std::string bpPathSaddle,
	int dinoBaseLevelHealth, int dinoBaseLevelStamina, int dinoBaseLevelOxygen, int dinoBaseLevelFood, int dinoBaseLevelWeight, int dinoBaseLevelMeleeDamage, int dinoBaseLevelMovementSpeed,
	int dinoTamedLevelHealth, int dinoTamedLevelStamina, int dinoTamedLevelOxygen, int dinoTamedLevelFood, int dinoTamedLevelWeight, int dinoTamedLevelMeleeDamage, int dinoTamedLevelMovementSpeed,
	float saddleArmor,
	std::list<GiveItemDefinition> items,
	float offsetX, float offsetY, float offsetZ = 300.0);