#include "pch.h"
#include "ShaderCompiler.h"
#include "shaderc/shaderc.hpp"
#include "Core/Logger.h"
#include "Shader.h"
#include "spirv_cross/spirv_glsl.hpp"

namespace TRE
{
	static shaderc_shader_kind VulkanStageToShaderC(const VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
			case VK_SHADER_STAGE_VERTEX_BIT:    return shaderc_vertex_shader;
			case VK_SHADER_STAGE_FRAGMENT_BIT:  return shaderc_fragment_shader;
			case VK_SHADER_STAGE_COMPUTE_BIT:   return shaderc_compute_shader;
		}
		assert(false);
		return {};
	}

	static uint32_t GetStrideFromVulkanFormat(VkFormat format)
	{
		switch (format)
		{
			case VK_FORMAT_R8_SINT:
				return sizeof(int);
			case VK_FORMAT_R32_SFLOAT:
				return sizeof(float);
			case VK_FORMAT_R32G32_SFLOAT:
				return sizeof(glm::vec2);
			case VK_FORMAT_R32G32B32_SFLOAT:
				return sizeof(glm::vec3);
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return sizeof(glm::vec4);
			case VK_FORMAT_R32G32_SINT:
				return sizeof(glm::ivec2);
			case VK_FORMAT_R32G32B32_SINT:
				return sizeof(glm::ivec3);
			case VK_FORMAT_R32G32B32A32_SINT:
				return sizeof(glm::ivec4);
			case VK_FORMAT_R32G32_UINT:
				return sizeof(glm::ivec2);
			case VK_FORMAT_R32G32B32_UINT:
				return sizeof(glm::ivec3);
			case VK_FORMAT_R32G32B32A32_UINT:
				return sizeof(glm::ivec4); // Need uintvec?
			default:
				TRE_CORE_WARN("Unsupported Format {0}", (int)format);
				return 0;
		}

		return 0;
	}

	static VkFormat GetVulkanFormat(const spirv_cross::SPIRType& type)
	{
		using namespace spirv_cross;
		if (type.basetype == SPIRType::Struct || type.basetype == SPIRType::Sampler)
		{
			TRE_CORE_WARN("Tried to convert a structure or SPIR sampler into a VkFormat enum value!");
			return VK_FORMAT_UNDEFINED;
		}
		else if (type.basetype == SPIRType::Image || type.basetype == SPIRType::SampledImage)
		{
			switch (type.image.format)
			{
				case spv::ImageFormatR8:
					return VK_FORMAT_R8_UNORM;
				case spv::ImageFormatR8Snorm:
					return VK_FORMAT_R8_SNORM;
				case spv::ImageFormatR8ui:
					return VK_FORMAT_R8_UINT;
				case spv::ImageFormatR8i:
					return VK_FORMAT_R8_SINT;
				case spv::ImageFormatRg8:
					return VK_FORMAT_R8G8_UNORM;
				case spv::ImageFormatRg8Snorm:
					return VK_FORMAT_R8G8_SNORM;
				case spv::ImageFormatRg8ui:
					return VK_FORMAT_R8G8_UINT;
				case spv::ImageFormatRg8i:
					return VK_FORMAT_R8G8_SINT;
				case spv::ImageFormatRgba8i:
					return VK_FORMAT_R8G8B8A8_SINT;
				case spv::ImageFormatRgba8ui:
					return VK_FORMAT_R8G8B8A8_UINT;
				case spv::ImageFormatRgba8:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case spv::ImageFormatRgba8Snorm:
					return VK_FORMAT_R8G8B8A8_SNORM;
				case spv::ImageFormatR32i:
					return VK_FORMAT_R32_SINT;
				case spv::ImageFormatR32ui:
					return VK_FORMAT_R32_UINT;
				case spv::ImageFormatRg32i:
					return VK_FORMAT_R32G32_SINT;
				case spv::ImageFormatRg32ui:
					return VK_FORMAT_R32G32_UINT;
				case spv::ImageFormatRgba32f:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case spv::ImageFormatRgba16f:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case spv::ImageFormatR32f:
					return VK_FORMAT_R32_SFLOAT;
				case spv::ImageFormatRg32f:
					return VK_FORMAT_R32G32_SFLOAT;
				case spv::ImageFormatR16f:
					return VK_FORMAT_R16_SFLOAT;
				case spv::ImageFormatRgba32i:
					return VK_FORMAT_R32G32B32A32_SINT;
				case spv::ImageFormatRgba32ui:
					return VK_FORMAT_R32G32B32A32_UINT;
				default:
					TRE_CORE_WARN("Failed to convert an image format to a VkFormat enum.");
					return VK_FORMAT_UNDEFINED;
			}
		}
		else if (type.vecsize == 1) //type.width = number of bytes
		{
			if (type.width == 8)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R8_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R8_UINT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 16)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R16_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R16_UINT;
					case SPIRType::Float:
						return VK_FORMAT_R16_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 32)
			{
				switch (type.basetype)
				{
				case SPIRType::Int:
					return VK_FORMAT_R32_SINT;
				case SPIRType::UInt:
					return VK_FORMAT_R32_UINT;
				case SPIRType::Float:
					return VK_FORMAT_R32_SFLOAT;
				default:
					return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 64)
			{
				switch (type.basetype)
				{
				case SPIRType::Int64:
					return VK_FORMAT_R64_SINT;
				case SPIRType::UInt64:
					return VK_FORMAT_R64_UINT;
				case SPIRType::Double:
					return VK_FORMAT_R64_SFLOAT;
				default:
					return VK_FORMAT_UNDEFINED;
				}
			}
			else
			{
				TRE_CORE_WARN("Invalid type width for conversion of SPIR-Type to VkFormat enum value!");
				return VK_FORMAT_UNDEFINED;
			}
		}
		else if (type.vecsize == 2)
		{
			if (type.width == 8)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R8G8_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R8G8_UINT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 16)
			{
				switch (type.basetype)
				{
				case SPIRType::Int:
					return VK_FORMAT_R16G16_SINT;
				case SPIRType::UInt:
					return VK_FORMAT_R16G16_UINT;
				case SPIRType::Float:
					return VK_FORMAT_R16G16_SFLOAT;
				default:
					return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 32)
			{
				switch (type.basetype)
				{
				case SPIRType::Int:
					return VK_FORMAT_R32G32_SINT;
				case SPIRType::UInt:
					return VK_FORMAT_R32G32_UINT;
				case SPIRType::Float:
					return VK_FORMAT_R32G32_SFLOAT;
				default:
					return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 64)
			{
				switch (type.basetype)
				{
				case SPIRType::Int64:
					return VK_FORMAT_R64G64_SINT;
				case SPIRType::UInt64:
					return VK_FORMAT_R64G64_UINT;
				case SPIRType::Double:
					return VK_FORMAT_R64G64_SFLOAT;
				default:
					return VK_FORMAT_UNDEFINED;
				}
			}
			else
			{
				TRE_CORE_WARN("Invalid type width for conversion of SPIR-Type to VkFormat enum value!");
				return VK_FORMAT_UNDEFINED;
			}
		}
		else if (type.vecsize == 3)
		{
			if (type.width == 8)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R8G8B8_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R8G8B8_UINT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 16)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R16G16B16_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R16G16B16_UINT;
					case SPIRType::Float:
						return VK_FORMAT_R16G16B16_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 32)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R32G32B32_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R32G32B32_UINT;
					case SPIRType::Float:
						return VK_FORMAT_R32G32B32_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 64)
			{
				switch (type.basetype)
				{
					case SPIRType::Int64:
						return VK_FORMAT_R64G64B64_SINT;
					case SPIRType::UInt64:
						return VK_FORMAT_R64G64B64_UINT;
					case SPIRType::Double:
						return VK_FORMAT_R64G64B64_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else
			{
				TRE_CORE_WARN("Invalid type width for conversion of SPIR-Type to VkFormat enum value!");
				return VK_FORMAT_UNDEFINED;
			}
		}
		else if (type.vecsize == 4)
		{
			if (type.width == 8)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R8G8B8A8_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R8G8B8A8_UINT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 16)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R16G16B16A16_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R16G16B16A16_UINT;
					case SPIRType::Float:
						return VK_FORMAT_R16G16B16A16_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 32)
			{
				switch (type.basetype)
				{
					case SPIRType::Int:
						return VK_FORMAT_R32G32B32A32_SINT;
					case SPIRType::UInt:
						return VK_FORMAT_R32G32B32A32_UINT;
					case SPIRType::Float:
						return VK_FORMAT_R32G32B32A32_SFLOAT; 
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else if (type.width == 64)
			{
				switch (type.basetype)
				{
					case SPIRType::Int64:
						return VK_FORMAT_R64G64B64A64_SINT;
					case SPIRType::UInt64:
						return VK_FORMAT_R64G64B64A64_UINT;
					case SPIRType::Double:
						return VK_FORMAT_R64G64B64A64_SFLOAT;
					default:
						return VK_FORMAT_UNDEFINED;
				}
			}
			else
			{
				TRE_CORE_WARN("Invalid type width for conversion to a VkFormat enum");
				return VK_FORMAT_UNDEFINED;
			}
		}
		else
		{
			TRE_CORE_WARN("Vector size in vertex input attributes isn't explicitly supported for parsing from SPIRType->VkFormat");
			return VK_FORMAT_UNDEFINED;
		}
	}

	ShaderCompiler::ShaderCompiler(const std::filesystem::path& ShaderPath, bool EnableOptimization) : m_ShaderPath(ShaderPath), m_EnableOptimization(EnableOptimization)
	{
		m_ShaderLanguage = ShaderLanguage::GLSL; //For the sake of allowing it to be modular in future
	}

	std::unique_ptr<Shader> ShaderCompiler::CompileShader(const std::filesystem::path& ShaderPath, bool EnableOptimization)
	{
		std::string path = ShaderPath.string();
		size_t found = path.find_last_of("/\\");
		std::string name = found != std::string::npos ? path.substr(found + 1) : path;
		found = name.find_last_of('.');
		name = found != std::string::npos ? name.substr(0, found) : name;
		std::string shaderStage = path.substr(path.find_last_of('.') + 1);
		VkShaderStageFlagBits ShaderStage{};
		if (shaderStage == "vert")
		{
			ShaderStage = VK_SHADER_STAGE_VERTEX_BIT;
		}
		else if (shaderStage == "frag")
		{
			ShaderStage = VK_SHADER_STAGE_FRAGMENT_BIT;
		}
		else
			TRE_CORE_CRITICAL("Shader stage not supported");
		TRE_CORE_INFO("Shader Name: {0}", name);

		std::unique_ptr<ShaderCompiler> Compiler = std::make_unique<ShaderCompiler>(ShaderPath, true);
		Compiler->Compile(ShaderStage);

		std::unique_ptr<Shader> GeneratedShader = std::make_unique<Shader>(ShaderPath);
		GeneratedShader->m_ShaderName = name;
		GeneratedShader->LoadAndCreateShader(Compiler->m_SPIRVData, ShaderStage);
		GeneratedShader->SetReflectionData(Compiler->m_ReflectionData);
		GeneratedShader->CreateDescriptors();

		return std::move(GeneratedShader);
	}

	void ShaderCompiler::Compile(VkShaderStageFlagBits ShaderStage)
	{
		m_SPIRVData.clear();

		std::string RawCode = ReadGLSLToString(m_ShaderPath.string());
		std::string PreProcessCode = PreProcess(RawCode, ShaderStage);
		
		if (auto Compiled = CompileGLSLToBinary(m_SPIRVData, PreProcessCode, ShaderStage); Compiled == false)
		{
			TRE_CORE_ERROR("Failed to compile shader");
		}
		else
		{
			TRE_CORE_INFO("Shader Compiled");
		}

		ReflectShaderData(ShaderStage, m_SPIRVData);

	}

	std::string ShaderCompiler::PreProcess(const std::string& Source, VkShaderStageFlagBits ShaderStage)
	{
		shaderc::Compiler ShaderCompiler;
		shaderc::CompileOptions options;
		auto PPResult = ShaderCompiler.PreprocessGlsl(Source, VulkanStageToShaderC(ShaderStage), m_ShaderPath.string().c_str(), options);
		if (PPResult.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			TRE_CORE_INFO("Shader Preprocess Error: {0}", PPResult.GetErrorMessage());
		}
		else
		{
			TRE_CORE_INFO("Success Precompiled");
		}

		std::string NewCode = { PPResult.cbegin(), PPResult.cend() };

		return NewCode;
	}

	bool ShaderCompiler::CompileGLSLToBinary(std::vector<uint32_t>& OutputBinary, const std::string& SourceCode, VkShaderStageFlagBits ShaderStage)
	{
		shaderc::Compiler ShaderCompiler;
		auto CompilationResult = ShaderCompiler.CompileGlslToSpv(SourceCode, VulkanStageToShaderC(ShaderStage), m_ShaderPath.string().c_str());
		auto status = CompilationResult.GetCompilationStatus();
		if (status == shaderc_compilation_status_success)
		{
			TRE_CORE_INFO("Shader Compiled");
			OutputBinary.clear();
			OutputBinary = { CompilationResult.begin(), CompilationResult.end() };
			return true;
		}
		else
		{
			TRE_CORE_INFO("Shader Compile Error: {0}", CompilationResult.GetErrorMessage());
			return false;
		}

		return true;
	}

	void ShaderCompiler::ReflectShaderData(VkShaderStageFlagBits ShaderStage, const std::vector<uint32_t>& ShaderBinary)
	{
		m_ReflectionData.PushConstants.clear();
		m_ReflectionData.DescriptorSets.clear();
		m_ReflectionData.VertexInputAttributeDescriptions.clear();

		spirv_cross::Compiler Compiler(ShaderBinary);
		auto Resources = Compiler.get_shader_resources();

		if (ShaderStage == VK_SHADER_STAGE_VERTEX_BIT)
		{
			//To sort stage inputs according to locations else will break, why issit not sorted tho?
			{
				std::map <std::uint32_t, SPIRV_CROSS_NAMESPACE::Resource> sort;
				for (auto& resource : Resources.stage_inputs)
				{
					sort[Compiler.get_decoration(resource.id, spv::DecorationLocation)] = resource;
				}
				Resources.stage_inputs.clear();
				for (const auto& resource : sort)
				{
					Resources.stage_inputs.push_back(resource.second);
				}
			}

			TRE_CORE_INFO("Reflecting Shader Stage Inputs, Size: {0}", Resources.stage_inputs.size());
			uint32_t OffsetStride = 0;
			for (const auto& resource : Resources.stage_inputs)
			{
				const auto& Name = resource.name;
				auto& Type = Compiler.get_type(resource.type_id);

				uint32_t Binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t Location = Compiler.get_decoration(resource.id, spv::DecorationLocation);

				VkVertexInputAttributeDescription VertexAttributeDesc{};
				VertexAttributeDesc.binding = Binding;
				VertexAttributeDesc.location = Location;
				VertexAttributeDesc.format = GetVulkanFormat(Type);
				VertexAttributeDesc.offset = OffsetStride;

				m_ReflectionData.VertexInputAttributeDescriptions.push_back(VertexAttributeDesc);
				TRE_CORE_TRACE("Reflected Input Stage: Name: {0} Binding:{1} Location: {2} Offset: {3}", Name, Binding, Location, OffsetStride);

				OffsetStride += GetStrideFromVulkanFormat(VertexAttributeDesc.format);
				//TRE_CORE_INFO("Reflected Input Stage: Offset: {0}", OffsetStride);
			}
			m_ReflectionData.VertexStride = OffsetStride;
		}

		TRE_CORE_INFO("Reflecting Uniform Buffers");
		for (const auto& resource : Resources.uniform_buffers)
		{
			auto Buffers = Compiler.get_active_buffer_ranges(resource.id);
			TRE_CORE_INFO("Size: {0}", Buffers.size());

			if (Buffers.size())
			{
				const auto& Name = resource.name;
				auto& Type = Compiler.get_type(resource.base_type_id);
				uint32_t MemberCount = (uint32_t)Type.member_types.size();

				uint32_t Binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
				uint32_t Set = Compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
				uint32_t Size = (uint32_t)Compiler.get_declared_struct_size(Type);

				if (Set >= m_ReflectionData.DescriptorSets.size())
					m_ReflectionData.DescriptorSets.resize(Set + 1);

				ShaderDescriptorSets& ShaderDescriptorSet = m_ReflectionData.DescriptorSets[Set];
				UniformBuffer_GLSL Ubo;
				Ubo.Binding = Binding;
				Ubo.Size = Size;
				Ubo.Name = Name;
				Ubo.ShaderStageFlag = VK_SHADER_STAGE_ALL;

				TRE_CORE_TRACE("Shader {0} ({1}, {2})", Name, Set, Binding);
				TRE_CORE_TRACE("Member Count: {0}", MemberCount);
				TRE_CORE_TRACE("Size: {0}", Size);
				
				ShaderDescriptorSet.UniformBuffers[Binding] = Ubo;
			}
		}

		TRE_CORE_INFO("Reflecting PushConstants, Size: {0}", Resources.push_constant_buffers.size());
		for (const auto& resource : Resources.push_constant_buffers)
		{
			const auto& Name = resource.name;
			TRE_CORE_INFO("Name: {0}", Name);

			auto& Type = Compiler.get_type(resource.base_type_id);
			uint32_t BufferSize = (uint32_t)Compiler.get_declared_struct_size(Type);
			uint32_t MemberCount = (uint32_t)Type.member_types.size(); //Number of member inside the struct
			uint32_t Offset = 0;

			if (m_ReflectionData.PushConstants.size())
				Offset = m_ReflectionData.PushConstants.back().Offset + m_ReflectionData.PushConstants.back().Size;

			auto& PushConstant = m_ReflectionData.PushConstants.emplace_back();
			PushConstant.Offset = Offset;
			PushConstant.ShaderStageFlag = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			PushConstant.Size = BufferSize - Offset;

			TRE_CORE_TRACE("Push Constant Name: {0}", Name);
			TRE_CORE_TRACE("Push Constant Member Count: {0}", MemberCount);
			TRE_CORE_TRACE("Push Constant Size: {0}", BufferSize);
		}

		TRE_CORE_INFO("Reflecting Image Samplers, Size: {0}", Resources.sampled_images.size());
		{
			//Sort according to sampler binding
			std::map <std::uint32_t, SPIRV_CROSS_NAMESPACE::Resource> sort;
			for (auto& resource : Resources.sampled_images)
			{
				sort[Compiler.get_decoration(resource.id, spv::DecorationBinding)] = resource;
			}
			Resources.sampled_images.clear();
			for (const auto& resource : sort)
			{
				Resources.sampled_images.push_back(resource.second);
			}
		}
		for (const auto& resource : Resources.sampled_images)
		{
			const auto& Name = resource.name;
			auto& BaseType = Compiler.get_type(resource.base_type_id);
			auto& Type = Compiler.get_type(resource.type_id);

			uint32_t binding = Compiler.get_decoration(resource.id, spv::DecorationBinding);
			uint32_t descriptorset = Compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			uint32_t dimension = BaseType.image.dim;
			uint32_t Arraysize = Type.array[0];

			if (Arraysize == 0)
				Arraysize = 1;

			if (descriptorset >= m_ReflectionData.DescriptorSets.size())
				m_ReflectionData.DescriptorSets.resize(descriptorset + 1);

			ShaderDescriptorSets& ShaderDescriptor = m_ReflectionData.DescriptorSets[descriptorset];
			auto& ImageSampler = ShaderDescriptor.ImageSamplers[binding];
			ImageSampler.Name = Name;
			ImageSampler.BindingPoint = binding;
			ImageSampler.DescriptorSet = descriptorset;
			ImageSampler.ShaderStage = ShaderStage;
			ImageSampler.ArraySize = Arraysize;

			TRE_CORE_TRACE("Image Sampler Name: {0}", Name);
			TRE_CORE_TRACE("Image Sampler Binding: {0}", binding);
		}
	}

	void ShaderCompiler::ClearReflectionData()
	{
		m_ReflectionData.DescriptorSets.clear();
		m_ReflectionData.PushConstants.clear();
	}

	std::string ShaderCompiler::ReadGLSLToString(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		std::string Result;

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		file.seekg(0, std::ios::end);
		auto FileSize = file.tellg();
		const int Skipped = SkipBOM(file);
		FileSize -= Skipped - 1;

		Result.resize(FileSize);

		file.read(Result.data() + 1, FileSize);
		Result[0] = '\t';
		file.close();

		return Result;
	}
	
	int ShaderCompiler::SkipBOM(std::istream& in)
	{
		char test[4] = { 0 };
		in.seekg(0, std::ios::beg);
		in.read(test, 3);
		if (strcmp(test, "\xEF\xBB\xBF") == 0)
		{
			in.seekg(3, std::ios::beg);
			return 3;
		}
		in.seekg(0, std::ios::beg);
		return 0;
	}
}