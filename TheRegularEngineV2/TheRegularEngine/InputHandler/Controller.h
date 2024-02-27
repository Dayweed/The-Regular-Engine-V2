#include <Windows.h>
#include <Xinput.h>
#include <array>
#include <unordered_map>

// Link against the XInput library
#pragma comment(lib, "Xinput9_1_0.lib")

class XInputController {
public:
    static const int MAX_CONTROLLERS = 4;

    enum class Button 
    {
	    A,
        B,
        X,
        Y,
        LB,
        RB,
        Start,
        Back,
        LeftThumb,
        RightThumb
    };

    // singleton the class so that only one instance of the class is created
	static XInputController& Instance() {
		static XInputController instance;
		return instance;
	}

    void update() {
        //std::cout << "XInputController::update() Called." << std::endl;
        DWORD dwResult;
        for (DWORD i = 0; i < MAX_CONTROLLERS; ++i) {
            dwResult = XInputGetState(i, &controllers[i].state);
            if (dwResult == ERROR_SUCCESS) {
                controllers[i].isConnected = true;
            } else {
                controllers[i].isConnected = false;
            }
        }

        // handle held buttons
        for (DWORD i = 0; i < MAX_CONTROLLERS; ++i) {
            for (auto& buttonState : m_ButtonStates[i]) {
                bool isPressed = (controllers[i].state.Gamepad.wButtons & static_cast<WORD>(buttonState.first)) != 0;
                buttonState.second.update(isPressed);
            }
        }
    }

    bool isButtonPressed(int controllerNum, Button button) const {
		return isButtonPressed(controllerNum, getButtonMask(button));
	}

    bool isButtonHeld(int controllerNum, Button button, int framesThreshold) const {
	    if (isControllerConnected(controllerNum)) {
	        WORD buttonMask = getButtonMask(button);
	        auto it = m_ButtonStates[controllerNum].find(buttonMask);

	        if (it != m_ButtonStates[controllerNum].end()) {
	            return it->second.holdTime >= framesThreshold;
	        }
	    }
	    return false;
	}

    bool isButtonPressed(int controllerNum, WORD button) const {
        if (isControllerConnected(controllerNum)) {
            return (controllers[controllerNum].state.Gamepad.wButtons & button) != 0;
        }
        return false;
    }

    bool isButtonReleased(int controllerNum, Button button) const {
        if (isControllerConnected(controllerNum)) {
            WORD buttonMask = getButtonMask(button);
            auto it = m_ButtonStates[controllerNum].find(buttonMask);

            if (it != m_ButtonStates[controllerNum].end()) {
                return it->second.isReleased;
            }
        }
        return false;
    }

    float getTriggerValue(int controllerNum, bool rightTrigger) const {
        if (isControllerConnected(controllerNum)) {
            return rightTrigger ? 
                controllers[controllerNum].state.Gamepad.bRightTrigger / 255.0f :
                controllers[controllerNum].state.Gamepad.bLeftTrigger / 255.0f;
        }
        return 0.0f;
    }

    std::pair<float, float> getThumbstickValue(int controllerNum, bool rightStick) const {
        if (isControllerConnected(controllerNum)) {
            const float DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE; // Use the thumbstick deadzone
            const float MAX_RANGE = 32767; // Max range for thumbstick

            float x = rightStick ? 
                controllers[controllerNum].state.Gamepad.sThumbRX :
                controllers[controllerNum].state.Gamepad.sThumbLX;

            float y = rightStick ? 
                controllers[controllerNum].state.Gamepad.sThumbRY :
                controllers[controllerNum].state.Gamepad.sThumbLY;

            // Apply deadzone
            float magnitude = sqrt(x * x + y * y);
            if (magnitude < DEADZONE) {
                x = 0;
                y = 0;
            } else {
                if (magnitude > MAX_RANGE) {
                    magnitude = MAX_RANGE;
                }
                x = (x / magnitude) * ((magnitude - DEADZONE) / (MAX_RANGE - DEADZONE));
                y = (y / magnitude) * ((magnitude - DEADZONE) / (MAX_RANGE - DEADZONE));
            }

            return { x / MAX_RANGE, y / MAX_RANGE };
        }
        return { 0.0f, 0.0f };
    }

    bool isControllerConnected(int controllerNum) const {
        if (controllerNum >= 0 && controllerNum < MAX_CONTROLLERS) {
            return controllers[controllerNum].isConnected;
        }
        return false;
    }

private:

    XInputController() {
        controllers.fill({ 0, {}, {}, false });
    }

    struct ButtonState
    {
        bool isReleased;
	    bool isPressed;
        int holdTime;

        void update(bool pressed)
        {
        	if (pressed)
        	{
        		isPressed = true;
        		holdTime++;
			}
        	else
			{
                if(isPressed)
                {
                	isReleased = true;
				}
                else
                {
                	isReleased = false;
                }
                isPressed = false;
				holdTime = 0;
			}

		}
    };

    std::unordered_map<WORD, ButtonState> m_ButtonStates[MAX_CONTROLLERS];

     WORD getButtonMask(Button button) const {
        switch (button) {
            case Button::A: return XINPUT_GAMEPAD_A;
            case Button::B: return XINPUT_GAMEPAD_B;
            case Button::X: return XINPUT_GAMEPAD_X;
            case Button::Y: return XINPUT_GAMEPAD_Y;
            case Button::LB: return XINPUT_GAMEPAD_LEFT_SHOULDER;
            case Button::RB: return XINPUT_GAMEPAD_RIGHT_SHOULDER;
            case Button::Start: return XINPUT_GAMEPAD_START;
            case Button::Back: return XINPUT_GAMEPAD_BACK;
            case Button::LeftThumb: return XINPUT_GAMEPAD_LEFT_THUMB;
            case Button::RightThumb: return XINPUT_GAMEPAD_RIGHT_THUMB;
            default: return 0; // Invalid button, handle accordingly
        }
    }
      
    struct ControllerState {
        DWORD dwPacketNumber;
        XINPUT_STATE state;
        XINPUT_VIBRATION vibration;
        bool isConnected;
    };

    std::array<ControllerState, MAX_CONTROLLERS> controllers;
};