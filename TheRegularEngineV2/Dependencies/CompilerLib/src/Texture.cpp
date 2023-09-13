#include "Texture.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <direct.h>

namespace TRE
{
	void Texture::RunCompiler(std::string descPath)
	{
		const char* textureExe = "..\\Compilers\\TextureCompiler.exe";
		if (std::filesystem::exists(textureExe) == false)
		{
			std::cout << "Error: TextureCompiler.exe does not exist" << std::endl;
			return;
		}

		char currentDir[FILENAME_MAX];
		if (_getcwd(currentDir, sizeof(currentDir)))
		{
			//std::string exePath = currentDir;
			//exePath += "\\";
			//exePath += textureExe;
			std::string exePath = textureExe;

			std::replace(descPath.begin(), descPath.end(), '/', '\\');
			//std::string newDescPath = currentDir;
			//newDescPath += "\\";
			//newDescPath += descPath;
			std::string newDescPath = descPath;

			char command[256];
			snprintf(command, sizeof(command), "\"%s %s\"", exePath.c_str(), newDescPath.c_str());

			int result = system(command);
			if (result != 0)
			{
				std::cout << "Error: TextureCompiler.exe failed to run. Code: " << result << std::endl;
				return;
			}
		}
		else
		{
			std::cout << "Error: could not get current directory" << std::endl;
			return;
		}
	}

	void Texture::Serialize(const std::string& filePath, const std::unique_ptr<Texture> texture)
	{
		std::ofstream file(filePath, std::ios::binary);

		file.write(reinterpret_cast<const char*>(texture->Name.data()), sizeof(Texture::Name));
		file.write(reinterpret_cast<const char*>(&texture->Width), sizeof(Texture::Width));
		file.write(reinterpret_cast<const char*>(&texture->Height), sizeof(Texture::Height));
		file.write(reinterpret_cast<const char*>(&texture->Format), sizeof(Texture::Format));
		file.write(reinterpret_cast<const char*>(&texture->Filter), sizeof(Texture::Filter));
		file.write(reinterpret_cast<const char*>(&texture->DataSize), sizeof(Texture::DataSize));
		file.write(reinterpret_cast<const char*>(texture->Data), texture->DataSize);

		file.close();
	}

	std::unique_ptr<Texture> Texture::Deserialize(const std::string& filePath)
	{
		auto texture = std::make_unique<Texture>();

		std::filesystem::path path = filePath;
		if (std::filesystem::exists(filePath))
		{
			std::ifstream file(filePath, std::ios::binary);
			if (file.is_open())
			{
				file.seekg(0, std::ios::end);
				std::size_t size = file.tellg();
				file.seekg(0, std::ios::beg);

				char* buffer = new char[size];
				file.read(buffer, size);
				file.close();

				std::size_t offset = 0;
				//Name
				memcpy(texture->Name.data(), buffer + offset, sizeof(Texture::Name));
				offset += sizeof(Texture::Name);
				//Width
				texture->Width = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);
				//Height
				texture->Height = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);
				//Format
				texture->Format = *reinterpret_cast<int*>(buffer + offset);
				offset += sizeof(int);
				//Filter
				texture->Filter = *reinterpret_cast<int*>(buffer + offset);
				offset += sizeof(int);
				//DataSize
				texture->DataSize = *reinterpret_cast<std::uint32_t*>(buffer + offset);
				offset += sizeof(std::uint32_t);
				//Data
				texture->Data = new char[texture->DataSize];
				memcpy(texture->Data, buffer + offset, texture->DataSize);

				delete[] buffer;
			}
			else
			{
				std::cout << "Error: could not open file: " << filePath << std::endl;
			}
		}
		else
		{
			std::cout << "Error: file does not exist: " << filePath << std::endl;
		}

		return std::move(texture);
	}
}