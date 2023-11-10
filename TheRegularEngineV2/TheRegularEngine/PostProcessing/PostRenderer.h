#pragma once
#include "Graphics/Device.h"
#include "Graphics/Pipeline.h"
#include "Graphics/RenderPass.h"
#include "Graphics/CommandBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/Material.h"
#include "Graphics/UniformBuffer.h"

namespace TRE
{
	struct PostVertex
	{
		glm::vec2 Position;
		glm::vec2 UV;
	};

	class PostProcessEffect
	{
	public:
		PostProcessEffect(const std::shared_ptr<Device>& device);
		virtual ~PostProcessEffect() {};

		void SetupName(const std::string& name) { m_Name = name; }
		virtual void SetupUBO() = 0;
		virtual void SetupShader(std::shared_ptr<Shader> shader) = 0;
		virtual void UpdateUBO() {};
		virtual void Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index);
	protected:
		std::shared_ptr<Pipeline> m_Pipeline;
		std::shared_ptr<RenderPass> m_Renderpass;
		std::shared_ptr<VertexBuffer> m_VertexBuffer;
		std::shared_ptr<IndexBuffer> m_IndexBuffer;

		std::shared_ptr<Material> m_Material;
		std::shared_ptr<UniformBuffer> m_UBO;
		std::string m_Name;
	};

	class PostProcessingRenderer
	{
	public:
		PostProcessingRenderer(const std::shared_ptr<Device>& device);
		~PostProcessingRenderer();

		void Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index);

		void AddPostEffect(const std::shared_ptr<PostProcessEffect>& effect, const int index, const std::string name);
	private:
		
		std::map<int, std::shared_ptr<PostProcessEffect>> m_PostEffects;
	};
}