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
	void TextureCompiler::Compile(const TextureDescriptorFile& descriptor)
	{
		std::uint32_t fileSize;
		std::uint8_t* fileData = ReadFileIntoBuffer(descriptor.GetAssetPath().c_str(), fileSize);
		if (fileData == nullptr)
		{
			std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
			return;
		}
		int width, height, actual_comps;
		std::uint32_t* pixels = (std::uint32_t*)stbi_load_from_memory(fileData, fileSize, &width, &height, &actual_comps, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
			return;
		}

		void* outputData = (void*)pixels;
		std::uint32_t outputSize = width * height * 4;
		crn_format format;

		if (descriptor.GetCompress())
		{
			SYSTEM_INFO sys_info;
			GetSystemInfo(&sys_info);
			int num_threads = std::max<int>(0, (int)sys_info.dwNumberOfProcessors - 1);
			//Max num of threads for crnlib is 16
			if (num_threads > 16)
			{
				num_threads = 16;
			}

			format = descriptor.GetNormalMap() ? cCRNFmtDXT5 : cCRNFmtDXT5;

			//Do compression here
			crn_comp_params comp_params;
			comp_params.clear();
			comp_params.m_file_type = cCRNFileTypeDDS;
			comp_params.m_faces = 1;
			comp_params.m_width = width;
			comp_params.m_height = height;
			comp_params.m_format = format;
			comp_params.m_pImages[0][0] = pixels;
			comp_params.m_quality_level = 128;
			comp_params.m_levels = 1;
			comp_params.m_dxt_quality = cCRNDXTQualitySuperFast;
			comp_params.m_num_helper_threads = num_threads;

			outputData = crn_compress(comp_params, outputSize);
			if (outputData == nullptr || outputSize == 0)
			{
				std::cout << "Failed to compress texture: " << descriptor.GetAssetPath() << std::endl;
				return;
			}
		}

		m_Texture = std::make_unique<Texture>();
		m_Texture->Data = outputData;
		m_Texture->DataSize = outputSize;
		int len = static_cast<int>(std::min<int>((int)descriptor.GetTextureName().length(), sizeof(m_Texture->Name)));
		for (int i = 0; i < len; ++i)
			m_Texture->Name[i] = descriptor.GetTextureName()[i];
		m_Texture->Width = width;
		m_Texture->Height = height;
		if (descriptor.GetNormalMap())
		{
			m_Texture->Format = descriptor.GetCompress() ? 137 : 37; //VK_FORMAT_BC3_UNORM_BLOCK  : VK_FORMAT_R8G8B8A8_UNORM  
		}
		else
		{
			m_Texture->Format = descriptor.GetCompress() ? 138 : 43; //VK_FORMAT_BC3_SRGB_BLOCK  : VK_FORMAT_R8G8B8A8_SRGB 
		}
		m_Texture->Filter = descriptor.GetLinear();
	}
}