#pragma once
#include "Buffer.h"
#include "Descriptor.h"
#include "RenderPass.h"
#include "UniformBuffer.h"
#include "Shader.h"
#include "VulkanUtilities.h"

namespace TRE
{
	enum class VertexInputDataType
	{
		None = 0, Float, Vec2, Vec3, Vec4, Int, IVec2, IVec3, IVec4, Bool, Mat3, Mat4
	};
	
	struct VertexBufferInput
	{
		VertexBufferInput(VertexInputDataType InputType) : Type(InputType)
		{
			Size = VertexDataTypeSize(Type);
		}

		static uint32_t VertexDataTypeSize(VertexInputDataType type);

		VertexInputDataType Type;
		uint32_t Size;
		uint32_t Offset;
	};

	class VertexBufferInputLayout //1 for each binding point
	{
		public:
			VertexBufferInputLayout(const std::initializer_list<VertexBufferInput>& Inputs) : m_Inputs(Inputs)
			{
				uint32_t offset = 0;
				VertexStride = 0;
				for (auto& Input : m_Inputs)
				{
					Input.Offset = offset;
					offset += Input.Size;
					VertexStride += Input.Size;
				}
			}

			[[nodiscard]] std::vector<VertexBufferInput>::iterator begin() { return m_Inputs.begin(); }
			[[nodiscard]] std::vector<VertexBufferInput>::iterator end() { return m_Inputs.end(); }
			[[nodiscard]] std::vector<VertexBufferInput>::const_iterator begin() const { return m_Inputs.begin(); }
			[[nodiscard]] std::vector<VertexBufferInput>::const_iterator end() const { return m_Inputs.end(); }

		std::vector<VertexBufferInput> m_Inputs;
		uint32_t BindPoint = 0;
		uint32_t VertexStride = 0;
	};


	enum class PrimitiveType //Although vk has more, the other 6 would not be used.
	{
		Point,
		Lines,
		LinesStrip,
		Triangles,
		TrianglesStrip,
		TranglesFan
	};

	struct PipelineConfigurations
	{
		std::shared_ptr<Shader> Shader;
		PrimitiveType Primitive;
		float LineWidth = 1.f;
		size_t VertexStride = 0;
		bool EnableCull = true;
		bool EnableBlending = false;
		bool EnableDepthTest = true;

		bool UseAutoShaderVertexInput = true; //This should be set to false if you have more than 1 bind points for vertex inputs
		std::vector<VertexBufferInputLayout> CustomVertexBufferInputLayout;
	};

	class Pipeline
	{
		public:
			Pipeline(const PipelineConfigurations& PipelineConfig, const std::shared_ptr<RenderPass>& TargetRenderPass);
			~Pipeline();

			VkFormat VertexDataTypeToVulkanFormat(VertexInputDataType type);

		public:
			VkPrimitiveTopology GetVulkanTopology(PrimitiveType TopologyType);
			VkPipelineLayout GetPipelineLayout();
			VkPipeline GetPipeline();
			PipelineConfigurations& GetConfig();

		private:
			VkPipeline m_Pipeline;
			VkPipelineLayout m_Layout;
			PipelineConfigurations m_Config;
	};
}