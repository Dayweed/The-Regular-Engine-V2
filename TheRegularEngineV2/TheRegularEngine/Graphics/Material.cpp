#include "pch.h"
#include "Material.h"
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Resource/ResourceManager.h"
#include "ShaderTypes/PBRShader.h"

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
		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
			m_EditorDescriptorSets.resize(ImageCont);

		AllocateTextures();
	}

	Material::Material(const ResourceHandle& handle)
	{
		m_Type = ResourceType::Material;
		auto ImageCont = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_DescriptorSets.resize(ImageCont);
		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
			m_EditorDescriptorSets.resize(ImageCont);

		m_Shader = ResourceManager::Instance().GetResource<Shader>(handle);

		if (m_Shader == nullptr)
		{
			TRE_CORE_ERROR("Material::Material(const ResourceHandle& handle) - Shader is nullptr");
			return;
		}

		AllocateTextures();
	}

	Material::~Material()
	{
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDeviceWaitIdle(Device);

		vkFreeDescriptorSets(Device, Engine::GetInstance().GetMainSceneRenderer()->GetDescriptorPool()->GetPool(), UINT32_T_CAST(m_DescriptorSets.size()), m_DescriptorSets.data());

		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			vkFreeDescriptorSets(Device, Engine::GetInstance().GetMainSceneRenderer()->GetDescriptorPool()->GetPool(), UINT32_T_CAST(m_EditorDescriptorSets.size()), m_EditorDescriptorSets.data());
		}
	}

	void Material::Invalidate()
	{
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			Engine::GetInstance().GetMainSceneRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_Shader->GetAllDescriptorLayout()[0], m_DescriptorSets[x]);
		}

		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			for (int x = 0; x < m_EditorDescriptorSets.size(); x++)
			{
				Engine::GetInstance().GetMainSceneRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_Shader->GetAllDescriptorLayout()[0], m_EditorDescriptorSets[x]);
			}
		}

		m_IsValid = true;

		if (!m_MaterialUBO)
			m_MaterialUBO = std::make_shared<UniformBuffer>(static_cast<uint32_t>(sizeof(MaterialUBO)), 1);

		m_UBO.m_Color = { 1.f, 1.f, 1.f, 1.f };
		m_MaterialUBO->SetData(&m_UBO, sizeof(MaterialUBO));
	}

	void Material::UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		if(m_IsValid == false)
			Invalidate();

		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				if (Write.dstBinding == 6)
					Write.pBufferInfo = &m_MaterialUBO->GetDescriptorBufferInfo();
				else
					Write.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			}
			else if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (Write.dstBinding == 7)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::ShadowMap))
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::ShadowMap]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 8)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::DepthMap))
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::DepthMap]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 9)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::IDMap))
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::IDMap]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 10)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::shadowMap2))
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::shadowMap2]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 11)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::BoxBlurMap))
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::BoxBlurMap]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 12)
				{
					const auto colorImages = Engine::GetInstance().GetMainSceneRenderer()->GetColorImages();
					if (colorImages.size() > 0)
						Write.pImageInfo = &(colorImages[Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentImageIndex()]->GetDescriptorImageInfo());
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else
				{
					if(m_Textures[Name] != nullptr)
						Write.pImageInfo = &m_Textures[Name]->GetDescriptorImageInfo();
					else
						Write.pImageInfo = &VulkanTexture::GetDefaultTexture()->GetDescriptorImageInfo();
				}
			}
			Write.dstSet = m_DescriptorSets[Index];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::UpdateForEditorSceneRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				if (Write.dstBinding == 6)
					Write.pBufferInfo = &m_MaterialUBO->GetDescriptorBufferInfo();
				else
					Write.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			}
			else if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (Write.dstBinding == 7)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::ShadowMap))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::ShadowMap]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 10)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::shadowMap2))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::shadowMap2]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else
				{
					if (m_Textures[Name] != nullptr)
						Write.pImageInfo = &m_Textures[Name]->GetDescriptorImageInfo();
					else
						Write.pImageInfo = &VulkanTexture::GetDefaultTexture()->GetDescriptorImageInfo();
				}
			}
			Write.dstSet = m_EditorDescriptorSets[Index];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::UpdateForAnimationRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index, const std::shared_ptr<UniformBuffer>& uboanimation)
	{
		if (m_IsValid == false)
			Invalidate();

		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				if (Write.dstBinding == 6)
					Write.pBufferInfo = &m_MaterialUBO->GetDescriptorBufferInfo();
				else if (Write.dstBinding == 8)
					Write.pBufferInfo = &uboanimation->GetDescriptorBufferInfo();
				else
					Write.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			}
			else if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (Write.dstBinding == 7)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::ShadowMap))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::ShadowMap]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 10)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::shadowMap2))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::shadowMap2]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else
				{
					if (m_Textures[Name] != nullptr)
						Write.pImageInfo = &m_Textures[Name]->GetDescriptorImageInfo();
					else
						Write.pImageInfo = &VulkanTexture::GetDefaultTexture()->GetDescriptorImageInfo();
				}
			}
			Write.dstSet = m_DescriptorSets[Index];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::UpdateForEditorAnimationRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index, const std::shared_ptr<UniformBuffer>& uboanimation)
	{
		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
			{
				if (Write.dstBinding == 6)
					Write.pBufferInfo = &m_MaterialUBO->GetDescriptorBufferInfo();
				else if (Write.dstBinding == 8)
					Write.pBufferInfo = &uboanimation->GetDescriptorBufferInfo();
				else
					Write.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			}
			else if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				if (Write.dstBinding == 7)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::ShadowMap))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::ShadowMap]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else if (Write.dstBinding == 10)
				{
					if (SceneRenderer::m_SceneImages.contains(SceneRenderer::SceneImage::shadowMap2))
					{
						Write.pImageInfo = &(SceneRenderer::m_SceneImages[SceneRenderer::SceneImage::shadowMap2]->GetDescriptorImageInfo());
					}
					else
						Write.pImageInfo = &m_EmptyImageInfo;
				}
				else
				{
					if (m_Textures[Name] != nullptr)
						Write.pImageInfo = &m_Textures[Name]->GetDescriptorImageInfo();
					else
						Write.pImageInfo = &VulkanTexture::GetDefaultTexture()->GetDescriptorImageInfo();
				}
			}
			Write.dstSet = m_EditorDescriptorSets[Index];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::UpdateCompsitePass(const VkDescriptorImageInfo& ImageInfo)
	{
		m_WriteDescriptors.clear();

		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				Write.pImageInfo = &ImageInfo;
			}
			Write.dstSet = m_DescriptorSets[Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex()];
			m_WriteDescriptors.push_back(Write);
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::SetTexture(std::string Name, std::shared_ptr<VulkanTexture> textures)
	{
		m_Textures[Name] = textures;
	}

	const bool Material::ContainsTexture(const ResourceHandle& resourceHandle, std::string& boundedName)
	{
		for (const auto& texture : m_Textures)
		{
			if (texture.second->GetHandle() == resourceHandle)
			{
				boundedName = texture.first;
				return true;
			}
		}
		return false;
	}

	void Material::Serialize()
	{
		if (m_Handle == PBR::GetDefaultMaterial())
			return;

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
		file << "End of Textures\n";
		
		file << "Color Value:\n";
		file << m_UBO.m_Color.x << " " << m_UBO.m_Color.y << " " << m_UBO.m_Color.z << " " << m_UBO.m_Color.w;

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
		glm::vec4 Color{};
		bool HasColor = false;

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
					if (line == "End of Textures")
						break;

					//Split line into texture name and texture GUID
					const std::string name = line.substr(0, line.find(" | "));
					const std::string textureGUID = line.substr(line.find(" | ") + 3);
					textureGUIDs[name] = textureGUID;
				}
			}
			else if (line == "Color Value:")
			{
				file >> Color.x >> Color.y >> Color.z >> Color.w;
				HasColor = true;
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

		if (mat->IsValid() == false)
			mat->Invalidate();
		
		if (HasColor)
		{
			mat->SetUBOData(Color);
			mat->SetMaterialUBO();
		}

		ResourceManager::Instance().AddResource(std::move(mat));

		return std::move(ResourceManager::Instance().GetResource<Material>(assetHandle));
	}

	void Material::AllocateTextures()
	{
		for (auto& [Name, Write] : m_Shader->GetWriteDescriptors())
		{
			if (Name == "shadowMap")
				continue;

			if (Write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				m_Textures[Name] = ResourceManager::Instance().GetResource<VulkanTexture>(VulkanTexture::GetDefaultTextureID());
		}
	}

	void MaterialDescriptorFile::Generate(const std::string& assetName)
	{
		const std::string& handleHex = Resource::GetGUIDHex(Resource::GenerateGUID(assetName));

		const std::string assetFolderPath = "../Assets/";
		const std::string resourceFolderPath = "../Resources/";
		const std::string resource = handleHex + ".material";
		const std::string descPath = assetFolderPath + resource + ".desc";
		const std::string resourcePath = resourceFolderPath + resource;
		SetAssetPath(assetName);
		SetDescriptorPath(descPath);
		SetResourcePath(resourcePath);
		GenerateDescriptorFile();
	}

	void MaterialDescriptorFile::Rename(const std::string& newName)
	{
		SetAssetPath(newName + ".material");
		GenerateDescriptorFile();
	}

	ResourceHandle MaterialDescriptorFile::GetResourceHandle()
	{
		std::string hexCode = m_ResourcePath.substr(m_ResourcePath.find_last_of('/') + 1);
		hexCode = hexCode.substr(0, hexCode.find_last_of('.'));
		return Resource::GetGUIDFromHex(hexCode);
	}

	void MaterialDescriptorFile::Write()
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
	}
}