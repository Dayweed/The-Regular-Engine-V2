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
	//std::unordered_map<int, int> InputHandler::m_keyTriggerMap;
	std::unordered_map<int, int> InputHandler::m_keyPreviousPress;

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
		else if (glfwGetKey(win_ptr, key) == GLFW_RELEASE)
		{
			event.Publish(InputEvent {key, action});
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
			//TRE_CORE_INFO("Mouse pressed:x {0}", key);
			event.Publish(InputEvent {button, action});
		}
		else if (glfwGetMouseButton(win_ptr, button) == GLFW_RELEASE)
		{
			//TRE_CORE_INFO("Mouse Released:x {0}", key);
			//event.Publish(InputEvent {button, action});
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
		return m_keyMap[key];
	}

	bool InputHandler::GetKeyPress(int key)
	{
		//if (glfwGetKey(Engine::GetInstance().GetWindow()->GetWindowHandle(), (int)key) == GLFW_RELEASE)
		//{
		//	m_keyPreviousPress[key] = 0;
		//}
		/*else */if (glfwGetKey(Engine::GetInstance().GetWindow()->GetWindowHandle(), (int)key) == GLFW_PRESS && m_keyPreviousPress[key] == false)
		{
			m_keyPreviousPress[key] = 1;
			return true;
		}
		else
		{
			return false;
		}
	}

	bool InputHandler::GetKeyRelease(int key)
	{
		if (glfwGetKey(Engine::GetInstance().GetWindow()->GetWindowHandle(), (int)key) == GLFW_RELEASE && m_keyPreviousPress[key] == true)
		{
			m_keyPreviousPress[key] = 0;
			return true;
		}
		else
		{
			return false;
		}
	}
}