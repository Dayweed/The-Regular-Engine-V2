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

	Material::Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader) : m_VertexShader(VertexShader), m_FragmentShader(FragShader)
	{
		m_Type = ResourceType::Material;
		auto ImageCont = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_DescriptorSets.resize(ImageCont);

		Invalidate();
	}

	/*Material::Material(std::shared_ptr<Material> CopyMaterial)
	{

	}*/

	Material::~Material()
	{
		vkDestroyDescriptorSetLayout(RendererContext::GetDevice()->GetLogicalDevice(), m_DescriptorSetLayout, nullptr);
	}

	void Material::Invalidate()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = m_VertexShader->GetDescriptorBindings();
		for (const auto& Bindings : m_FragmentShader->GetDescriptorBindings())
		{
			setLayoutBindings.push_back(Bindings);
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
		descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutInfo.bindingCount = setLayoutBindings.size();
		descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

		if (auto Result = vkCreateDescriptorSetLayout(RendererContext::GetDevice()->GetLogicalDevice(), &descriptorSetLayoutInfo, nullptr, &m_DescriptorSetLayout); Result != VK_SUCCESS)
		{
			TRE_CORE_ERROR("Unable to create pipeline descriptor set layout");
			assert(Result == VK_SUCCESS);
		}
	}

	void Material::AllocateLayouts()
	{
		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			Engine::GetInstance().GetRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_DescriptorSetLayout, m_DescriptorSets[x]);
		}
	}

	void Material::UpdateForRendering(const std::shared_ptr<UniformBuffer>& UBO, uint32_t Index)
	{
		m_WriteDescriptors.clear();

		for (auto x : m_VertexShader->GetWriteDescriptorSets())
		{
			x.second.pBufferInfo = &UBO->GetDescriptorBufferInfo();
			x.second.dstSet = m_DescriptorSets[Index];
			m_WriteDescriptors.push_back(x.second);
		}
		
		int x = 0;
		if (m_FragmentShader->GetWriteDescriptorSets().size() != 0)
		{
			for (auto FragmentBindings : m_FragmentShader->GetWriteDescriptorSets())
			{
				if (FragmentBindings.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
				{
					FragmentBindings.second.pImageInfo = &m_Textures[x]->GetDescriptorImageInfo();
					FragmentBindings.second.dstSet = m_DescriptorSets[Index];
					m_WriteDescriptors.push_back(FragmentBindings.second);
					++x;
				}
			}
		}

		vkUpdateDescriptorSets(RendererContext::GetDevice()->GetLogicalDevice(), static_cast<uint32_t>(m_WriteDescriptors.size()), m_WriteDescriptors.data(), 0, nullptr);
	}

	void Material::Serialize()
	{
		std::string path = "../Assets/";
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

		file << "VertexShader:\n" << m_VertexShader->GetHandleHex() << std::endl;
		file << "FragmentShader:\n" << m_FragmentShader->GetHandleHex() << std::endl;
		file << "Textures:\n";
		//For loop next time
		for (auto texture : m_Textures)
		{
			file << texture->GetHandleHex() << std::endl;
		}

		file.close();
	}

	std::shared_ptr<Material> Material::Deserialize(const std::string& assetHexGUID)
	{
		//Open material file
		std::string materialPath = "../Assets/" + assetHexGUID + ".material";
		std::ifstream file(materialPath);
		if (!file.is_open())
		{
			TRE_CORE_ERROR("Unable to open file {0}", materialPath);
			return nullptr;
		}

		std::string line;
		std::string vertexShaderGUID;
		std::string fragmentShaderGUID;
		std::vector<std::string> textureGUIDs;

		while (std::getline(file, line))
		{
			if (line == "VertexShader:")
			{
				std::getline(file, vertexShaderGUID);
			}
			else if (line == "FragmentShader:")
			{
				std::getline(file, fragmentShaderGUID);
			}
			else if (line == "Textures:")
			{
				while (std::getline(file, line))
				{
					textureGUIDs.push_back(line);
				}
			}
		}

		auto vertShader = ResourceManager::Instance().GetResource<Shader>(Resource::GetGUIDFromHex(vertexShaderGUID));
		auto fragShader = ResourceManager::Instance().GetResource<Shader>(Resource::GetGUIDFromHex(fragmentShaderGUID));
		std::unique_ptr<Material> mat = std::make_unique<Material>(vertShader, fragShader);
		ResourceHandle assetHandle = Resource::GetGUIDFromHex(assetHexGUID);
		mat->m_Handle = assetHandle;

		mat->m_Textures.resize(textureGUIDs.size());
		for (int i = 0; i < textureGUIDs.size(); ++i)
		{	
			std::string textureHexGUID = textureGUIDs[i];
			auto texture = ResourceManager::Instance().GetResource<VulkanTexture>(Resource::GetGUIDFromHex(textureHexGUID));
			//Load into engine if not in asset manager
			if (texture == nullptr)
			{
				texture = VulkanTexture::Deserialize(textureHexGUID);
			}
			mat->m_Textures[i] = texture;
		}

		ResourceManager::Instance().AddResource(std::move(mat));

		return std::move(ResourceManager::Instance().GetResource<Material>(assetHandle));
	}
}