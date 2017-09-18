#include "GiveCustomItems.h"

bool GiveCustomItems(
	unsigned long long steamId,
	std::list<GiveItemDefinition> items)
{
	AShooterPlayerController* aShooterPC = FindPlayerControllerFromSteamId(steamId);
	if (aShooterPC)
	{
		//give items
		for (std::list<GiveItemDefinition>::iterator it = items.begin(); it != items.end(); ++it)
		{
			std::list<std::string> blueprints;
			if (it->blueprints.size() > 0) blueprints = it->blueprints;
			else blueprints.push_back(it->blueprint);

			for (std::list<std::string>::iterator it2 = blueprints.begin(); it2 != blueprints.end(); ++it2)
			{
				std::wstring itemBlueprint = ConvertToWideStr(*it2);

				//remove "Blueprint'" from start
				std::wstring rBp = L"Blueprint'";
				size_t fBp = itemBlueprint.find(rBp);
				if (fBp == 0) itemBlueprint.replace(fBp, rBp.length(), L"");

				//remove ending single quote
				std::wstring rQ = L"'";
				size_t fQ = itemBlueprint.rfind(rQ);
				if (fQ == itemBlueprint.length() - 1) itemBlueprint.replace(fQ, rQ.length(), L"");

				//append _C if missing
				if (!(itemBlueprint.rfind(L"_C") == itemBlueprint.length() - 2 || itemBlueprint.rfind(L"_c") == itemBlueprint.length() - 2)) itemBlueprint.append(L"_C");

				UObject* object = Globals::StaticLoadObject(UObject::StaticClass(), nullptr, itemBlueprint.c_str(), nullptr, 0, 0, true);
				if (object && ((object->GetClassField()->GetClassCastFlagsField() >> 5) & 1))
				{
					TSubclassOf<UPrimalItem> archetype;
					archetype.uClass = reinterpret_cast<UClass*>(object);

					AShooterCharacter* aShooterCharacter = static_cast<AShooterCharacter*>(aShooterPC->GetCharacterField());

					for (int n = 0; n < it->count; n++)
					{
						UPrimalItem* item = UPrimalItem::AddNewItem(archetype, aShooterCharacter->GetMyInventoryComponentField(), false, false, it->quality, false, it->quantity, false, 0.0, false, TSubclassOf<UPrimalItem>());
					}
				}
			}
		}

		return true;
	}

	return false;
}