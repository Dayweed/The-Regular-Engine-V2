#include "TextureCompiler.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "DirectXTex.h"

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
		stbi_uc* pixels = stbi_load_from_memory(fileData, fileSize, &width, &height, &actual_comps, STBI_rgb_alpha);

		if (pixels == nullptr)
		{
			std::cout << "Failed to load texture image: " << descriptor.GetAssetPath() << std::endl;
			return;
		}

		void* outputData = nullptr;
		std::uint32_t outputSize = width * height * 4;
		if (descriptor.GetCompress())
		{
			DirectX::Image image;
			image.width = width;
			image.height = height;
			image.format = DXGI_FORMAT_R8G8B8A8_UNORM;
			image.rowPitch = width * 4;
			image.slicePitch = image.rowPitch * height;
			image.pixels = pixels;

			DirectX::ScratchImage scratchImage{};
			DirectX::TexMetadata metadata{};
			metadata.width = width;
			metadata.height = height;
			metadata.depth = 1;
			metadata.arraySize = 1;
			metadata.mipLevels = 1;
			metadata.miscFlags = 0;
			metadata.miscFlags2 = 0;
			metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
			scratchImage.Initialize(metadata);

			HRESULT hr = DirectX::Compress(image, DXGI_FORMAT_BC1_UNORM, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, scratchImage);
			if (FAILED(hr))
			{
				std::cout << "Failed to compress texture image: " << descriptor.GetAssetPath() << std::endl;
				return;
			}

			outputSize = scratchImage.GetPixelsSize();
			outputData = new void*[outputSize];
			memcpy(outputData, scratchImage.GetPixels(), outputSize);
		}
		else
		{
			outputData = new void*[outputSize];
			memcpy(outputData, pixels, outputSize);
		}

		m_Texture = std::make_unique<Texture>();
		m_Texture->DataSize = outputSize;
		m_Texture->Data = new void*[outputSize];
		memcpy(m_Texture->Data, outputData, outputSize);
		m_Texture->Width = width;
		m_Texture->Height = height;
		if (descriptor.GetNormalMap())
		{
			m_Texture->Format = descriptor.GetCompress() ? 137 : 37; //VK_FORMAT_BC3_UNORM_BLOCK  : VK_FORMAT_R8G8B8A8_UNORM  
		}
		else
		{
			m_Texture->Format = descriptor.GetCompress() ? 131 /*138*/ : 43; //VK_FORMAT_BC3_SRGB_BLOCK  : VK_FORMAT_R8G8B8A8_SRGB 
		}
		m_Texture->Filter = descriptor.GetLinear();

		delete[] fileData;
		stbi_image_free(pixels);
	}
}