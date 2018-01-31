#pragma once
#include "../Plugin.h"
#include <ArkPluginLibrary/SpawnCustomDino.h>

DECLARE_COMMAND(SpawnTemplate_Console);

void SpawnTemplateConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog);