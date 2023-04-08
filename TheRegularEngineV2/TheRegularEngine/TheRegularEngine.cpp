// TheRegularEngine.cpp : Defines the functions for the static library.
#include "pch.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

int main()
{
	glm::vec3 Test{ 0.1f, 1.f, 2.f };
	glfwInit();
	GLFWwindow* win = glfwCreateWindow(1000, 600, "Tomas Is My Hero", nullptr, nullptr);

	while (1)
	{
		glfwPollEvents();
	}
}
