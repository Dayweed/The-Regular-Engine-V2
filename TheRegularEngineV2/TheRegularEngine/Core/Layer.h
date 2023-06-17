#pragma once

namespace TRE
{
	class Layer
	{
	public:
		Layer() = default;
		~Layer() = default;
		virtual void Update() = 0;
		virtual void Shutdown() = 0;
	};

}