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
			Material(const std::shared_ptr<Shader>& Shader);
			~Material();

			void Invalidate();
			void UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index);

			void SetTexture(std::string Name, std::shared_ptr<VulkanTexture> textures);
			std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> GetTextures() { return m_Textures; }

			const VkDescriptorSet& GetDescriptor(uint32_t FrameIndex);

			static ResourceType GetType() {return ResourceType::Material;}

			void Serialize() override;
			static std::shared_ptr<Material> Deserialize(const std::string& assetHexGUID);
		private:
			std::shared_ptr<Shader> m_Shader;

			std::vector<VkDescriptorSet> m_DescriptorSets;
			std::vector<VkWriteDescriptorSet> m_WriteDescriptors;
			std::unordered_map<std::string, std::shared_ptr<VulkanTexture>> m_Textures;
	};
}