#pragma once
#include "pch.h"
#include "Core/entt.hpp"
#include "RenderObject.h"

namespace TRE
{
	class MeshRenderer
	{
	public:
		std::shared_ptr<RenderObject> m_RenderObject;
	};
}