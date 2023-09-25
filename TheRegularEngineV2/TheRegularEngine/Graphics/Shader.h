#pragma once
#include "ShaderResource.h"
#include "Resource/Resource.h"

namespace TRE
{
	class Shader : public Resource
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

			static ResourceType GetType() { return ResourceType::Shader; }

			void Serialize() override;

		public:
			VkPipelineShaderStageCreateInfo GetPipelineShaderInfo();
			std::unordered_map<std::string, VkWriteDescriptorSet> GetWriteDescriptorSets();
			const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributes();
			uint32_t GetVertexStrides();

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