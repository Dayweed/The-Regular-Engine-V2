#include "pch.h"
#include "ParticleRenderer.h"
#include "Core/Engine.h"
#include "Core/ECS.h"
#include "ECS/Components/Particle2DComponent.h"
#include "Camera.h"
#include "EditorCamera.h"
#include "Core/ECS.h"

namespace TRE
{
	ParticleRenderer::ParticleRenderer(const std::shared_ptr<Device>& device)
	{
		auto SC = Engine::GetInstance().GetWindow()->GetSwapChain();
		RenderPassInfo RPConfig{};
		RPConfig.FinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		RPConfig.ImageFormat = SC->GetColorFormat();
		RPConfig.DepthFinalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		RPConfig.DepthImageFormat = SC->GetDepthFormat();
		RPConfig.DepthEnabled = true;
		RPConfig.ClearColor = false;
		m_Renderpass = std::make_shared<RenderPass>(device, RPConfig);

		PipelineConfigurations PipelineConfig{};
		PipelineConfig.Primitive = PrimitiveType::Triangles;
		PipelineConfig.Shader = ResourceManager::Instance().GetResource<Shader>(12);
		PipelineConfig.CullMode = VK_CULL_MODE_NONE;
		PipelineConfig.EnableBlending = true;
		PipelineConfig.EnableDepthTest = false;
		m_Pipeline = std::make_shared<Pipeline>(PipelineConfig, m_Renderpass);

		float x = -0.5f; float y = -0.5f;
		float width = 1, height = 1;
		std::vector<QuadVertex> data(4);

		data[0].Position = glm::vec3(x, y, 0.0f);
		//data[0].TexCoord = glm::vec2(0, 0);
		data[0].TexCoord = glm::vec2(0, 1);

		data[1].Position = glm::vec3(x + width, y, 0.0f);
		//data[1].TexCoord = glm::vec2(1, 0);
		data[1].TexCoord = glm::vec2(1, 1);

		data[2].Position = glm::vec3(x + width, y + height, 0.0f);
		//data[2].TexCoord = glm::vec2(1, 1);
		data[2].TexCoord = glm::vec2(1, 0);

		data[3].Position = glm::vec3(x, y + height, 0.0f);
		//data[3].TexCoord = glm::vec2(0, 1);
		data[3].TexCoord = glm::vec2(0, 0);

		std::vector<int> indices = { 0,1,2,2,3,0 };

		m_IndexBuffer = std::make_shared<IndexBuffer>(static_cast<void*>(indices.data()),
			UINT32_T_CAST(sizeof(int) * indices.size()),
			UINT32_T_CAST(indices.size()));

		m_VertexBuffer = std::make_shared<VertexBuffer>(static_cast<void*>(data.data()),
			UINT32_T_CAST(data.size() * sizeof(QuadVertex)));
	
		m_DefaultMaterial = std::make_shared<Material>(ResourceManager::Instance().GetResource<Shader>(12));
		m_DefaultMaterial->Invalidate();
		m_DefaultMaterial->SetTexture("DiffuseMap", VulkanTexture::GetDefaultTexture());
	}

	void ParticleRenderer::Render(std::shared_ptr<UniformBuffer> ubo2D, std::shared_ptr<UniformBuffer> ubo3D, const std::shared_ptr<CommandBuffer>& commandBuffer, bool isEditor)
	{
		std::multimap<ResourceHandle, Entity> sortedParticles;
		for (const auto& emitter : ECSManager::Instance().GetEntities<Particle2DComponent>())
		{
			const Particle2DComponent& particleComp = emitter->GetComponent<Particle2DComponent>();
			if (particleComp.m_Material)
			{
				particleComp.m_Material->SetUBOData(particleComp.m_Color);
				particleComp.m_Material->SetMaterialUBO();

				if (particleComp.m_Texture)
					particleComp.m_Material->SetTexture("DiffuseMap", particleComp.m_Texture);
				sortedParticles.insert(std::make_pair(particleComp.m_Material->GetHandle(), emitter));
			}
			else
				sortedParticles.insert(std::make_pair(m_DefaultMaterial->GetHandle(), emitter));

		}

		auto index = Engine::GetInstance().GetWindow()->GetSwapChain()->GetCurrentBufferIndex();
		Renderer::BindPipeline(commandBuffer, m_Pipeline);
		for (const auto& ent: sortedParticles)
		{
			const ResourceHandle currentHandle = ent.first;
			const Particle2DComponent& particleComp = ent.second->GetComponent<Particle2DComponent>();
			if (particleComp.m_Running)
			{
				if (currentHandle != m_PreviousMaterialHandle)
				{
					std::shared_ptr<UniformBuffer> ubo = particleComp.m_3DWorld ? ubo3D : ubo2D;
					if (particleComp.m_Material)
					{
						if (isEditor)
						{
							particleComp.m_Material->UpdateForEditorSceneRendering(ubo, index);
							vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &particleComp.m_Material->GetEditorDescriptor(index), 0, NULL);
						}
						else
						{
							particleComp.m_Material->UpdateForRendering(ubo, index);
							vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &particleComp.m_Material->GetDescriptor(index), 0, NULL);
						}
					}
					else
					{
						if (isEditor)
						{
							m_DefaultMaterial->UpdateForEditorSceneRendering(ubo, index);
							vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultMaterial->GetEditorDescriptor(index), 0, NULL);
						}
						else
						{
							m_DefaultMaterial->UpdateForRendering(ubo, index);
							vkCmdBindDescriptorSets(commandBuffer->GetInUseCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline->GetPipelineLayout(), 0, 1, &m_DefaultMaterial->GetDescriptor(index), 0, NULL);
						}
					}
				}
				
				for (auto& particle : particleComp.m_Particles)
				{
					Particle_PushConstant pc{};
					pc.L2W = particle.L2W;

					vkCmdPushConstants(commandBuffer->GetInUseCommandBuffer(), m_Pipeline->GetPipelineLayout(),
						VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(Particle_PushConstant), &pc);

					VkDeviceSize offsets[] = { 0 };
					VkBuffer VB = m_VertexBuffer->GetBuffer();

					vkCmdBindVertexBuffers(commandBuffer->GetInUseCommandBuffer(), 0, 1, &VB, offsets);
					vkCmdBindIndexBuffer(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

					vkCmdDrawIndexed(commandBuffer->GetInUseCommandBuffer(), m_IndexBuffer->GetIndexCount(), 1, 0, 0, 0);
				}

				m_PreviousMaterialHandle = currentHandle;
			}
		}
		m_PreviousMaterialHandle = 0;
	}
}