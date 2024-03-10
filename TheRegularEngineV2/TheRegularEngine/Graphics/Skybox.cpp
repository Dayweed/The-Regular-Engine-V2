#include "pch.h"
#include "Skybox.h"
#include "Resource/ResourceManager.h"
#include "VulkanUtilities.h"

namespace TRE
{
	Skybox::Skybox()
	{
		//Backface culling
		std::vector<glm::vec3> vertices
		{
				glm::vec3(-0.5f, -0.5f, -0.5f), // Vertex 0
				glm::vec3(0.5f, -0.5f, -0.5f), // Vertex 1
				glm::vec3(0.5f, 0.5f, -0.5f), // Vertex 2
				glm::vec3(-0.5f, 0.5f, -0.5f), // Vertex 3
				glm::vec3(-0.5f, -0.5f, 0.5f), // Vertex 4
				glm::vec3(0.5f, -0.5f, 0.5f), // Vertex 5
				glm::vec3(0.5f, 0.5f, 0.5f), // Vertex 6
				glm::vec3(-0.5f, 0.5f, 0.5f) // Vertex 7
		};
		std::vector<uint32_t> indices
		{
			0, 1, 2, // Triangle 1 (front face)
			2, 3, 0, // Triangle 2 (front face)
			1, 5, 6, // Triangle 3 (right face)
			6, 2, 1, // Triangle 4 (right face)
			7, 6, 5, // Triangle 5 (back face)
			5, 4, 7, // Triangle 6 (back face)
			4, 0, 3, // Triangle 7 (left face)
			3, 7, 4, // Triangle 8 (left face)
			4, 5, 1, // Triangle 9 (bottom face)
			1, 0, 4, // Triangle 10 (bottom face)
			3, 2, 6, // Triangle 11 (top face)
			6, 7, 3  // Triangle 12 (top face)
		};

		m_SkyboxVertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(vertices.data()),
			UINT32_T_CAST(vertices.size() * sizeof(vertices[0])));

		m_SkyboxIndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(indices.data()),
			UINT32_T_CAST(indices.size() * sizeof(uint32_t)),
			UINT32_T_CAST(indices.size()));

		auto Skybox1 = Resource::GetGUIDFromHex("1378dc636337bd6f");
		auto Skybox2 = Resource::GetGUIDFromHex("8b8b3e5f06862842");
		auto Skybox3 = Resource::GetGUIDFromHex("d34a9554a71b1295");
		auto Skybox4 = Resource::GetGUIDFromHex("6ba20f52c80692f0");
		auto Skybox5 = Resource::GetGUIDFromHex("5e46643065bc4883");
		auto Skybox6 = Resource::GetGUIDFromHex("3f8f723d38a469d6");
		auto Texture1 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox1);
		auto Texture2 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox2);
		auto Texture3 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox3);
		auto Texture4 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox4);
		auto Texture5 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox5);
		auto Texture6 = ResourceManager::Instance().GetResource<VulkanTexture>(Skybox6);

		m_Textures[0] = Texture1;
		m_Textures[1] = Texture2;
		m_Textures[2] = Texture3;
		m_Textures[3] = Texture4;
		m_Textures[4] = Texture5;
		m_Textures[5] = Texture6;

		RecreateCubeMap();

		m_SkyboxMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(3));
		m_SkyboxMaterial->Invalidate();
		ReloadCubeMap();
	}

	Skybox::~Skybox()
	{

	}

	void Skybox::SetTexture(int index, std::shared_ptr<VulkanTexture> newtexture)
	{
		m_Textures[index] = newtexture;
	}

	void Skybox::ReloadCubeMap()
	{
		m_SkyboxMaterial->SetTexture("SamplerCubeMap", m_SkyboxTexture);
	}

	void Skybox::RecreateCubeMap()
	{
		CubeMapConfig CubeConfig{};
		CubeConfig.Filter = VK_FILTER_NEAREST;
		CubeConfig.Format = m_Textures[0]->GetFormat();
		CubeConfig.Height = m_Textures[0]->GetHeight();
		CubeConfig.Width = m_Textures[0]->GetWidth();
		CubeConfig.SamplerAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		CubeConfig.Textures = { m_Textures[3], m_Textures[1], m_Textures[5], m_Textures[4], m_Textures[0], m_Textures[2] };

		m_SkyboxTexture.reset();

		m_SkyboxTexture = std::make_shared<VulkanTexture>(CubeConfig);
	}

	void Skybox::UpdateMaterial(std::shared_ptr<UniformBuffer> UBO, uint32_t Index, VkDescriptorImageInfo image, bool iseditor)
	{
		if (iseditor)
		{
			m_SkyboxMaterial->UpdateForEditorSceneRendering(UBO, Index);
		}
		else
		{
			m_SkyboxMaterial->UpdateForRendering(UBO, Index);
		}
	}
}