#pragma once
#include "vulkan/vulkan.h"

namespace TRE
{
	struct UniformBuffer_GLSL
	{
		VkDescriptorBufferInfo DescriptorInfo;
		VkShaderStageFlagBits ShaderStageFlag = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t Binding = 0;
		uint32_t Size = 0;
		std::string Name;

	};

	struct PushConstants
	{
		VkShaderStageFlagBits ShaderStageFlag = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t Offset = 0;
		uint32_t Size = 0;
	};

	struct ImageSampler
	{

	};

	struct ShaderDescriptorSets
	{
		std::vector<UniformBuffer_GLSL> UniformBuffers;
		std::vector<ImageSampler> ImageSamplers;

		std::unordered_map<std::string, VkWriteDescriptorSet> WriteDescriptorSets;

		operator bool() const { return !(UniformBuffers.empty() && ImageSamplers.empty()); }
	};

	struct ShaderReflectionData
	{
		std::vector<ShaderDescriptorSets> DescriptorSets;
		std::vector<PushConstants> PushConstants;
	};
}