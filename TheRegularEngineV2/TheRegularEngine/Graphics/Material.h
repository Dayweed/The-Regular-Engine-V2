#pragma once
#include "Shader.h"
#include "VulkanTexture.h"
#include "UniformBuffer.h"
#include "Resource/Resource.h"

namespace TRE
{
	class Material : public Resource
	{
		public:
			Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader);
			//Material(std::shared_ptr<Material> CopyMaterial);
			~Material();

			void Invalidate();
			void AllocateLayouts();
			void UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);

			//This should be removed eventually
			void SetTextures(std::shared_ptr<VulkanTexture> Textures) { m_Textures.push_back(Textures); }

			const VkDescriptorSet& GetDescriptor(uint32_t FrameIndex);
			std::vector<std::shared_ptr<VulkanTexture>>& GetTextures() { return m_Textures; }

			static ResourceType GetType() {return ResourceType::Material;}

			void Serialize() override;
			static std::shared_ptr<Material> Deserialize(const std::string& assetHexGUID);
		private:
			std::shared_ptr<Shader> m_VertexShader;
			std::shared_ptr<Shader> m_FragmentShader;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
			VkDescriptorSetLayout m_DescriptorSetLayout;

			std::vector<std::shared_ptr<VulkanTexture>> m_Textures;
	};
}