#include "pch.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "../EventSystem/Events/InputEvent.h"

namespace TRE
{
	void InputHandler::key_cb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		switch (action)
		{
			case GLFW_PRESS:
			{
				TRE_CORE_INFO("Key pressed: {0}", key);
				event.publish(InputEvent {key, action});
				break;
			}
			case GLFW_REPEAT:
				break;
			case GLFW_RELEASE:
				event.publish(InputEvent {key, action});
				break;
			default:
				break;
		}
	}

	void InputHandler::mousebutton_cb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		switch(action)
		{
			case GLFW_PRESS:
				break;
			case GLFW_REPEAT:
				break;
			case GLFW_RELEASE:
				event.publish(InputEvent {button, action});
				break;
			default:
				break;
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