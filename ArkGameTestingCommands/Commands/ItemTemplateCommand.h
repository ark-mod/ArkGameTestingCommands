#pragma once
#include "../Plugin.h"
#include <ArkPluginLibrary/GiveCustomItems.h>

DECLARE_COMMAND(ItemTemplate_Console);

void ItemTemplateConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog);