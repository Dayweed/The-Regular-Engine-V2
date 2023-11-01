#include "pch.h"
#include "Utilities.h"

namespace TRE::Util
{
	std::shared_ptr<VulkanTexture> CreateIcon(std::string PNG_name)
	{
		const auto tmpGUID = AssetManager::Instance().GetAssetHandle(PNG_name);
		const auto tmpHexGUID = Resource::GetGUIDHex(tmpGUID);
		std::unique_ptr<VulkanTexture> tmpButton = std::make_unique<VulkanTexture>("../Resources/" + tmpHexGUID + ".DDS");
		tmpButton->SetHandle(tmpGUID);
		AssetManager::Instance().AddAsset(PNG_name, std::move(tmpButton));
		std::shared_ptr<VulkanTexture> Icon = ResourceManager::Instance().GetResource<VulkanTexture>(tmpGUID);
		return Icon;
	}

	ImTextureID GetTextureID(const VkDescriptorImageInfo& ImageInfo)
	{
		if (!ImageInfo.imageView)
			return nullptr;

		return ImGui_ImplVulkan_AddTexture(ImageInfo.sampler, ImageInfo.imageView, ImageInfo.imageLayout);
	}

	glm::vec3 Scale(const glm::vec3& v, float desiredLength)
	{
		return v * desiredLength / length(v);
	}

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
	{
		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
			return false;

		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3];

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		scale.x = length(Row[0]);
		Row[0] = Scale(Row[0], static_cast<T>(1));

		scale.y = length(Row[1]);
		Row[1] = Scale(Row[1], static_cast<T>(1));

		scale.z = length(Row[2]);
		Row[2] = Scale(Row[2], static_cast<T>(1));

		int i, j, k = 0;
		T root, trace = Row[0].x + Row[1].y + Row[2].z;
		if (trace > static_cast<T>(0))
		{
			root = sqrt(trace + static_cast<T>(1));
			rotation.w = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation.x = root * (Row[1].z - Row[2].y);
			rotation.y = root * (Row[2].x - Row[0].z);
			rotation.z = root * (Row[0].y - Row[1].x);
		} // End if > 0
		else
		{
			static int Next[3] = { 1, 2, 0 };
			i = 0;
			if (Row[1].y > Row[0].x) i = 1;
			if (Row[2].z > Row[i][i]) i = 2;
			j = Next[i];
			k = Next[j];

			root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

			rotation[i] = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation[j] = root * (Row[i][j] + Row[j][i]);
			rotation[k] = root * (Row[i][k] + Row[k][i]);
			rotation.w = root * (Row[j][k] - Row[k][j]);
		} // End if <= 0

		return true;
	}
}