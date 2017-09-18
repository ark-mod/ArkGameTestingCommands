#include "Tools.h"
#include <codecvt>

AShooterPlayerController* FindPlayerControllerFromSteamId(unsigned __int64 steamId)
{
	AShooterPlayerController* result = nullptr;

	auto playerControllers = Ark::GetWorld()->GetPlayerControllerListField();
	for (uint32_t i = 0; i < playerControllers.Num(); ++i)
	{
		auto playerController = playerControllers[i];

		APlayerState* playerState = playerController->GetPlayerStateField();
		__int64 currentSteamId = playerState->GetUniqueIdField()->UniqueNetId->GetUniqueNetIdField();

		if (currentSteamId == steamId)
		{
			AShooterPlayerController* aShooterPC = static_cast<AShooterPlayerController*>(playerController.Get());

			result = aShooterPC;
			break;
		}
	}

	return result;
}

wchar_t* ConvertToWideStr(const std::string& str)
{
	size_t newsize = str.size() + 1;

	wchar_t* wcstring = new wchar_t[newsize];

	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, str.c_str(), _TRUNCATE);

	return wcstring;
}

std::string FromWStringToString(const std::wstring &s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv1;
	std::string u8str = conv1.to_bytes(s);
	return u8str;
}

std::string GetCurrentDir()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(nullptr, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	return std::string(buffer).substr(0, pos);
}

__int64 GetSteamId(AShooterPlayerController* playerController)
{
	__int64 steamId = 0;

	APlayerState* playerState = playerController->GetPlayerStateField();
	if (playerState)
	{
		steamId = playerState->GetUniqueIdField()->UniqueNetId->GetUniqueNetIdField();
	}

	return steamId;
}

std::string GetUClassName(UClass* uClass) {
	if (uClass == nullptr) return "[null]";

	FString* className = new FString();
	FName bp = uClass->GetNameField();
	bp.ToString(className);
	std::string result = className->ToString();
	delete className;

	return result;
}

bool IsPointInsideSphere(FVector point, float sphereX, float sphereY, float sphereZ, float sphereRadius)
{
	long double x = point.X - sphereX;
	long double y = point.Y - sphereY;
	long double z = point.Z - sphereZ;

	long double distancesq = x * x + y * y + z * z;
	return distancesq < sphereRadius * sphereRadius;
}

bool IsPointInside2dCircle(FVector point, float circleX, float circleY, float circleRadius)
{
	long double x = point.X - circleX;
	long double y = point.Y - circleY;

	long double distancesq = x * x + y * y;
	return distancesq < circleRadius * circleRadius;
}