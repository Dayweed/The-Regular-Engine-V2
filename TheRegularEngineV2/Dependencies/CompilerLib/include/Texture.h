#pragma once
#include <string>
#include <memory>
#include <array>
#include <iostream>

namespace TRE
{
	struct Texture
	{
		//std::array<char, 64>	Name;
		std::uint32_t			Width;
		std::uint32_t			Height;
		int						Format;
		int						Filter;
		std::uint32_t			DataSize;
		void*					Data;

		~Texture()
		{
			delete[] Data;
		}

		static void RunCompiler(std::string descPath);
		static void Serialize(const std::string& filePath, const std::unique_ptr<Texture> geom);
		static std::unique_ptr<Texture> Deserialize(const std::string& filePath);
	};
}