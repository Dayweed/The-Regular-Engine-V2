#include "Graphics/Device.h"
#include "Graphics/Material.h"

namespace TRE
{
	class PBR : public Material
	{
	public:
		static const ResourceHandle& GetDefaultHandle();

	private:
		static void CreateDefaultMaterial();

	private:
		static ResourceHandle m_DefaultHandle;
	};
}