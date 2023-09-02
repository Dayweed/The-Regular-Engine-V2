#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H
#include "pch.h"
#include "GLFW/glfw3.h"
namespace TRE
{
    /*!*****************************************************************************
    *\brief 	Contains static functions for glfw's callback functions.
    *
    *******************************************************************************/
    class InputHandler
    {
        public:
        /*!*****************************************************************************
        *\brief     Response function for keyboard callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	key         key token.
        *\param 	scancode    key scancode.
        *\param 	action      key action. (Pressed/Held/Released..)
        *\param 	mod         key mods. (Shift/Capslock/Ctrl..)
        *******************************************************************************/
        static void key_cb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod);
        /*!*****************************************************************************
        *\brief 	Response function for mouse button callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	button      mouse button.
        *\param 	action      key action. (Pressed/Held/Released..)
        *\param 	mod         key mods. (Shift/Capslock/Ctrl..)
        *******************************************************************************/
        static void mousebutton_cb(GLFWwindow* win_ptr, int button, int action, int mod);
        /*!*****************************************************************************
        *\brief 	Response function for mouse scroll callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	xoffset     new xoffset of scrollwheel.
        *\param 	yoffset     new yoffset of scrollwheel.
        *******************************************************************************/
        static void mousescroll_cb(GLFWwindow* win_ptr, double xoffset, double yoffset);
        /*!*****************************************************************************
        *\brief 	Response function for mouse position callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	xpos        new mouse position's x value.
        *\param 	ypos        new mouse position's y value.
        *******************************************************************************/
        static void mousepos_cb(GLFWwindow* win_ptr, double xpos, double ypos);
        /*!*****************************************************************************
        *\brief 	Response function for when mouse is focus on application window.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	entered     new window focus state.
        *******************************************************************************/
        static void mousefocus_cb(GLFWwindow* win_ptr, int entered);

        private:
            bool m_KeysHeld[512];
            bool m_KeysPressed[512];
            bool m_KeysReleased[512];
            bool m_MouseButtonsHeld[32];
            bool m_MouseButtonsPressed[32];
            bool m_MouseButtonsReleased[32];
    };
}

#endif