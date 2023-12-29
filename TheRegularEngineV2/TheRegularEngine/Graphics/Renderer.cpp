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
	std::shared_ptr<SceneRenderer> Renderer::s_MainRenderer = nullptr;
	std::shared_ptr<CommandBuffer> Renderer::m_CommandBuffer = nullptr;
	FinalRenderData* Renderer::s_FinalRenderData = nullptr;

	std::unique_ptr<Buffer> CreateVertexBuffer(const std::vector<QuadVertex>& vertices)
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

	std::unique_ptr<Buffer> CreateIndexBuffer(const std::vector<int>& indices)
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
			s_FinalRenderData->VertexBuffer = std::make_unique<VertexBuffer>(static_cast<void*>(data.data()),
				UINT32_T_CAST(sizeof(QuadVertex) * data.size()));

			std::vector<int> indices = { 0,1,2,2,3,0 };
			s_FinalRenderData->IndexBuffer = std::make_unique<IndexBuffer>(static_cast<void*>(indices.data()),
				UINT32_T_CAST(sizeof(int) * indices.size()),
				UINT32_T_CAST(indices.size()));

			s_FinalRenderData->RenderPass = SwapChain->GetRenderPass();

			PipelineConfigurations PipelineConfig;
			PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(4);
			PipelineConfig.Primitive = PrimitiveType::Triangles;
			PipelineConfig.VertexStride = PipelineConfig.Shader->GetVertexStrides();
			s_FinalRenderData->Pipeline = std::make_shared<Pipeline>(PipelineConfig, s_FinalRenderData->RenderPass);
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
		s_FinalRenderData = nullptr;
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
		viewport.y = (float)height;
		viewport.width = (float)width;
		viewport.height = -(float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &viewport);

		VkRect2D scissor {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &scissor);

		Renderer::BindPipeline(m_CommandBuffer, s_FinalRenderData->Pipeline);

		s_FinalRenderData->Material->UpdateCompsitePass(Engine::GetInstance().GetMainSceneRenderer()->GetColorImages()[swapChain->GetCurrentImageIndex()]->GetDescriptorImageInfo());
		vkCmdBindDescriptorSets(m_CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, s_FinalRenderData->Pipeline->GetPipelineLayout(),
			0, 1, &s_FinalRenderData->Material->GetDescriptor(swapChain->GetCurrentBufferIndex()), 0, NULL);

		VkDeviceSize offsets[] = { 0 };
		auto Vbuffer = s_FinalRenderData->VertexBuffer->GetBuffer();
		vkCmdBindVertexBuffers(m_CommandBuffer->GetInUseCommandBuffer(), 0, 1, &Vbuffer, offsets);
		auto Ibuffer = s_FinalRenderData->IndexBuffer->GetBuffer();
		vkCmdBindIndexBuffer(m_CommandBuffer->GetInUseCommandBuffer(), Ibuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(m_CommandBuffer->GetInUseCommandBuffer(), 6, 1, 0, 0, 0);

		EndRenderPass(m_CommandBuffer);
		m_CommandBuffer->End();
	}

	//To be implemented after framebuffer/renderpass abstraction
	void Renderer::BeginRenderPass(const std::shared_ptr<CommandBuffer>& CommandBuffer, const std::shared_ptr<RenderPass>& Renderpass)
	{
		(void)CommandBuffer;
		(void)Renderpass;
	}

	void Renderer::EndRenderPass(const std::shared_ptr<CommandBuffer>& CommandBuffer)
	{
		vkCmdEndRenderPass(CommandBuffer->GetInUseCommandBuffer());
	}

	void Renderer::BeginFrame()
	{
		Engine::GetInstance().GetMainSceneRenderer()->BeginFrame();

		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			Engine::GetInstance().GetEditorSceneRenderer()->BeginEditorFrame();
		}
	}
	
	void Renderer::EndFrame()
	{
		Engine::GetInstance().GetMainSceneRenderer()->EndFrame();

		if (Engine::GetInstance().GetEngineInfo().EnableEditor)
		{
			Engine::GetInstance().GetEditorSceneRenderer()->EndFrame();
		}
	}

	void Renderer::BindPipeline(const std::shared_ptr<CommandBuffer>& CommandBuffer, const std::shared_ptr<Pipeline>& Pipeline, bool IsCompute)
	{
		if (!IsCompute)
			vkCmdBindPipeline(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline->GetPipeline());
		else
			vkCmdBindPipeline(CommandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline->GetPipeline());
	}
}