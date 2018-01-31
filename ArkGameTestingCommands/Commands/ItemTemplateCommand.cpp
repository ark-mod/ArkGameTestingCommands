#include "ItemTemplateCommand.h"

void ItemTemplateConsoleCommand(APlayerController* aPlayerController, FString* msg, bool bWriteToLog)
{
	auto aShooterPlayerController = static_cast<AShooterPlayerController*>(aPlayerController);

	auto &plugin = Plugin::Get();
	auto cmd = ArkLibrary::GetCommand(CommandName_ItemTemplate_Console);

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
	
		int count;
		std::list<ArkLibrary::GiveItemDefinition> items;
	
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
	
				ArkLibrary::GiveItemDefinition giveItemDef;
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
			ArkApi::GetApiUtils().SendChatMessage(aShooterPlayerController, L"[system]", L"Failed to read template...");
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