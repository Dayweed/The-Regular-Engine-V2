#include "pch.h"
#include "Core/Logger.h"
#include "InputHandler.h"
#include "EventSystem/Events/InputEvent.h"
#include "EventSystem/Events/EditorEvent.h"
#include "EventSystem/EventHandler/EventHandler.h"
#include "GLFW/glfw3.h"

namespace TRE
{
	std::unordered_map<int, int> InputHandler::m_keyMap;
	std::unordered_map<int, bool> InputHandler::m_keyPreviousPress;
	std::unordered_map<int, bool> InputHandler::m_keyPress;
	std::unordered_map<int, bool> InputHandler::m_keyRelease;


	void InputHandler::KeyCb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (glfwGetKey(win_ptr, key) == GLFW_PRESS)
		{
			event.Publish(InputEvent {key, action});
			event.Publish(TypingEvent {key, mod});
 			m_keyPress[key] = true;
		}
		else if (glfwGetKey(win_ptr, key) == GLFW_RELEASE)
		{
			event.Publish(InputEvent {key, action});
			m_keyPress[key] = false;
		}
		m_keyMap[key] = action;
	}

	void InputHandler::MouseButtonCb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			event.Publish(InputEvent {button, action});
		}
		else if (glfwGetMouseButton(win_ptr, button) == GLFW_RELEASE)
		{		
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

	void InputHandler::CheckMouseEvent(GLFWwindow* win_ptr, int button, int action)
	{
		EventHandler& event = EventHandler::getEventHandlerInstance();
		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			//TRE_CORE_INFO("Mouse Button: {0}", button);
			event.Publish(MouseHoldEvent {button, action});
		}
	}

	bool InputHandler::GetKeyHold(int key)
	{
		return m_keyMap[key] && m_keyPreviousPress[key];
	}

	bool InputHandler::GetKeyPress(int key)
	{
		return m_keyPress[key] && !m_keyPreviousPress[key];
	}

	bool InputHandler::GetKeyRelease(int key)
	{
		return !m_keyPress[key] && m_keyPreviousPress[key];
	}

	void InputHandler::ClearKeys()
	{
		for (std::pair<const int, bool>& key : m_keyPress)
		{
			m_keyPreviousPress[key.first] = key.second;
		}
	}
}