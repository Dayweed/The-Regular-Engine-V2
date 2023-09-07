#include "pch.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "EventSystem/Events/InputEvent.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "GLFW/glfw3.h"
#define DEBUG 1
namespace TRE
{
	bool InputHandler::m_IsMouseHeld = false;
	void InputHandler::KeyCb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (glfwGetKey(win_ptr, key) == GLFW_PRESS)
		{
			TRE_CORE_INFO("Key pressed: {0}", key);
			event.publish(InputEvent {key, action});
		}
	}

	void InputHandler::MouseButtonCb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (m_IsMouseHeld)
		{
			TRE_CORE_INFO("Button pressed: {0}", button);
			event.publish(InputEvent {button, action});
		}
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			m_IsMouseHeld = true;
			TRE_CORE_INFO("Button pressed: {0}", button);
			event.publish(InputEvent {button, action});
		}
		if (glfwGetMouseButton(win_ptr, button) == GLFW_RELEASE)
		{
			m_IsMouseHeld = false;
		}
	}

	void InputHandler::MouseScrollCb(GLFWwindow* win_ptr, double xoffset, double yoffset)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseScrollEvent {xoffset, yoffset});
	}

	void InputHandler::MousePosCb(GLFWwindow* win_ptr, double xpos, double ypos)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseMoveEvent {xpos, ypos});
	}

	void InputHandler::MouseFocusCb(GLFWwindow* win_ptr, int entered)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseFocusEvent {entered});
	}
}