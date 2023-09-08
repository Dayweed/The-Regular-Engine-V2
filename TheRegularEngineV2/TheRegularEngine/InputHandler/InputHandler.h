#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

struct GLFWwindow;
namespace TRE
{
    /*!*****************************************************************************
    *\brief 	Contains static functions for glfw's callback functions.
    *
    *******************************************************************************/
    class InputHandler
    {
        public:
        enum class MouseCode
        {
            LEFTCLICK, RIGHTCLICK, MIDDLE, BOTTOMBUTTON, TOPBUTTON
        };
        /*!*****************************************************************************
        *\brief     Response function for keyboard callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	key         key token.
        *\param 	scancode    key scancode.
        *\param 	action      key action. (Pressed/Held/Released..)
        *\param 	mod         key mods. (Shift/Capslock/Ctrl..)
        *******************************************************************************/
        static void KeyCb(GLFWwindow* win_ptr, int key, int scancode, int action, int mod);
        /*!*****************************************************************************
        *\brief 	Response function for mouse button callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	button      mouse button.
        *\param 	action      key action. (Pressed/Held/Released..)
        *\param 	mod         key mods. (Shift/Capslock/Ctrl..)
        *******************************************************************************/
        static void MouseButtonCb(GLFWwindow* win_ptr, int button, int action, int mod);
        /*!*****************************************************************************
        *\brief 	Response function for mouse scroll callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	xoffset     new xoffset of scrollwheel.
        *\param 	yoffset     new yoffset of scrollwheel.
        *******************************************************************************/
        static void MouseScrollCb(GLFWwindow* win_ptr, double xoffset, double yoffset);
        /*!*****************************************************************************
        *\brief 	Response function for mouse position callbacks.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	xpos        new mouse position's x value.
        *\param 	ypos        new mouse position's y value.
        *******************************************************************************/
        static void MousePosCb(GLFWwindow* win_ptr, double xpos, double ypos);
        /*!*****************************************************************************
        *\brief 	Response function for when mouse is focus on application window.
        *
        *\param 	win_ptr     pointer to GLFW window.
        *\param 	entered     new window focus state.
        *******************************************************************************/
        static void MouseFocusCb(GLFWwindow* win_ptr, int entered);

        static void CheckMouseEvent(GLFWwindow* win_ptr, int button, int action);

        private:
    };
}

#endif