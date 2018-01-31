#pragma once
#include "../Plugin.h"

DECLARE_COMMAND(Health_Chat);
DECLARE_COMMAND(Health_Console);

void HealthChatCommand(AShooterPlayerController* aShooterPlayerController, FString* msg, EChatSendMode::Type mode);
void HealthConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog);
std::list<std::string> HealthInternal(AShooterPlayerController* aShooterPlayerController, ArkLibrary::CommandDefinition *cmd);