#pragma once
#include "GLFW/glfw3.h"

namespace TRE
{
	class Window
	{
		public:
			Window();
			~Window();

			void PollEvents();

			GLFWwindow* GetWindowHandle();
			int ShouldWindowClose();

		private:
			GLFWwindow* m_WindowHandle = nullptr;
	};
}