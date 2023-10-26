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
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec2 TexCoord;
	};

	std::shared_ptr<SceneRenderer> Renderer::s_MainRenderer = nullptr;
	std::shared_ptr<CommandBuffer> Renderer::m_CommandBuffer = nullptr;
	FinalRenderData* Renderer::s_FinalRenderData = nullptr;

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
		s_FinalRenderData = new FinalRenderData;

		auto SwapChain = Engine::GetInstance().GetWindow()->GetSwapChain();
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();
		float x = -1; float y = -1;
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
			s_FinalRenderData->RenderPass = SwapChain->GetRenderPassPointer();

			PipelineConfigurations PipelineConfig;
			PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(4);
			PipelineConfig.Primitive = PrimitiveType::Triangles;
			PipelineConfig.VertexStride = PipelineConfig.Shader->GetVertexStrides();
			s_FinalRenderData->Pipeline = std::make_unique<Pipeline>(PipelineConfig, s_FinalRenderData->RenderPass);
			s_FinalRenderData->Material = std::make_unique<Material>(PipelineConfig.Shader);
			s_FinalRenderData->Material->Invalidate();
		}

		m_CommandBuffer = std::make_shared<CommandBuffer>("Final Pass", true);
	}

	void Renderer::Shutdown()
	{
		auto Device = RendererContext::GetDevice()->GetLogicalDevice();
		vkDeviceWaitIdle(Device);
		delete s_FinalRenderData;
		m_CommandBuffer = nullptr;
	}

	void Renderer::RenderToSwapChain()
	{
		auto& swapChain = Engine::GetInstance().GetWindow()->GetSwapChain();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = swapChain->GetWidth();
		uint32_t height = swapChain->GetHeight();

		m_CommandBuffer->Begin();

		s_FinalRenderData->RenderPass->BeginRenderPass(m_CommandBuffer->GetInUseCommandBuffer(), swapChain->GetCurrentFrameBuffer());

		VkViewport viewport {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.height = (float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		s_FinalRenderData->ImageInfo = Engine::GetInstance().GetMainSceneRenderer()->GetColorImages()[swapChain->GetCurrentImageIndex()]->GetDescriptorImageInfo();
		
		vkCmdBindPipeline(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, s_FinalRenderData->Pipeline->GetPipeline());

		s_FinalRenderData->Material->UpdateCompsitePass(s_FinalRenderData->ImageInfo);
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, s_FinalRenderData->Pipeline->GetPipelineLayout(),
			0, 1, &s_FinalRenderData->Material->GetDescriptor(swapChain->GetCurrentBufferIndex()), 0, NULL);

		VkDeviceSize offsets[] = { 0 };
		auto Vbuffer = s_FinalRenderData->VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &Vbuffer, offsets);
		auto Ibuffer = s_FinalRenderData->IndexBuffer->GetBuffer();
		vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), Ibuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), 6, 1, 0, 0, 0);

		s_FinalRenderData->RenderPass->EndRenderPass(m_CommandBuffer->GetInUseCommandBuffer());
		vkEndCommandBuffer(m_CommandBuffer->GetInUseCommandBuffer());
	}
}