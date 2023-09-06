#include "pch.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "../EventSystem/Events/InputEvent.h"
#include "EventSystem/EventHandler/EventHandler.h"
#define DEBUG 1
namespace TRE
{
	bool InputHandler::m_isMouseHeld = false;
	void InputHandler::key_cb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
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

	void InputHandler::mousebutton_cb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (m_isMouseHeld)
		{
			TRE_CORE_INFO("Button pressed: {0}", button);
			event.publish(InputEvent {button, action});
		}
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			m_isMouseHeld = true;
			TRE_CORE_INFO("Button pressed: {0}", button);
			event.publish(InputEvent {button, action});
		}
		if (glfwGetMouseButton(win_ptr, button) == GLFW_RELEASE)
		{
			m_isMouseHeld = false;
		}
	}

	void InputHandler::mousescroll_cb(GLFWwindow* win_ptr, double xoffset, double yoffset)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseScrollEvent {xoffset, yoffset});
	}

	void InputHandler::mousepos_cb(GLFWwindow* win_ptr, double xpos, double ypos)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseMoveEvent {xpos, ypos});
	}

	void InputHandler::mousefocus_cb(GLFWwindow* win_ptr, int entered)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.publish(MouseFocusEvent {entered});
	}
}