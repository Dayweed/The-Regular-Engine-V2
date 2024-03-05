#include <map>
#include <stdexcept>
#include <Xinput.h>
#include <chrono>

// Link against the XInput library
#pragma comment(lib, "Xinput9_1_0.lib")

enum class GamepadButton : int {
    A = 0,
    B = 1,
    X = 2,
    Y = 3,
    DPadUp = 4,
    DPadDown = 5,
    DPadLeft = 6,
    DPadRight = 7,
    LeftShoulder = 8,
    RightShoulder = 9,
    Start = 10,
    Back = 11,
    LeftThumb = 12,
    RightThumb = 13
};

struct ButtonState
{
     bool wasPressed;
    bool isPressed;
    bool isReleased;
    std::chrono::steady_clock::time_point pressTime;
    float holdTime;

    ButtonState() : wasPressed(false), isPressed(false), isReleased(false), holdTime(0.0f) {}

    void update(bool pressed)
    {
        this->wasPressed = this->isPressed;
        this->isPressed = pressed;
        this->isReleased = !pressed && this->wasPressed;

        if (this->isPressed) {
            if (!this->wasPressed) {
                pressTime = std::chrono::steady_clock::now();
            }
            holdTime = std::chrono::duration<float>(std::chrono::steady_clock::now() - pressTime).count();
        } else {
            holdTime = 0.0f;
        }
    }
};

class XInputController
{
public:
    struct ControllerState
    {
        XINPUT_STATE state;
        std::map<int, ButtonState> buttonStates;

        ControllerState() : state() {}
    };

    static XInputController& Instance()
    {
        static XInputController instance;
        return instance;
    }

    static const int MAX_CONTROLLERS = 4;
    ControllerState controllers[MAX_CONTROLLERS];



    WORD intToGamepadButton(int button) {
       auto it = ButtonMap.find(button);
        if (it != ButtonMap.end()){
            return it->second;
        }
        throw std::invalid_argument("Invalid button number");
    }

    void update()
    {
        DWORD dwResult;
        for (DWORD i = 0; i < MAX_CONTROLLERS; ++i)
        {
            ControllerState& controller = controllers[i];

            dwResult = XInputGetState(i, &controller.state);

            if (dwResult == ERROR_SUCCESS)
            {
                for (auto const& pair : ButtonMap)
                {
                    bool isPressed = (controller.state.Gamepad.wButtons & pair.second) != 0;
                    controller.buttonStates[pair.first].update(isPressed);
                }
            }
        }
    }

    bool isButtonPressed(int controllerNum, int button)
    {
        if (controllerNum < 0 || controllerNum >= MAX_CONTROLLERS)
        {
	        return false;
        }

        return controllers[controllerNum].buttonStates[button].isPressed;
    }

    bool isButtonHeld(int controllerNum, int button, float time)
    {
        if (controllerNum < 0 || controllerNum >= MAX_CONTROLLERS)
            return false;

        return controllers[controllerNum].buttonStates[button].holdTime > time;
    }

    bool isButtonReleased(int controllerNum, int button)
    {
        if (controllerNum < 0 || controllerNum >= MAX_CONTROLLERS)
            return false;

        return controllers[controllerNum].buttonStates[button].isReleased;
    }

    float getThumbstickX(int controllerNum, bool right) {
        if (controllerNum < 0 || controllerNum >= MAX_CONTROLLERS)
            return 0.0f;

        float value = right ? controllers[controllerNum].state.Gamepad.sThumbRX
                            : controllers[controllerNum].state.Gamepad.sThumbLX;

        return abs(value) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? 0.0f : value / 32767.0f;
    }

    float getThumbstickY(int controllerNum, bool right) {
        if (controllerNum < 0 || controllerNum >= MAX_CONTROLLERS)
            return 0.0f;

        float value = right ? controllers[controllerNum].state.Gamepad.sThumbRY
                            : controllers[controllerNum].state.Gamepad.sThumbLY;

        return abs(value) < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ? 0.0f : value / 32767.0f;
    }

    private:
    XInputController() {}
    XInputController(const XInputController&) = delete;
    void operator=(const XInputController&) = delete;

    std::map<int, WORD> ButtonMap = {
        { 0, XINPUT_GAMEPAD_A },
        { 1, XINPUT_GAMEPAD_B },
        { 2, XINPUT_GAMEPAD_X },
        { 3, XINPUT_GAMEPAD_Y },
        { 4, XINPUT_GAMEPAD_DPAD_UP },
        { 5, XINPUT_GAMEPAD_DPAD_DOWN },
        { 6, XINPUT_GAMEPAD_DPAD_LEFT },
        { 7, XINPUT_GAMEPAD_DPAD_RIGHT },
        { 8, XINPUT_GAMEPAD_LEFT_SHOULDER },
        { 9, XINPUT_GAMEPAD_RIGHT_SHOULDER },
        { 10, XINPUT_GAMEPAD_START },
        { 11, XINPUT_GAMEPAD_BACK },
        { 12, XINPUT_GAMEPAD_LEFT_THUMB },
        { 13, XINPUT_GAMEPAD_RIGHT_THUMB }
    };
};