#include "pch.h"
#include "Window.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	Window::Window()
	{
		if (int Err = glfwInit(); Err)
		{
			//Assert
		}
	}

	Window::~Window()
	{
		
	}

	GLFWWindow* Window::GetWindowHandle()
	{
		return m_WindowHandle;
	}
}