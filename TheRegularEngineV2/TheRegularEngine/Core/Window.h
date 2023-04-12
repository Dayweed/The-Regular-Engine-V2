#pragma once

namespace TRE
{
	class GLFWWindow;

	class Window
	{
		public:
			Window();
			~Window();

			GLFWWindow* GetWindowHandle();

		private:
			GLFWWindow* m_WindowHandle;
	};
}