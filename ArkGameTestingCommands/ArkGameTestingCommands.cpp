#include <fstream>
#include "Plugin.h"
#include "Commands/HealthCommand.h"
#include "Commands/ItemTemplateCommand.h"
#include "Commands/SpawnTemplateCommand.h"
#include "Commands/TeleportToCommand.h"

#pragma comment(lib, "ArkApi.lib")
#pragma comment(lib, "ArkPluginLibrary.lib")

DECLARE_COMMAND(ReloadConfig_Console)

void LoadConfig()
{
	auto& plugin = Plugin::Get();
	std::ifstream file(ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/ArkGameTestingCommands/config.json");
	if (!file.is_open())
	{
		Log::GetLog()->error("Could not open file config.json");
		throw;
	}
	 
	file >> plugin.json;
	file.close();
}

void ReloadConfig(APlayerController* playerController, FString* cmd, bool shouldLog)
{
	auto& plugin = Plugin::Get();

	LoadConfig();
	ArkLibrary::ReloadInternalSettingsForCommandsAndFeatures(plugin.json);

	auto aShooterPlayerController = static_cast<AShooterPlayerController*>(playerController);
	ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"Test config reloaded!");
}

void Load()
{
	Log::Get().Init("ArkCrossServerChat");

	auto& plugin = Plugin::Get();
	auto& commands = ArkApi::GetCommands();

	LoadConfig();
	ArkLibrary::LoadCommandsAndFeatures(plugin.json);

	// chat
	ArkLibrary::AddCommand(CommandName_Health_Chat,
		[&commands](wchar_t* name) { commands.AddChatCommand(name, &HealthChatCommand); });

	// console
	ArkLibrary::AddCommand(CommandName_ReloadConfig_Console,
		[&commands](wchar_t* name) { commands.AddConsoleCommand(name, &ReloadConfig); });
	ArkLibrary::AddCommand(CommandName_Health_Console,
		[&commands](wchar_t* name) { commands.AddConsoleCommand(name, &HealthConsoleCommand); });
	ArkLibrary::AddCommand(CommandName_ItemTemplate_Console,
		[&commands](wchar_t* name) { commands.AddConsoleCommand(name, &ItemTemplateConsoleCommand); });
	ArkLibrary::AddCommand(CommandName_SpawnTemplate_Console,
		[&commands](wchar_t* name) { commands.AddConsoleCommand(name, &SpawnTemplateConsoleCommand); });
	ArkLibrary::AddCommand(CommandName_TeleportTo_Console,
		[&commands](wchar_t* name) { commands.AddConsoleCommand(name, &TeleportToConsoleCommand); });
}

void Unload()
{
	auto& plugin = Plugin::Get();
	auto& commands = ArkApi::GetCommands();

	// chat
	ArkLibrary::RemoveCommand(CommandName_Health_Chat,
		[&commands](wchar_t* name) { commands.RemoveChatCommand(name); });

	// console
	ArkLibrary::RemoveCommand(CommandName_ReloadConfig_Console,
		[&commands](wchar_t* name) { commands.RemoveConsoleCommand(name); });
	ArkLibrary::RemoveCommand(CommandName_Health_Console,
		[&commands](wchar_t* name) { commands.RemoveConsoleCommand(name); });
	ArkLibrary::RemoveCommand(CommandName_ItemTemplate_Console,
		[&commands](wchar_t* name) { commands.RemoveConsoleCommand(name); });
	ArkLibrary::RemoveCommand(CommandName_SpawnTemplate_Console,
		[&commands](wchar_t* name) { commands.RemoveConsoleCommand(name); });
	ArkLibrary::RemoveCommand(CommandName_TeleportTo_Console,
		[&commands](wchar_t* name) { commands.RemoveConsoleCommand(name); });
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Unload();
		break;
	}
	return TRUE;
}