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
		Shader,
	};

	class Asset
	{
		public:
			Asset() = default;
			virtual ~Asset() {}
			static AssetHandle GenerateGUID();
			static std::string GetGUIDHex(const AssetHandle assetHandle);
			static AssetHandle GetGUIDFromHex(const std::string& GUID);
			static AssetHandle GetGUIDFromPath(const std::string& path);
			
			virtual void Serialize() {}

			void SetHandle(AssetHandle handle)
			{
				m_Handle = handle;
			}

			AssetHandle GetHandle() const
			{
				return m_Handle;
			}

			std::string GetHandleHex() const
			{
				std::stringstream ss;
				ss << std::hex << m_Handle;
				return ss.str();
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