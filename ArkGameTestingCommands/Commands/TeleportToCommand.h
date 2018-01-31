#pragma once
#include "../Plugin.h"

DECLARE_COMMAND(TeleportTo_Console);

void TeleportToConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog);