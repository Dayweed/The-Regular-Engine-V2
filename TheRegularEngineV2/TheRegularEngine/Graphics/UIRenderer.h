#pragma once
#include "pch.h"
#include "Device.h"
#include "UniformBuffer.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Material.h"
#include "CommandBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace TRE
{
	class Ent;
	typedef std::shared_ptr<Ent> Entity;

	struct UIVertex
	{
		glm::vec3 Position;
		glm::vec2 UV;
	};

	struct UIUBO
	{
		glm::mat4 m_ProjView2DSpace;
	};

	struct UI_PushConstant
	{
		glm::mat4 L2W;
		glm::vec4 Color;
	};

	class UIRenderer
	{
		public:
			UIRenderer(const std::shared_ptr<Device>& Device);
			~UIRenderer();

			void Render(VkFramebuffer TargetFramebuffer, const std::shared_ptr<CommandBuffer>& CommandBuffer, bool IsEditor);

		private:
			std::shared_ptr<Device> m_Device;
			std::shared_ptr<Pipeline> m_UIPipeline;
			std::shared_ptr<RenderPass> m_UIRenderpass;
			std::vector<std::shared_ptr<Material>> m_UIMaterial;
			std::shared_ptr<UniformBuffer> m_UIUBO;

		private:
			std::shared_ptr<VertexBuffer> m_TestVertexBuffer;
			std::shared_ptr<IndexBuffer> m_TestIndexBuffer;
			std::vector<std::pair<int, Entity>> m_UIEntities;
	};
}