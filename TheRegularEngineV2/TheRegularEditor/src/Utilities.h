#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glm/glm.hpp"
#include "imgui_impl_vulkan.h"

namespace TRE::Util
{
	ImTextureID GetTextureID(const VkDescriptorImageInfo& ImageInfo);

	glm::vec3 Scale(const glm::vec3& v, float desiredLength);

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale);
}