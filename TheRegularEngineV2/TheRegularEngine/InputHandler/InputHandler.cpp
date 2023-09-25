#include "pch.h"
#include "GLFW/glfw3.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "EventSystem/Events/InputEvent.h"
#include "EventSystem/Events/EditorEvent.h"
#include "EventSystem/EventHandler/EventHandler.h"

#define DEBUG 1
namespace TRE
{
	void InputHandler::KeyCb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (glfwGetKey(win_ptr, key) == GLFW_PRESS)
		{
			//EventHandler::getEventHandlerInstance().Publish(ConsoleDebugEvent{ "Testing Key inputs here" });
			event.Publish(InputEvent {key, action});
			event.Publish(TypingEvent {key, mod});
		}
	}

	void InputHandler::MouseButtonCb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			//TRE_CORE_INFO("Mouse pressed:x {0}", key);
			event.Publish(InputEvent {button, action});
		}
		else if (glfwGetMouseButton(win_ptr, button) == GLFW_RELEASE)
		{
			//TRE_CORE_INFO("Mouse Released:x {0}", key);
			event.Publish(InputEvent {button, action});
		}
	}

	void InputHandler::MouseScrollCb(GLFWwindow* win_ptr, double xoffset, double yoffset)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.Publish(MouseScrollEvent {xoffset, yoffset});
	}

	void InputHandler::MousePosCb(GLFWwindow* win_ptr, double xpos, double ypos)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.Publish(MouseMoveEvent {xpos, ypos});
	}

	void InputHandler::MouseFocusCb(GLFWwindow* win_ptr, int entered)
	{
		(void)win_ptr;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		event.Publish(MouseFocusEvent {entered});
	}

	void TRE::InputHandler::CheckMouseEvent(GLFWwindow* win_ptr, int button, int action)
	{
		EventHandler& event = EventHandler::getEventHandlerInstance();
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			//TRE_CORE_INFO("Mouse Button: {0}", button);
			event.Publish(MouseHoldEvent {button, action});
		}
	}
}