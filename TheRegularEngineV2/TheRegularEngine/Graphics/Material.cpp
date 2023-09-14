#include "pch.h"
#include "Material.h"
#include "Core/Engine.h"
#include "Core/Logger.h"

namespace TRE
{
	const VkDescriptorSet& Material::GetDescriptor(uint32_t FrameIndex)
	{
		return m_DescriptorSets[FrameIndex];
	}

	Material::Material(const std::shared_ptr<Shader>& VertexShader, const std::shared_ptr<Shader>& FragShader) : m_VertexShader(VertexShader), m_FragmentShader(FragShader)
	{
		auto ImageCont = Engine::GetInstance().GetWindow()->GetSwapChain()->GetImageCount();
		m_DescriptorSets.resize(ImageCont);

		Invalidate();
	}

	Material::Material(std::shared_ptr<Material> CopyMaterial)
	{

	}

	Material::~Material()
	{

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

		for (int x = 0; x < m_DescriptorSets.size(); x++)
		{
			Engine::GetInstance().GetRenderer()->GetDescriptorPool()->AllocateDescriptorSet(m_DescriptorSetLayout, m_DescriptorSets[x]);
		}
	}

	void Material::UpdateForRendering(std::vector<VkWriteDescriptorSet> DescriptorWrites)
	{

	}
}