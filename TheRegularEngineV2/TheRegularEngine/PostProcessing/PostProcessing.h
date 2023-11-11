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
	};

	class PostProcessingManager
	{
	public:
		static PostProcessingManager& Instance()
		{
			static PostProcessingManager instance;
			return instance;
		}
		void Init();
		void Render(VkFramebuffer targetFramebuffer, const std::shared_ptr<CommandBuffer>& commandBuffer, const int index);
		void Shutdown();

		void AddPostEffect(std::unique_ptr<PostProcessEffect> effect, const int index, const std::string name);
		void RemovePostEffect(const std::string& name);
		
		template<typename T>
		T& GetPostEffect(const std::string& name);
	private:
		PostProcessingManager() {};
		PostProcessingManager(PostProcessingManager const&) = delete;
		void operator=(PostProcessingManager const&) = delete;
		void* operator new(size_t) = delete;
	private:
		std::map<std::string, std::pair<int, std::unique_ptr<PostProcessEffect>>> m_PostEffects;
	};

	template<typename T>
	inline T& PostProcessingManager::GetPostEffect(const std::string& name)
	{
		if(m_PostEffects.find(name) != m_PostEffects.end())
			return *static_cast<T*>(m_PostEffects[name].second.get());
		//return T();
	}
}