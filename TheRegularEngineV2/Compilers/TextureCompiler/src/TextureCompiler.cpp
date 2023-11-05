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
		
		DXGI_FORMAT loadFormat, compileFormat;
		int vkFormat;

		bool issRGB = descriptor.GetsRGB();
		bool isTransparent = descriptor.GetTransparent();

		if (issRGB)
		{
			vkFormat = 43; // VK_FORMAT_R8G8B8A8_SRGB
		}
		else
		{
			vkFormat = 44; // VK_FORMAT_R8G8B8A8_UNORM
		}

		loadFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //I am assuming all textures that come in are sRGB

		//compileFormat = loadFormat;

		if (descriptor.GetCompress())
		{
			DirectX::Image image;
			image.width = width;
			image.height = height;
			image.format = loadFormat;
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
			metadata.format = loadFormat;
			metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
			scratchImage.Initialize(metadata);

			int BCn = descriptor.GetBCn();
			switch (BCn)
			{
			case (1):
				compileFormat = issRGB ? DXGI_FORMAT_BC1_UNORM_SRGB : DXGI_FORMAT_BC1_UNORM;
				if (issRGB)
				{
					vkFormat = isTransparent ? 134 : 132; // VK_FORMAT_BC1_RGBA_SRGB_BLOCK : VK_FORMAT_BC1_RGB_SRGB_BLOCK
				}
				else
				{
					vkFormat = isTransparent ? 133 : 131; // VK_FORMAT_BC1_RGBA_UNORM_BLOCK : VK_FORMAT_BC1_RGB_UNORM_BLOCK
				}
				break;
			case (3):
				compileFormat = issRGB ? DXGI_FORMAT_BC3_UNORM_SRGB : DXGI_FORMAT_BC3_UNORM;
				vkFormat = issRGB ? 138 : 137; // VK_FORMAT_BC3_SRGB_BLOCK  : VK_FORMAT_BC3_UNORM_BLOCK 
				break;
			case(5):
				compileFormat = DXGI_FORMAT_BC5_UNORM;
				vkFormat = 141; // VK_FORMAT_BC5_UNORM_BLOCK
				break;
			case (7):
				compileFormat = DXGI_FORMAT_BC7_UNORM;
				vkFormat = 145; // VK_FORMAT_BC7_UNORM_BLOCK
				break;
			default:
				std::cout << "Failed to compress texture image: " << descriptor.GetAssetPath() << std::endl;
				return;
			}

			HRESULT hr = DirectX::Compress(image, compileFormat, DirectX::TEX_COMPRESS_DEFAULT, 0.5f, scratchImage);
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
		m_Texture->Format = vkFormat;
		m_Texture->Filter = descriptor.GetLinear();

		delete[] fileData;
		stbi_image_free(pixels);
	}
}