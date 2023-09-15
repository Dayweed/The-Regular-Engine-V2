#ifndef INPUT_EVENT_H
#define INPUT_EVENT_H
#include "pch.h"
#include "EventBase.h"
#include "InputHandler/KeyButton.h"

namespace TRE
{
	struct InputEvent : Event
	{
		int _key, _state;
		InputEvent(const int& key, const int& state) : _key(key), _state(state)
		{}
	};

    struct TypingEvent : Event
    {
        int m_Key, m_Mod;
        TypingEvent(const int& key, const int& mod) : m_Key(key), m_Mod(mod)
		{}
    };

	struct MouseHoldEvent : Event
	{
		int _key, _state;

        MouseHoldEvent(const int& key, const int& state) : _key(key), _state(state)
		{}
	};

    struct MouseScrollEvent : Event
    {
        double _xoffset, _yoffset;
        MouseScrollEvent(const double& xoffset, const double& yoffset) : _xoffset(xoffset), _yoffset(yoffset)
        {}
    };

    struct MouseMoveEvent : Event
    {
        double _xpos, _ypos;
        MouseMoveEvent(const double& xpos, const double& ypos) : _xpos(xpos), _ypos(ypos)
        {}
    };

    struct MouseFocusEvent : Event
    {
        bool _isFocused;
        MouseFocusEvent(const int& entered) : _isFocused(entered)
        {}
    };
}
#endif