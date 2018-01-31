#include "TeleportToCommand.h"

void TeleportToConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog)
{
	auto aShooterPlayerController = static_cast<AShooterPlayerController*>(aPlayerController);

	auto &plugin = Plugin::Get();
	auto cmd = ArkLibrary::GetCommand(CommandName_TeleportTo_Console);

	TArray<FString> Parsed;
	msg->ParseIntoArray(Parsed, L" ", true);

	if (Parsed.IsValidIndex(1))
	{
		std::string templateName = Parsed[1].ToString();

		auto templatesList = cmd->Json["Locations"];

		auto templateEntryIter = templatesList.find(templateName);
		if (templateEntryIter == templatesList.end())
		{
			ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"The template does not exist! Maybe you forgot to reload..?");
			return;
		}

		auto templateEntry = templateEntryIter.value();
		
		__int64 steamId = ArkApi::GetApiUtils().GetSteamIdFromController(aShooterPlayerController);
		
		float x, y, z;
		
		try
		{
			x = templateEntry.value("x", 0.0);
			y = templateEntry.value("y", 0.0);
			z = templateEntry.value("z", 0.0);
		}
		catch (const std::exception&)
		{
			ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"Failed to read template...");
			return;
		}
		
		aShooterPlayerController->SetPlayerPos(x, y, z);
	}
}