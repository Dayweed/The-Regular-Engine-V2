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
	std::unordered_map<InputHandler::MouseCode, bool> InputHandler::m_LastMouseEvent;
	std::unordered_map<InputHandler::MouseCode, bool> InputHandler::m_MouseEvent;
	void InputHandler::KeyCb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod)
	{
		(void)win_ptr;
		(void)scancode;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();

		if (glfwGetKey(win_ptr, key) == GLFW_PRESS)
		{
			TRE_CORE_INFO("Key pressed: {0}", key);
			event.Publish(InputEvent {key, action});
		}
	}

	void InputHandler::MouseButtonCb(GLFWwindow* win_ptr, int button, int action, int mod)
	{
		(void)win_ptr;
		(void)mod;
		EventHandler& event = EventHandler::getEventHandlerInstance();
		MouseCode key = static_cast<MouseCode>(button);

		if (glfwGetMouseButton(win_ptr, button) == GLFW_PRESS)
		{
			TRE_CORE_INFO("Key pressed: {0}", key);
			event.Publish(InputEvent {button, action});
		}
		/*switch (action)
		{
		case(GLFW_PRESS):
			TRE_CORE_INFO("button pressed: {0}", button);
			m_MouseEvent.insert_or_assign(key, true);
			break;
		case(GLFW_RELEASE):
			TRE_CORE_INFO("button Release: {0}", button);	
			m_MouseEvent.insert_or_assign(key, false);	
			break;
		}
		event.Publish(InputEvent {button, action});*/
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

	bool TRE::InputHandler::CheckMouseEvent(MouseCode key)
	{
		EventHandler& event = EventHandler::getEventHandlerInstance();
		if (key == MouseCode::MIDDLE || key == MouseCode::LEFTCLICK || key == MouseCode::RIGHTCLICK)
		{
			if (m_MouseEvent[key] == false)
			{
				TRE_CORE_INFO("called");
				m_LastMouseEvent.insert_or_assign(key, false);
				event.Publish(InputEvent {static_cast<int>(key), GLFW_RELEASE});
				return false;
			}
			else if (m_MouseEvent[key] && m_LastMouseEvent[key] == false)
			{
				m_LastMouseEvent.insert_or_assign(key, true);
				m_MouseEvent.insert_or_assign(key, false);
				return true;
			}
		}
		else if (key == MouseCode::TOPBUTTON || key == MouseCode::BOTTOMBUTTON)
		{
			return m_MouseEvent[key];
		}
		return false;
	}
}