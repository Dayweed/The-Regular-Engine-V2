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
		Material,
	};

	class Asset
	{
		public:
			Asset() = default;
			virtual ~Asset() {}
			static AssetHandle GenerateGUID();

		public:
			void SetHandle(AssetHandle handle)
			{
				m_Handle = handle;
			}

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