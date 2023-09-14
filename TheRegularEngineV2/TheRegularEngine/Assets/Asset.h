#pragma once

namespace TRE
{
	using AssetHandle = uint64_t;

	enum class AssetType : uint16_t
	{
		None,
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

		public:
			AssetHandle GetHandle() const
			{
				return m_Handle;
			}

			AssetType GetType() const
			{
				return m_Type;
			}

		protected:
			AssetHandle m_Handle{ 0 };
			AssetType m_Type{ AssetType::None };
	};
}