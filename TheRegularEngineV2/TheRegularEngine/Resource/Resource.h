#pragma once

namespace TRE
{
	using ResourceHandle = uint64_t;

	enum class ResourceType : uint16_t
	{
		None,
		Audio,
		Font,
		Mesh,
		Texture,
		Material,
		Shader,
	};

	class Resource
	{
	public:
		Resource() = default;
		virtual ~Resource() {}
		static ResourceHandle GenerateGUID();
		static ResourceHandle GenerateGUID(const std::string& assetName);
		static std::string GetGUIDHex(const ResourceHandle assetHandle);
		static ResourceHandle GetGUIDFromHex(const std::string& GUID);
		static ResourceHandle GetGUIDFromPath(const std::string& path);
			
		virtual void Serialize() {}

		void SetHandle(ResourceHandle handle)
		{
			m_Handle = handle;
		}

		ResourceHandle GetHandle() const
		{
			return m_Handle;
		}

		ResourceHandle& GetHandleRef()
		{
			return m_Handle;
		}

		std::string GetHandleHex() const
		{
			std::stringstream ss;
			ss << std::hex << m_Handle;
			return ss.str();
		}

		ResourceType GetType() const
		{
			return m_Type;
		}

	protected:
		ResourceHandle m_Handle{ 0 };
		ResourceType m_Type{ ResourceType::None };
	};
}