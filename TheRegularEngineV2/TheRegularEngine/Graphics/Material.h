#pragma once
#include "Shader.h"
#include "VulkanTexture.h"
#include "UniformBuffer.h"
#include "Resource/Resource.h"
#include "DescriptorFile.h"

namespace TRE
{
	struct MaterialUBO
	{
		glm::vec4 m_Color;
	};

	class MaterialDescriptorFile : public DescriptorFile
	{
	public:
		//Asset path will act as material name 
		void Generate();
		void Rename(const std::string& newName);
		ResourceHandle GetResourceHandle();
	protected:
		void Write() override;
		void Read() override;
	};

	class Material : public Resource
	{
		public:
			Material(const std::shared_ptr<Shader>& Shader);
			Material(const ResourceHandle& handle);
			~Material();

			void Invalidate();
			void UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index, const VkDescriptorImageInfo& ShadowMap);
			void UpdateForEditorSceneRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index, const VkDescriptorImageInfo& ShadowMap = VkDescriptorImageInfo());
			void UpdateCompsitePass(const VkDescriptorImageInfo& ImageInfo); //To be removed

			void SetTexture(std::string Name, std::shared_ptr<VulkanTexture> textures);

			const std::unordered_map<std::string, std::shared_ptr<VulkanTexture>>& GetTextures() { return m_Textures; }
			std::unordered_map<std::string, std::shared_ptr<VulkanTexture>>& GetTexturesRef() { return m_Textures; }

			const VkDescriptorSet& GetDescriptor(uint32_t FrameIndex);
			const VkDescriptorSet& GetEditorDescriptor(uint32_t FrameIndex) { return m_EditorDescriptorSets[FrameIndex]; }

			MaterialUBO& GetMaterialUBO() { return m_UBO; }
			void SetMaterialUBO() { m_MaterialUBO->SetData(&m_UBO, sizeof(MaterialUBO)); }
			void SetUBOData(glm::vec4 Test) { m_UBO.m_Color = Test; }

			static ResourceType GetType() { return ResourceType::Material; }

			void Serialize() override;
			static std::shared_ptr<Material> Deserialize(const std::string& assetHexGUID);

			bool IsValid() { return m_IsValid; }
		private:
			void AllocateTextures();
		private:
			std::shared_ptr<Shader> m_Shader;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			std::vector<VkDescriptorSet> m_EditorDescriptorSets;
			std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
			std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> m_Textures;

			std::shared_ptr<UniformBuffer> m_MaterialUBO;
			MaterialUBO m_UBO;

			bool m_IsValid = false;
	};
}