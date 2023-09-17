#include "pch.h"
#include "Material.h"
#include "Core/Engine.h"
#include "Core/Logger.h"
#include "Assets/AssetManager.h"

namespace TRE
{
	const VkDescriptorSet& Material::GetDescriptor(uint32_t FrameIndex)
	{
		return m_DescriptorSets[FrameIndex];
	}

	Material::Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader) : m_VertexShader(VertexShader), m_FragmentShader(FragShader)
	{
		m_Type = AssetType::Material;
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
		for (auto FragmentBindings : m_FragmentShader->GetWriteDescriptorSets())
		{
			if (FragmentBindings.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				auto imageInfo = m_Textures[x]->GetDescriptorImageInfo();
				FragmentBindings.second.pImageInfo = &imageInfo;
				FragmentBindings.second.dstSet = m_DescriptorSets[Index];
				m_WriteDescriptors.push_back(FragmentBindings.second);
				++x;
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

		file << "VertexShader: " << m_VertexShader->GetHandle() << std::endl;
		file << "FragmentShader: " << m_FragmentShader->GetHandle() << std::endl;
		//file << "Textures: " << m_Textures->GetHandle() << std::endl;

		file.close();
	}
}