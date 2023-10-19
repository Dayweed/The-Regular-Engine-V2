#pragma once
#include "ShaderResource.h"
#include "Resource/Resource.h"
#include "DescriptorFile.h"

namespace TRE
{
	class ShaderDescriptorFile : public DescriptorFile
	{
		//Asset path will be the shader path
	public:
		void Load(const std::string& shaderName, const std::string& hexHandle);
	protected:
	private:
	};

	class Shader : public Resource
	{
		public:
			Shader() = default;
			Shader(const std::filesystem::path& ShaderPath);
			~Shader();
			
			void LoadAndCreateShader(const std::map<VkShaderStageFlagBits, std::vector<uint32_t>>& ShaderBinary);
			void SetReflectionData(const ShaderReflectionData& ReflectionData);
			void CreateDescriptors();

			std::vector<PushConstants>& GetPushConstants();
			std::vector<VkDescriptorSetLayout> GetAllDescriptorLayout();
			std::map<std::string, VkWriteDescriptorSet> GetWriteDescriptors();

			static ResourceType GetType() { return ResourceType::Shader; }

			static void SetupShaders();

		public:
			const std::vector<VkPipelineShaderStageCreateInfo>& GetPipelineShaderInfo();
			const std::vector<VkVertexInputAttributeDescription>& GetVertexAttributes();
			uint32_t GetVertexStrides();

		private:
			std::vector<VkPipelineShaderStageCreateInfo> m_PipelineShaderCreateInfos;
			std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts;

			std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_ShaderBinary;
			ShaderReflectionData m_ReflectionData;

			VkDescriptorSet m_DescriptorSet;

			std::filesystem::path m_ShaderPath;
			std::string m_ShaderName;

			std::unordered_map<uint32_t, std::vector<VkDescriptorPoolSize>> m_Types;

			friend class ShaderCompiler;
	};
}