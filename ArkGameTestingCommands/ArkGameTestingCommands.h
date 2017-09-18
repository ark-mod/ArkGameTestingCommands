#pragma once
#include "API/Base.h"
#include "API/UE.h"

void SpawnCustomDino(unsigned long long steamId, TArray<FString> &Parsed, int dinoBaseLevelHealth, int dinoBaseLevelStamina, int dinoBaseLevelOxygen, int dinoBaseLevelFood, int dinoBaseLevelWeight, int dinoBaseLevelMeleeDamage, int dinoBaseLevelMovementSpeed, int dinoTamedLevelHealth, int dinoTamedLevelStamina, int dinoTamedLevelOxygen, int dinoTamedLevelFood, int dinoTamedLevelWeight, int dinoTamedLevelMeleeDamage, int dinoTamedLevelMovementSpeed, float saddleQuality, AShooterPlayerController * aShooterController);
