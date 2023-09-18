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

		spirv_cross::Compiler Compiler(ShaderBinary);
		auto Resources = Compiler.get_shader_resources();

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