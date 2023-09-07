#include "pch.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "EventSystem/Events/InputEvent.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "GLFW/glfw3.h"
#define DEBUG 1
namespace TRE
{
	void InputHandler::key_cb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (glfwGetKey(win_ptr, key) == GLFW_PRESS)
			event.publish(InputEvent {key, action});
	}

	void InputHandler::mousebutton_cb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		switch(action)
		{
			case GLFW_PRESS:
				event.publish(InputEvent {button, (int)KeyState::keyPressed});
				break;
			case GLFW_REPEAT:
				//event.publish(InputEvent {button, (int)KeyState::keyHeld});
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