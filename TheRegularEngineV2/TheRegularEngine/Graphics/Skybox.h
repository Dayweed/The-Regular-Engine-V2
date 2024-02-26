#pragma once
#include "VulkanTexture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Material.h"

namespace TRE
{
	class Skybox
	{
		public:
			Skybox();
			~Skybox();

			void RecreateCubeMap();
			void ReloadCubeMap();
			void UpdateMaterial(std::shared_ptr<UniformBuffer> UBO, uint32_t Index, VkDescriptorImageInfo image, bool iseditor);

			void SetTexture(int index, std::shared_ptr<VulkanTexture> newtexture);

			VkBuffer GetVertexBuffer() { return m_SkyboxVertexBuffer->GetBuffer(); }
			VkBuffer GetIndexBuffer() { return m_SkyboxIndexBuffer->GetBuffer(); }
			uint32_t GetIndexCount() { return m_SkyboxIndexBuffer->GetIndexCount(); }
			std::shared_ptr<Material> GetMaterial() { return m_SkyboxMaterial; }

		private:
			std::shared_ptr<VulkanTexture> m_Textures[6]; //Skybox should always consist of only 6 textures
			std::shared_ptr<VulkanTexture> m_SkyboxTexture; //The cubemap to render
			std::shared_ptr<Material> m_SkyboxMaterial;

			std::unique_ptr<VertexBuffer> m_SkyboxVertexBuffer;
			std::unique_ptr<IndexBuffer> m_SkyboxIndexBuffer;
	};
}