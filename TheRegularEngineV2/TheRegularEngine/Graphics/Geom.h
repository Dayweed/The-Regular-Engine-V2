#pragma once
#include "pch.h"

namespace TRE
{
	struct IColor
	{
		std::uint8_t R;
		std::uint8_t G;
		std::uint8_t B;
		std::uint8_t A;

		IColor(std::uint8_t r = 0, std::uint8_t g = 0, std::uint8_t b = 0, std::uint8_t a = 0) : R(r), G(g), B(b), A(a) {}

		bool operator==(const IColor& _Other) const
		{
			return R == _Other.R && G == _Other.G && B == _Other.B && A == _Other.A;
		}
	};

	struct Extra
	{
		glm::vec3 Color{};
		glm::vec3 Normal{};
		glm::vec2 UV{};
	};

	struct Submesh
	{
		std::vector<glm::vec3> Position;
		std::vector<Extra> Extra;
		std::vector<std::uint32_t> Indices;
		std::uint32_t MaterialIndex;

		//Next time then compress
	};

	struct Mesh
	{
		std::string Name;
		std::vector<Submesh> Vertices;
	};

	struct Geom
	{
		std::string Name;
		std::vector<Mesh> Meshes;
	};
}