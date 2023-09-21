#include "pch.h"
#include "Resource.h"
#include <random>

namespace TRE
{
	ResourceHandle Resource::GenerateGUID()
	{
		auto currentTime = std::chrono::system_clock::now().time_since_epoch().count();

		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<ResourceHandle> dis(0, std::numeric_limits<ResourceHandle>::max());
		auto randomNumber = dis(gen);
		std::string combinedString = std::to_string(currentTime) + std::to_string(randomNumber);
		std::hash<std::string> hasher;
		std::size_t hashedValue = hasher(combinedString);

		return hashedValue;
	}

	std::string Resource::GetGUIDHex(const ResourceHandle assetHandle)
	{
		std::stringstream ss;
		ss << std::hex << assetHandle;
		return ss.str();
	}

	ResourceHandle Resource::GetGUIDFromHex(const std::string& GUID)
	{
		return std::stoull(GUID, nullptr, 16);
	}

	ResourceHandle Resource::GetGUIDFromPath(const std::string& path)
	{
		std::string guid = path.substr(path.find_last_of('/') + 1);
		guid = guid.substr(0, guid.find_first_of('.'));
		return GetGUIDFromHex(guid);
	}
}