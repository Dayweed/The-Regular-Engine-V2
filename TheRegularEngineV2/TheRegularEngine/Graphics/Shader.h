#pragma once
#include "ShaderResource.h"
#include "Assets/Asset.h"

namespace TRE
{
	class Shader : public Asset
	{
		public:
			Shader() = default;
			Shader(const std::filesystem::path& ShaderPath);
			~Shader();
			
			void LoadAndCreateShader(const std::vector<uint32_t>& ShaderBinary, VkShaderStageFlagBits ShaderStage);
			void SetReflectionData(const ShaderReflectionData& ReflectionData);
			void CreateDescriptors();

			std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorBindings();
			std::vector<PushConstants>& GetPushConstants();

			static AssetType GetType() { return AssetType::Shader; }

		public:
			VkPipelineShaderStageCreateInfo GetPipelineShaderInfo();
			const std::unordered_map<std::string, VkWriteDescriptorSet>& GetWriteDescriptorSets();

		private:
			VkPipelineShaderStageCreateInfo m_PipelineShaderCreateInfo;
			std::vector<uint32_t> m_ShaderBinary;
			ShaderReflectionData m_ReflectionData;

			std::vector<VkDescriptorSetLayoutBinding> m_DescriptorBindings;
			VkDescriptorSet m_DescriptorSet;

			std::filesystem::path m_ShaderPath;
			std::string m_ShaderName;

			std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> m_Types;

			friend class ShaderCompiler;
	};
}