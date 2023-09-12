#pragma once

namespace TRE
{
	using AssetHandle = uint64_t;

	enum class AssetType : uint16_t
	{
		Audio,
		Font,
		Mesh,
		Texture,
	};

	class Asset
	{
		public:
			Asset() = default;
			virtual ~Asset() {}

		private:
			AssetHandle m_Handle;
			AssetType m_Type;
	};
}