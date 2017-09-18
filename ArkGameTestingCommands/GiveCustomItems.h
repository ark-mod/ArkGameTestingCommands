#pragma once
#include "API/Base.h"
#include "API/UE.h"
#include "Tools.h"
#include "GiveItemsTemplate.h"
#include <list>
#include <iostream>

bool GiveCustomItems(
	unsigned long long steamId,
	std::list<GiveItemDefinition> items);