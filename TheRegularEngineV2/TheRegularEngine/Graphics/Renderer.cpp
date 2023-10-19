#include "pch.h"
#include "Renderer.h"
#include "Core/Engine.h"
#include "Buffer.h"
#include "VulkanUtilities.h"
#include "Pipeline.h"
#include "Resource/ResourceManager.h"
#include "Material.h"

namespace TRE
{
	struct FinalRenderData
	{
		std::unique_ptr<Buffer> VertexBuffer;
		std::unique_ptr<Buffer> IndexBuffer;
		std::unique_ptr<Pipeline> Pipeline;
		std::unique_ptr<Material> Material;
		VkDescriptorImageInfo ImageInfo;
		VkSampler Sampler;
	};

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	std::shared_ptr<SceneRenderer> Renderer::s_MainRenderer = nullptr;
	static std::unique_ptr<FinalRenderData> s_FinalRenderData = std::make_unique<FinalRenderData>();

	static std::unique_ptr<Buffer> CreateVertexBuffer(const std::vector<QuadVertex>& vertices)
	{
		uint32_t m_VertexCount = static_cast<std::uint32_t>(vertices.size());
		assert(m_VertexCount >= 3 && "Vertex count must be at least 3");

		uint32_t vertexSize = sizeof(vertices[0]);
		Buffer stagingBuffer(vertexSize, m_VertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//Create a staging buffer to copy the vertex data to
		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)vertices.data());

		//Flush data from staging buffer to vertex buffer
		std::unique_ptr<Buffer> Vbuffer = std::make_unique<Buffer>(vertexSize, m_VertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = vertexSize * m_VertexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), Vbuffer->GetBuffer(), bufferSize);

		return Vbuffer;
	}

	static std::unique_ptr<Buffer> CreateIndexBuffer(const std::vector<int>& indices)
	{
		uint32_t m_IndexCount = static_cast<std::uint32_t>(indices.size());

		uint32_t indexSize = sizeof(indices[0]);
		Buffer stagingBuffer(indexSize, m_IndexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)indices.data());

		std::unique_ptr<Buffer> m_IndexBuffer = std::make_unique<Buffer>(indexSize, m_IndexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkDeviceSize bufferSize = indexSize * m_IndexCount;
		vkUtils::CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);

		return m_IndexBuffer;
	}

	void Renderer::Init()
	{
		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();
		float x = -1;
		float y = -1;
		float width = 2, height = 2;

		std::vector<QuadVertex> data(4);

		data[0].Position = glm::vec3(x, y, 0.0f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		data[3].TexCoord = glm::vec2(0, 1);

		if (!Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			s_FinalRenderData->VertexBuffer = CreateVertexBuffer(data);
			std::vector<int> indices = { 0, 1, 2, 2, 3, 0, };
			s_FinalRenderData->IndexBuffer = CreateIndexBuffer(indices);

			PipelineConfigurations PipelineConfig;
			PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(4);
			PipelineConfig.Primitive = PrimitiveType::Triangles;
			PipelineConfig.VertexStride = PipelineConfig.Shader->GetVertexStrides();
			s_FinalRenderData->Pipeline = std::make_unique<Pipeline>(PipelineConfig, SwapChain->GetRenderPassPointer());
			s_FinalRenderData->Material = std::make_unique<Material>(PipelineConfig.Shader);
			s_FinalRenderData->Material->Invalidate();
			
			VkSamplerCreateInfo samplerCreateInfo = {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.maxAnisotropy = 1.0f;
			samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
			samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 100.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			vkCreateSampler(Device, &samplerCreateInfo, nullptr, &s_FinalRenderData->Sampler);

		}
	}

	void Renderer::SetMainRenderer(const std::shared_ptr<SceneRenderer>& SceneRenderer)
	{
		s_MainRenderer = SceneRenderer;
	}
		
	const std::shared_ptr<SceneRenderer>& Renderer::GetMainRenderer()
	{
		return s_MainRenderer;
	}

	void Renderer::RenderToSwapChain()
	{
		auto swapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetWidth();
		uint32_t height = swapChain->GetHeight();

		VkCommandBuffer TargetCommandBuffer = swapChain->GetCurrentCommandBuffer();

		VkCommandBufferBeginInfo TargetCBInfo = {};
		TargetCBInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		TargetCBInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (auto Result = vkBeginCommandBuffer(TargetCommandBuffer, &TargetCBInfo); Result != VK_SUCCESS)
		{
			assert(Result == VK_SUCCESS && "Failed to begin target command buffer");
		}

		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = swapChain->GetRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();
		renderPassBeginInfo.framebuffer = swapChain->GetCurrentFrameBuffer();

		vkCmdBeginRenderPass(TargetCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(TargetCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(TargetCommandBuffer, 0, 1, &scissor);

		s_FinalRenderData->ImageInfo.imageView = s_MainRenderer->GetColorImages()[swapChain->GetCurrentBufferIndex()]->GetImageView();
		s_FinalRenderData->ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		s_FinalRenderData->ImageInfo.sampler = s_FinalRenderData->Sampler;

		vkCmdBindPipeline(TargetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_FinalRenderData->Pipeline->GetPipeline());
		s_FinalRenderData->Material->UpdateCompsitePass(s_FinalRenderData->ImageInfo);
		vkCmdBindDescriptorSets(TargetCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, s_FinalRenderData->Pipeline->GetPipelineLayout(), 0, 
			1, &s_FinalRenderData->Material->GetDescriptor(swapChain->GetCurrentBufferIndex()), 0, NULL);
		VkDeviceSize offsets[] = { 0 };
		auto Vbuffer = s_FinalRenderData->VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(TargetCommandBuffer, 0, 1, &Vbuffer, offsets);
		auto Ibuffer = s_FinalRenderData->IndexBuffer->GetBuffer();
		vkCmdBindIndexBuffer(TargetCommandBuffer, Ibuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(TargetCommandBuffer, 6, 1, 0, 0, 0);

		vkCmdEndRenderPass(TargetCommandBuffer);
		vkEndCommandBuffer(TargetCommandBuffer);
	}
}