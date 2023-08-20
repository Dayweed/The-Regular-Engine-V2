#include "pch.h"
#include "MeshRenderer.h"

namespace TRE
{
	/*MeshRenderer::MeshRenderer()
	{
		std::cout << "ctor\n";
	}

	MeshRenderer::~MeshRenderer()
	{
		std::cout << "dtor\n";
	}
	*/
	void MeshRenderer::SetRenderObject(const std::shared_ptr<RenderObject>& renderObject)
	{
		m_RenderObject = renderObject;
	}
}