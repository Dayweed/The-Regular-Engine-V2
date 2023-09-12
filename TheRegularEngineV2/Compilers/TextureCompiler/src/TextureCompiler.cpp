#include "TextureCompiler.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "crnlib.h"
#include "crn_decomp.h"
#include "dds_defs.h"
#include "windows.h"
#include <thread>

namespace
{
	std::uint8_t* ReadFileIntoBuffer(const char* filename, std::uint32_t& size)
	{
		size = 0;

		FILE* pFile = fopen(filename, "rb");
		if (!pFile)
			return nullptr;

		fseek(pFile, 0, SEEK_END);
		size = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		std::uint8_t* pBuf = new std::uint8_t[size];
		if (!pBuf)
		{
			fclose(pFile);
			return nullptr;
		}

		if (fread(pBuf, size, 1, pFile) != 1)
		{
			delete[] pBuf;
			fclose(pFile);
			return nullptr;
		}

		fclose(pFile);
		return pBuf;
	}
}

namespace TRE
{
	void TextureDescriptorFile::Write()
	{
		m_DescriptorFile << "Texture File Path:\n";
		m_TexturePath = m_AssetPath.substr(0, m_AssetPath.find_last_of("."));
		m_TexturePath += ".DDS";
		m_DescriptorFile << m_TexturePath << "\n\n";
		m_TextureName = m_TexturePath.substr(m_TexturePath.find_last_of("/") + 1);
		m_TextureName = m_TextureName.substr(0, m_TextureName.find_last_of("."));
		m_DescriptorFile << "Texture Name:\n";
		m_DescriptorFile << m_TextureName << "\n\n";
		m_DescriptorFile << "Check vulkan page for enums\n";
		m_DescriptorFile << "Texture Format:\n";
		m_DescriptorFile << m_Format << "\n\n";
		m_DescriptorFile << "Texture Filter:\n";
		m_DescriptorFile << m_Filter << "\n\n";
		m_DescriptorFile << "Compress:\n";
		m_DescriptorFile << m_Compress << "\n\n";
	}

	void TextureDescriptorFile::Read()
	{
		std::string line;
		std::getline(m_DescriptorFile, line);
		if (line == "Texture File Path:")
		{
			std::getline(m_DescriptorFile, line);
			m_TexturePath = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Name:")
		{
			std::getline(m_DescriptorFile, line);
			m_TextureName = line;
			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Name is not valid" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Format:")
		{
			std::getline(m_DescriptorFile, line);

			m_Format = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Format missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Texture Filter:")
		{
			std::getline(m_DescriptorFile, line);

			m_Filter = std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Filter missing" << std::endl;
			return;
		}
		std::getline(m_DescriptorFile, line);
		if (line == "Compress:")
		{
			std::getline(m_DescriptorFile, line);

			m_Compress = (bool)std::stoi(line);

			std::getline(m_DescriptorFile, line);
		}
		else
		{
			std::cout << "Error: Texture Compression flag missing" << std::endl;
			return;
		}
	}

	void TextureCompiler::Compile(const TextureDescriptorFile& descriptor)
	{
		/*int texWidth, texHeight, texChannels;*/
		//std::uint32_t fileSize;
		//std::uint8_t* fileData = ReadFileIntoBuffer(descriptor.GetAssetPath().c_str(), fileSize);
		//if (fileData == nullptr)
		//{
		//	std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
		//	return;
		//}
		int width, height, actual_comps;
		//std::uint32_t* pixels = (std::uint32_t*)stbi_load_from_memory(fileData, fileSize, &width, &height, &actual_comps, STBI_rgb_alpha);

		std::uint32_t* pixels = (std::uint32_t*)stbi_load(descriptor.GetAssetPath().c_str(), &width, &height, &actual_comps, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
			return;
		}
		void* outputData = (void*)pixels;
		std::uint32_t outputSize = width * height * 4;

		if (descriptor.GetCompress())
		{
			SYSTEM_INFO sys_info;
			GetSystemInfo(&sys_info);
			int num_threads = std::max<int>(0, (int)sys_info.dwNumberOfProcessors - 1);

			//Do compression here
			crn_comp_params comp_params;
			comp_params.m_file_type = cCRNFileTypeDDS;
			comp_params.m_faces = 1;
			comp_params.m_width = width;
			comp_params.m_height = height;
			comp_params.set_flag(cCRNCompFlagPerceptual, true);
			comp_params.set_flag(cCRNCompFlagDXT1AForTransparency, true);
			comp_params.set_flag(cCRNCompFlagHierarchical, true);
			comp_params.m_format = cCRNFmtDXT5;
			comp_params.m_pImages[0][0] = pixels;
			comp_params.m_quality_level = 128;
			comp_params.m_levels = 1;
			comp_params.m_dxt_quality = cCRNDXTQualitySuperFast;
			comp_params.m_num_helper_threads = num_threads;

			outputData = crn_compress(comp_params, outputSize);
			std::cout << "Compressed texture size: " << outputSize << std::endl;
			std::cout << "Compressing texture" << std::endl;
		}

		m_Texture = std::make_unique<Texture>();
		m_Texture->Data = outputData;
		m_Texture->DataSize = outputSize;
		int len = static_cast<int>(std::min<int>(descriptor.GetTextureName().length(), sizeof(m_Texture->Name)));
		for (int i = 0; i < len; ++i)
			m_Texture->Name[i] = descriptor.GetTextureName()[i];
		m_Texture->Width = width;
		m_Texture->Height = height;
		m_Texture->Format = descriptor.GetFormat();
		m_Texture->Filter = descriptor.GetFilter();

		//crn_free_block(outputData);
		//stbi_image_free(pixels);
	}
}