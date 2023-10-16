#include "pch.h"
#include "Material.h"
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Resource/ResourceManager.h"

namespace TRE
{
	const VkDescriptorSet& Material::GetDescriptor(uint32_t FrameIndex)
	{
		return m_DescriptorSets[FrameIndex];
	}

	Material::Material(const std::shared_ptr<Shader>& Shader) : m_Shader(Shader)
	{
		m_Type = ResourceType::Material;
		auto ImageCont = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_DescriptorSets.resize(ImageCont);
	}

	Material::~Material()
	{

	}

	void Material::Invalidate()
	{
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			Engine::GetInstance().GetRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_Shader->GetAllDescriptorLayout()[0], m_DescriptorSets[x]);
		}
	}

	void Material::UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				Write.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			}
			else if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				Write.pImageInfo = &m_Textures[Name]->GetDescriptorImageInfo();
			}
			Write.dstSet = m_DescriptorSets[Index];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::SetTexture(std::string Name, std::shared_ptr<VulkanTexture> textures) 
	{
		m_Textures[Name] = textures;
	}

	void Material::Serialize()
	{
		std::string path = "../Resources/";
		std::filesystem::directory_entry entry(path);
		if (!entry.exists())
		{
			std::filesystem::create_directory(path);
		}

		path += GetHandleHex() + ".material";

		std::ofstream file(path);
		if (!file.is_open())
		{
			TRE_CORE_ERROR("Unable to open file {0}", path);
			return;
		}

		file << "Shader:\n" << m_Shader->GetHandleHex() << std::endl;
		file << "Textures:\n";
		//For loop next time
		for (auto [Name, texture] : m_Textures)
		{
			file << texture->GetHandleHex() << std::endl;
		}

		file.close();
	}

	std::shared_ptr<Material> Material::Deserialize(const std::string& assetHexGUID)
	{
		//Open material file
		std::string materialPath = "../Resources/" + assetHexGUID + ".material";
		std::ifstream file(materialPath);
		if (!file.is_open())
		{
			TRE_CORE_ERROR("Unable to open file {0}", materialPath);
			return nullptr;
		}

		std::string line;
		std::string ShaderGUID;
		std::vector<std::string> textureGUIDs;

		while (std::getline(file, line))
		{
			if (line == "Shader:")
			{
				std::getline(file, ShaderGUID);
			}
			else if (line == "Textures:")
			{
				while (std::getline(file, line))
				{
					textureGUIDs.push_back(line);
				}
			}
		}

		auto ShaderAsset = ResourceManager::Instance().GetResource<Shader>(Resource::GetGUIDFromHex(ShaderGUID));
		std::unique_ptr<Material> mat = std::make_unique<Material>(ShaderAsset);
		mat->Invalidate();
		ResourceHandle assetHandle = Resource::GetGUIDFromHex(assetHexGUID);
		mat->m_Handle = assetHandle;

		//mat->m_Textures.resize(textureGUIDs.size());
		for (int i = 0; i < textureGUIDs.size(); ++i)
		{	
			std::string textureHexGUID = textureGUIDs[i];
			auto texture = ResourceManager::Instance().GetResource<VulkanTexture>(Resource::GetGUIDFromHex(textureHexGUID));
			//Load into engine if not in asset manager
			if (texture == nullptr)
			{
				texture = VulkanTexture::Deserialize(textureHexGUID);
			}
			//mat->m_Textures[i] = texture;
		}

		ResourceManager::Instance().AddResource(std::move(mat));

		return std::move(ResourceManager::Instance().GetResource<Material>(assetHandle));
	}
}