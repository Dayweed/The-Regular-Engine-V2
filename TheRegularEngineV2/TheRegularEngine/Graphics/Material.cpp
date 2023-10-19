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

		AllocateTextures();
	}

	Material::~Material()
	{

	}

	void Material::Invalidate()
	{
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			Renderer::GetMainRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_Shader->GetAllDescriptorLayout()[0], m_DescriptorSets[x]);
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
		const std::string resourceFolderPath = "../Resources/";
		const std::string resource = GetHandleHex() + ".material";
		const std::string resourcePath = resourceFolderPath + resource;

		std::filesystem::directory_entry entry(resourceFolderPath);
		if (!entry.exists())
		{
			std::filesystem::create_directory(resourceFolderPath);
		}

		std::ofstream file(resourceFolderPath + resource);
		if (!file.is_open())
		{
			TRE_CORE_ERROR("Unable to open file {0}", resourceFolderPath + resource);
			return;
		}

		file << "Shader:\n" << m_Shader->GetHandleHex() << std::endl;
		file << "Textures:\n";

		const ResourceHandle& defaultTextureHandle = VulkanTexture::GetDefaultTextureID();

		for (const auto& [Name, texture] : m_Textures)
		{
			file << Name << " | ";
			if(texture->GetHandle() == defaultTextureHandle)
				file << "0" << std::endl;
			else
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
		std::unordered_map<std::string, std::string> textureGUIDs;

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
					//Split line into texture name and texture GUID
					const std::string name = line.substr(0, line.find(" | "));
					const std::string textureGUID = line.substr(line.find(" | ") + 3);
					textureGUIDs[name] = textureGUID;
				}
			}
		}

		auto ShaderAsset = ResourceManager::Instance().GetResource<Shader>(Resource::GetGUIDFromHex(ShaderGUID));
		std::unique_ptr<Material> mat = std::make_unique<Material>(ShaderAsset);
		ResourceHandle assetHandle = Resource::GetGUIDFromHex(assetHexGUID);
		mat->m_Handle = assetHandle;

		for (const auto& [Name, GUID] : textureGUIDs)
		{
			if (GUID == "0")
			{
				mat->m_Textures[Name] = ResourceManager::Instance().GetResource<VulkanTexture>(VulkanTexture::GetDefaultTextureID());
			}
			else
			{
				mat->m_Textures[Name] = ResourceManager::Instance().GetResource<VulkanTexture>(Resource::GetGUIDFromHex(GUID));
			}
		}

		for (auto x : mat->m_Textures)
		{
			if (x.second == nullptr)
				std::cout << "nullptr\n";
			//std::cout << x.first << " | " << x.second->GetHandleHex() << std::endl;
		}

		ResourceManager::Instance().AddResource(std::move(mat));

		return std::move(ResourceManager::Instance().GetResource<Material>(assetHandle));
	}

	void Material::AllocateTextures()
	{
		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				m_Textures[Name] = ResourceManager::Instance().GetResource<VulkanTexture>(VulkanTexture::GetDefaultTextureID());
		}
	}

	void MaterialDescriptorFile::Generate()
	{
		const std::string& handleHex = Resource::GetGUIDHex(Resource::GenerateGUID());

		const std::string assetFolderPath = "../Assets/";
		const std::string resourceFolderPath = "../Resources/";
		const std::string resource = handleHex + ".material";
		const std::string descPath = assetFolderPath + resource + ".desc";
		const std::string resourcePath = resourceFolderPath + resource;
		SetAssetPath("Material_Instance.material");
		SetDescriptorPath(descPath);
		SetResourcePath(resourcePath);
		GenerateDescriptorFile();
	}

	void MaterialDescriptorFile::Rename(const std::string& newName)
	{
		SetAssetPath("Material_Instance.material");
		GenerateDescriptorFile();
	}

	/*void MaterialDescriptorFile::Write()
	{
		m_DescriptorFile << "Resource:\n";
		m_DescriptorFile << m_ResourcePath << "\n\n";
	}

	void MaterialDescriptorFile::Read()
	{
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Resource:")
		{
			std::getline(m_DescriptorFile, line);
			m_ResourcePath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Material resource missing" << std::endl;
			return;
		}
	}*/
}