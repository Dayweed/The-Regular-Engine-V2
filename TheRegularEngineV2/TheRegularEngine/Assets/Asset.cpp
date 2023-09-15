#include "pch.h"
#include "Asset.h"
#include <random>

namespace TRE
{
	AssetHandle Asset::GenerateGUID()
	{
		auto currentTime = std::chrono::system_clock::now().time_since_epoch().count();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<AssetHandle> dis(0, std::numeric_limits<AssetHandle>::max());
		auto randomNumber = dis(gen);
		std::string combinedString = std::to_string(currentTime) + std::to_string(randomNumber);
		std::hash<std::string> hasher;
		std::size_t hashedValue = hasher(combinedString);

		return hashedValue;
	}
}