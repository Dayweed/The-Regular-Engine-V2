#pragma once
#include "pch.h"
#include "Geom.h"

namespace TRE
{
	class GeomManager
	{
	public:
		static GeomManager& Instance()
		{
			static GeomManager instance;
			return instance;
		}

		void Serialize(const std::unique_ptr<Geom> geom, const std::string& filePath);
		std::unique_ptr<Geom> Deserialize(const std::string& filePath);
	private:
		GeomManager() {};
		GeomManager(GeomManager const&) = delete;
		void operator=(GeomManager const&) = delete;
		void* operator new(size_t) = delete;
	};
}