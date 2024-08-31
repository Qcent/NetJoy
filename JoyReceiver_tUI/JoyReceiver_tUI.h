/*

Copyright (c) 2024 Dave Quinn <qcent@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#pragma once
#include "./../JoyReceiver++/JoyReceiver++.h"
#include <codecvt>
#include <thread>

#undef min
#undef max

#define APP_VERSION_NUM     L"2.0.0.0"

#define AXIS_INPUT_DEADZONE   3000    // stick input values must be greater than this to register
// stripped down SDLButtonMapping from JoySender++
class SDLButtonMapping {
public:
    enum class ButtonType {
        UNSET,
        HAT,
        STICK,
        THUMB,
        TRIGGER,
        SHOULDER,
        BUTTON
    };

    enum class ButtonName {
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        START,
        BACK,
        LEFT_THUMB,
        RIGHT_THUMB,
        LEFT_SHOULDER,
        RIGHT_SHOULDER,
        GUIDE,
        A,
        B,
        X,
        Y,
        LEFT_TRIGGER,
        RIGHT_TRIGGER,
        LEFT_STICK_LEFT,
        LEFT_STICK_UP,
        LEFT_STICK_RIGHT,
        LEFT_STICK_DOWN,
        RIGHT_STICK_LEFT,
        RIGHT_STICK_UP,
        RIGHT_STICK_RIGHT,
        RIGHT_STICK_DOWN,
    };

    class ButtonMapInput {
    public:
        ButtonType input_type;
        int index;
        int value;

        ButtonMapInput(ButtonType input_type = ButtonType::UNSET, int index = -1, int value = 0)
            : input_type(input_type), index(index), value(value) {}

        void set(ButtonType input_type, int index, int value) {
            this->input_type = input_type;
            this->index = index;
            this->value = value;
        }

        void clear() {
            input_type = ButtonType::UNSET;
            index = -1;
            value = 0;
        }

        bool operator==(const ButtonMapInput& other) const {
            return (this->input_type == other.input_type) &&
                (this->index == other.index) &&
                (this->value == other.value);
        }

    };

    using ButtonMap = std::unordered_map<ButtonName, ButtonMapInput>;

    ButtonMap buttonMaps;
    std::vector<ButtonName> stickButtonNames;
    std::vector<ButtonName> triggerButtonNames;
    std::vector<ButtonName> thumbButtonNames;
    std::vector<ButtonName> shoulderButtonNames;
    std::vector<ButtonName> dpadButtonNames;
    std::vector<ButtonName> genericButtonNames;

    SDLButtonMapping()
        : buttonMaps({ {ButtonName::DPAD_UP, ButtonMapInput()},
                      {ButtonName::DPAD_DOWN, ButtonMapInput()},
                      {ButtonName::DPAD_LEFT, ButtonMapInput()},
                      {ButtonName::DPAD_RIGHT, ButtonMapInput()},
                      {ButtonName::START, ButtonMapInput()},
                      {ButtonName::BACK, ButtonMapInput()},
                      {ButtonName::LEFT_THUMB, ButtonMapInput()},
                      {ButtonName::RIGHT_THUMB, ButtonMapInput()},
                      {ButtonName::LEFT_SHOULDER, ButtonMapInput()},
                      {ButtonName::RIGHT_SHOULDER, ButtonMapInput()},
                      {ButtonName::GUIDE, ButtonMapInput()},
                      {ButtonName::A, ButtonMapInput()},
                      {ButtonName::B, ButtonMapInput()},
                      {ButtonName::X, ButtonMapInput()},
                      {ButtonName::Y, ButtonMapInput()},
                      {ButtonName::LEFT_TRIGGER, ButtonMapInput()},
                      {ButtonName::RIGHT_TRIGGER, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_LEFT, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_UP, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_RIGHT, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_DOWN, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_LEFT, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_UP, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_RIGHT, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_DOWN, ButtonMapInput()} }),
        stickButtonNames(generateButtonList(ButtonType::STICK)),
        triggerButtonNames(generateButtonList(ButtonType::TRIGGER)),
        thumbButtonNames(generateButtonList(ButtonType::THUMB)),
        shoulderButtonNames(generateButtonList(ButtonType::SHOULDER)),
        dpadButtonNames(generateButtonList(ButtonType::HAT)),
        genericButtonNames(generateGenericButtonList()) {}

    std::vector<ButtonName> getSetButtonNames() const {
        std::vector<ButtonName> setButtons;
        for (const auto& buttonMap : buttonMaps) {
            if (buttonMap.second.input_type != ButtonType::UNSET) {
                setButtons.push_back(buttonMap.first);
            }
        }
        return setButtons;
    }

    std::vector<ButtonName> getUnsetButtonNames() const {
        std::vector<ButtonName> unsetButtons;
        for (const auto& buttonMap : buttonMaps) {
            if (buttonMap.second.input_type == ButtonType::UNSET) {
                unsetButtons.push_back(buttonMap.first);
            }
        }
        return unsetButtons;
    }


    static std::string getButtonNameString(ButtonName buttonName) {
        switch (buttonName) {
        case ButtonName::DPAD_UP: return "DPAD_UP";
        case ButtonName::DPAD_DOWN: return "DPAD_DOWN";
        case ButtonName::DPAD_LEFT: return "DPAD_LEFT";
        case ButtonName::DPAD_RIGHT: return "DPAD_RIGHT";
        case ButtonName::START: return "START";
        case ButtonName::BACK: return "BACK";
        case ButtonName::LEFT_THUMB: return "LEFT_THUMB";
        case ButtonName::RIGHT_THUMB: return "RIGHT_THUMB";
        case ButtonName::LEFT_SHOULDER: return "LEFT_SHOULDER";
        case ButtonName::RIGHT_SHOULDER: return "RIGHT_SHOULDER";
        case ButtonName::GUIDE: return "GUIDE";
        case ButtonName::A: return "A";
        case ButtonName::B: return "B";
        case ButtonName::X: return "X";
        case ButtonName::Y: return "Y";
        case ButtonName::LEFT_TRIGGER: return "LEFT_TRIGGER";
        case ButtonName::RIGHT_TRIGGER: return "RIGHT_TRIGGER";
        case ButtonName::LEFT_STICK_LEFT: return "LEFT_STICK_LEFT";
        case ButtonName::LEFT_STICK_UP: return "LEFT_STICK_UP";
        case ButtonName::LEFT_STICK_RIGHT: return "LEFT_STICK_RIGHT";
        case ButtonName::LEFT_STICK_DOWN: return "LEFT_STICK_DOWN";
        case ButtonName::RIGHT_STICK_LEFT: return "RIGHT_STICK_LEFT";
        case ButtonName::RIGHT_STICK_UP: return "RIGHT_STICK_UP";
        case ButtonName::RIGHT_STICK_RIGHT: return "RIGHT_STICK_RIGHT";
        case ButtonName::RIGHT_STICK_DOWN: return "RIGHT_STICK_DOWN";

        default: return "UNKNOWN";
        }
    }

    static std::string getButtonTypeString(ButtonType buttonType) {
        switch (buttonType) {
        case ButtonType::HAT: return "DPAD";
        case ButtonType::STICK: return "STICK";
        case ButtonType::THUMB: return "THUMB";
        case ButtonType::TRIGGER: return "TRIGGER";
        case ButtonType::BUTTON: return "BUTTON";
        case ButtonType::SHOULDER: return "SHOULDER";
        case ButtonType::UNSET: return "UNSET";
        default: return "UNKNOWN";
        }
    }


    std::string displayInput(ButtonName buttonName) {
        if (buttonMaps.count(buttonName) > 0) {
            const auto& buttonInput = buttonMaps.at(buttonName);
            return "Name: " + getButtonNameString(buttonName) + ", Input Type: " + getButtonTypeString(buttonInput.input_type) +
                ", Index: " + std::to_string(buttonInput.index) +
                ", Value: " + std::to_string(buttonInput.value);
        }
        else {
            return "Button not found: " + getButtonNameString(buttonName);
        }
    }

    static std::wstring displayInput(ButtonMapInput input) {
        if (input.input_type == SDLButtonMapping::ButtonType::UNSET)
            return L" (UNSET) ";
        std::string out = getButtonTypeString(input.input_type) + " " +
            std::to_string(input.index) + ": " +
            std::to_string(input.value);

        return g_toWide(out);
    }

private:

    std::vector<ButtonName> generateButtonList(ButtonType buttonType) const {

        auto compareByName = [this](ButtonName id1, ButtonName id2) {
            return getButtonNameString(id1) < getButtonNameString(id2);
        };

        std::vector<ButtonName> buttonList;
        for (const auto& buttonMap : buttonMaps) {
            const auto& buttonID = buttonMap.first;
            const std::string buttonName = getButtonNameString(buttonID);
            const auto& buttonInput = buttonMap.second;
            const auto bType = getButtonTypeString(buttonType);
            if (buttonName.find(bType) != std::string::npos) {
                buttonList.push_back(buttonID);
            }
        }
        if (buttonType != ButtonType::HAT) {
            // Sort the buttonList vector alphabetically
            std::sort(buttonList.begin(), buttonList.end());
        }
        return buttonList;
    }

    std::vector<ButtonName> generateGenericButtonList() const {
        // Sort the genericButtons vector by length using the custom comparator
        auto compareByNameLength = [this](ButtonName id1, ButtonName id2) {
            std::string name1 = getButtonNameString(id1);
            std::string name2 = getButtonNameString(id2);
            return name1.length() < name2.length();
        };

        std::vector<ButtonName> genericButtons;
        for (const auto& buttonMap : buttonMaps) {
            const auto& buttonID = buttonMap.first;
            const std::string buttonName = getButtonNameString(buttonID);
            const auto& buttonInput = buttonMap.second;
            if (buttonName.find("DPAD") == std::string::npos &&
                buttonName.find("SHOULDER") == std::string::npos &&
                buttonName.find("THUMB") == std::string::npos &&
                buttonName.find("TRIGGER") == std::string::npos &&
                buttonName.find("STICK") == std::string::npos) {
                genericButtons.push_back(buttonID);
            }
        }
        std::sort(genericButtons.begin(), genericButtons.end(), compareByNameLength);
        return genericButtons;
    }
};

///////
/// textUI 
//////
#include "./../JoySender_tUI/tUI/textUI.h"

void setErrorMsg(const wchar_t* text, size_t length);
void updateFPS(const wchar_t* text, size_t length);
void exitAppCallback(mouseButton& button);
void newControllerColorsCallback(mouseButton& button);
void checkForQuit();
bool checkKey(int key, bool pressed);
int screenLoop(textUI& screen);
void loopCount(int& counter, int maxCount);
void countUpDown(int& counter, int maxCount);

#pragma warning(disable : 4996)
// Converts strings to wide strings and back again
std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

int g_mode = 1;
int g_currentColorScheme;
ColorScheme g_simpleScheme;

wchar_t g_stringBuff[500];      // for holding generated text data
wchar_t* errorPointer = g_stringBuff;   // max length 100
wchar_t* clientPointer = g_stringBuff + 100;    // max length 25
wchar_t* fpsPointer = clientPointer + 25;   // max length 10
wchar_t* msgPointer1 = fpsPointer + 10;   // max length 100
wchar_t* msgPointer2 = msgPointer1 + 100;   // max length 100
wchar_t* msgPointer3 = msgPointer2 + 100;   // max length 165

textBox errorOut(4, 22, 45, 0, ALIGN_CENTER, errorPointer, BRIGHT_RED);
textBox clientMsg(1, 8, 20, 0, ALIGN_LEFT, clientPointer, BRIGHT_GREEN);
textBox fpsMsg(1, 9, 20, 0, ALIGN_LEFT, fpsPointer, BRIGHT_CYAN);
textBox output1(3, 19, 60, 0, ALIGN_LEFT, msgPointer1, BRIGHT_BLUE);
textBox output2(3, 21, 60, 0, ALIGN_LEFT, msgPointer2, BRIGHT_BLUE);
textBox output3(3, 24, 60, 0, ALIGN_LEFT, msgPointer3, BRIGHT_BLUE);

//
// text buffer setting functions

void setErrorMsg(const wchar_t* text, size_t length) {
    wcsncpy_s(errorPointer, length, text, _TRUNCATE);
    errorOut.SetText(errorPointer);
}

void updateFPS(const wchar_t* text, size_t length) {
    wcsncpy_s(fpsPointer, length, text, _TRUNCATE);
    fpsMsg.SetText(fpsPointer);
    fpsMsg.Draw();
}

 
// ********************************
//  mouseButton callback functions

// callback links buttons status with _partner
void ShareButtonStatusCallback(mouseButton& button) {
    auto other = button.GetPartner();
    other->SetStatus(button.Status());
    other->Update();
}

// used by quit button, sets APP_KILLED to true
void exitAppCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        APP_KILLED = true;
    }
}

// will generate a new color scheme for program and controller face
void newControllerColorsCallback(mouseButton& button) {
    
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);

        WORD newRandomBG = generateRandomInt(0, 15) << 4;

        // update globals for universal color consistency
        g_simpleScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(g_simpleScheme, newRandomBG);
        g_currentColorScheme = 0;

        // Draw Controller
        DrawControllerFace(g_screen, g_simpleScheme, newRandomBG, g_mode);

        tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[RANDOMSCHEME].controllerColors);

        // non controller buttons
        g_screen.SetButtonsColors(buttonColors);
        g_screen.DrawButtons();

        // on screen text 
        output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);
        clientMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
        fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);

        // draw messages
        output1.Draw();
        clientMsg.Draw();
    }
}


// ********************************
// tUI Helper Functions

// used to display current DS4 inputs via on-screen controller face
void buttonStatesFromDS4Report(BYTE* ds4_report) {
    byte shoulderRedraw = 0;  // 0x01: left, 0x08: right
    auto updateButtonStatus = [&](mouseButton& button, bool condition, byte special = 0) {
        if (condition && button.Status() == MOUSE_OUT) {
            button.SetStatus(MOUSE_HOVERED | MOUSE_DOWN);
            if (!special)
                button.Update();
            else
                shoulderRedraw |= special;
        }
        else if (!condition && button.Status() == (MOUSE_HOVERED | MOUSE_DOWN)) {
            button.SetStatus(MOUSE_OUT);
            if (!special)
                button.Update();
            else
                shoulderRedraw |= special;
        }
    };


    // Left Stick
    updateButtonStatus(button_LStickLeft_highlight, ds4_report[0] < 100);   // left
    updateButtonStatus(button_LStickRight_highlight, ds4_report[0] > 156);   // right

    ds4_report++;
    updateButtonStatus(button_LStickDown_highlight, ds4_report[0] > 156);   // down
    updateButtonStatus(button_LStickUp_highlight, ds4_report[0] < 100);   // up

        // Right Stick
    ds4_report++;
    updateButtonStatus(button_RStickLeft_highlight, ds4_report[0] < 100);   // left
    updateButtonStatus(button_RStickRight_highlight, ds4_report[0] > 156);   // right

    ds4_report++;
    updateButtonStatus(button_RStickDown_highlight, ds4_report[0] > 156);   // down
    updateButtonStatus(button_RStickUp_highlight, ds4_report[0] < 100);   // up

    ds4_report++;
    // Main Face Buttons
    updateButtonStatus(button_A_outline, ds4_report[0] & 0x20);   // A
    updateButtonStatus(button_B_outline, ds4_report[0] & 0x40);    // B
    updateButtonStatus(button_X_outline, ds4_report[0] & 0x10);    // X
    updateButtonStatus(button_Y_outline, ds4_report[0] & 0x80);    // Y

        // DPAD
    int dval = ds4_report[0] & 0x0F;
    updateButtonStatus(button_DpadUp_outline, (dval == 7 || dval == 1 || dval == 0));
    updateButtonStatus(button_DpadRight_outline, abs(dval - 2) < 2);
    updateButtonStatus(button_DpadDown_outline, abs(dval - 4) < 2);
    updateButtonStatus(button_DpadLeft_outline, abs(dval - 6) < 2);

    ds4_report++;
    // Shoulders: L1 / R1
    updateButtonStatus(button_L1_highlight, ds4_report[0] & 0x01, 0x01);   // left shoulder
    updateButtonStatus(button_R1_highlight, ds4_report[0] & 0x02, 0x08);   // right shoulder

        // Triggers: L2 / R2
    updateButtonStatus(button_L2_highlight, ds4_report[0] & 0x04, 0x01);   // left trigger
    updateButtonStatus(button_R2_highlight, ds4_report[0] & 0x08, 0x08);   // right trigger

        // Share and Options buttons
    updateButtonStatus(button_Back_highlight, ds4_report[0] & 0x10);    // back
    updateButtonStatus(button_Start_highlight, ds4_report[0] & 0x20);   // start

        // Thumbs: L3 / R3
    updateButtonStatus(button_L3_highlight, ds4_report[0] & 0x40);   // left thumb
    updateButtonStatus(button_R3_highlight, ds4_report[0] & 0x80);   // right thumb

    ds4_report++;
    // PS & T-Pad 
    updateButtonStatus(button_Guide_highlight, ds4_report[0] & 0x03);  // guide


    // must redraw both buttons in this order for highlighting to always be correct 
    //* except there are still minor issues
    if (shoulderRedraw & 0x01) {
        button_L1_highlight.Update();
        button_L2_highlight.Update();
    }
    if (shoulderRedraw & 0x08) {
        button_R1_highlight.Update();
        button_R2_highlight.Update();
    }
}

// used to display current joystick inputs via on-screen controller face
void buttonStatesFromXboxReport(XUSB_REPORT& xboxReport) {
    byte shoulderRedraw = 0;  // 0x01: left, 0x08: right
    auto updateButtonStatus = [&](mouseButton& button, bool condition, byte special = 0) {
        if (condition && button.Status() == MOUSE_OUT) {
            button.SetStatus(MOUSE_HOVERED | MOUSE_DOWN);
            if (!special)
                button.Update();
            else
                shoulderRedraw |= special;
        }
        else if (!condition && button.Status() == (MOUSE_HOVERED | MOUSE_DOWN)) {
            button.SetStatus(MOUSE_OUT);
            if (!special)
                button.Update();
            else
                shoulderRedraw |= special;
        }
    };

    // Left Stick
    updateButtonStatus(button_LStickLeft_highlight, xboxReport.sThumbLX < -AXIS_INPUT_DEADZONE);   // left
    updateButtonStatus(button_LStickRight_highlight, xboxReport.sThumbLX > AXIS_INPUT_DEADZONE);   // right
    updateButtonStatus(button_LStickDown_highlight, xboxReport.sThumbLY < -AXIS_INPUT_DEADZONE);   // down
    updateButtonStatus(button_LStickUp_highlight, xboxReport.sThumbLY > AXIS_INPUT_DEADZONE);   // up

    // Right Stick
    updateButtonStatus(button_RStickLeft_highlight, xboxReport.sThumbRX < -AXIS_INPUT_DEADZONE);   // left
    updateButtonStatus(button_RStickRight_highlight, xboxReport.sThumbRX > AXIS_INPUT_DEADZONE);   // right
    updateButtonStatus(button_RStickDown_highlight, xboxReport.sThumbRY < -AXIS_INPUT_DEADZONE);   // down
    updateButtonStatus(button_RStickUp_highlight, xboxReport.sThumbRY > AXIS_INPUT_DEADZONE);   // up

    // Shoulders: L1 / R1
    updateButtonStatus(button_L1_highlight, xboxReport.wButtons & XUSB_GAMEPAD_LEFT_SHOULDER, 0x01);   // left shoulder
    updateButtonStatus(button_R1_highlight, xboxReport.wButtons & XUSB_GAMEPAD_RIGHT_SHOULDER, 0x08);   // right shoulder

    // Triggers: L2 / R2
    updateButtonStatus(button_L2_highlight, xboxReport.bLeftTrigger > 0, 0x01);   // left trigger
    updateButtonStatus(button_R2_highlight, xboxReport.bRightTrigger > 0, 0x08);   // right trigger

    // Thumbs: L3 / R3
    updateButtonStatus(button_L3_highlight, xboxReport.wButtons & XUSB_GAMEPAD_LEFT_THUMB);   // left thumb
    updateButtonStatus(button_R3_highlight, xboxReport.wButtons & XUSB_GAMEPAD_RIGHT_THUMB);   // right thumb

    // DPAD
    updateButtonStatus(button_DpadUp_outline, xboxReport.wButtons & XUSB_GAMEPAD_DPAD_UP);
    updateButtonStatus(button_DpadDown_outline, xboxReport.wButtons & XUSB_GAMEPAD_DPAD_DOWN);
    updateButtonStatus(button_DpadLeft_outline, xboxReport.wButtons & XUSB_GAMEPAD_DPAD_LEFT);
    updateButtonStatus(button_DpadRight_outline, xboxReport.wButtons & XUSB_GAMEPAD_DPAD_RIGHT);

    // Buttons
    updateButtonStatus(button_Start_highlight, xboxReport.wButtons & XUSB_GAMEPAD_START);  // start
    updateButtonStatus(button_Back_highlight, xboxReport.wButtons & XUSB_GAMEPAD_BACK);    // back
    updateButtonStatus(button_Guide_highlight, xboxReport.wButtons & XUSB_GAMEPAD_GUIDE);   // guide

    // Main Face Buttons
    updateButtonStatus(button_A_outline, xboxReport.wButtons & XUSB_GAMEPAD_A);   // A
    updateButtonStatus(button_B_outline, xboxReport.wButtons & XUSB_GAMEPAD_B);   // B
    updateButtonStatus(button_X_outline, xboxReport.wButtons & XUSB_GAMEPAD_X);   // X
    updateButtonStatus(button_Y_outline, xboxReport.wButtons & XUSB_GAMEPAD_Y);   // Y

    // must redraw both buttons in this order for highlighting to always be correct 
    //* except there are still minor issues
    if (shoulderRedraw & 0x01) {
        button_L1_highlight.Update();
        button_L2_highlight.Update();
    }
    if (shoulderRedraw & 0x08) {
        button_R1_highlight.Update();
        button_R2_highlight.Update();
    }
}

// to allow for screen loop and animation, wait for connection to client in a separate thread
void threadedAwaitConnection(TCPConnection& server, int& retVal, char* clientIP) {
    
    //
    // Await Connection in Non Blocking Mode
    server.set_server_blocking(false);
    std::pair<SOCKET, sockaddr_in> connectionResult;
    SOCKET clientSocket;
    while (!APP_KILLED && retVal == WSAEWOULDBLOCK) {
        // Attempt to accept a client connection in non Blocking mode
        connectionResult = server.await_connection();
        clientSocket = connectionResult.first;
        if (clientSocket == INVALID_SOCKET) {
            retVal = WSAGetLastError();
            if (retVal == WSAEWOULDBLOCK) {
                // No client connection is immediately available
                Sleep(10); // Sleep for 10 milliseconds
            }
            else if (retVal == WSAEINVAL) {
                // Invalid argument error * seems to trigger when we don't have access to the specified port
                APP_KILLED = true;
                break;
            }
            else {
                // An error occurred during accept()
                //std::cerr << "Failed to accept client connection: " << retVal << std::endl;
                retVal = -1;
                break; // Exit the loop or handle the error condition
            }
        }
        else {
            // A client connection is established
            // Process the connection
                // set clientIP
            sockaddr_in clientAddress = connectionResult.second;           
            inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIP, INET_ADDRSTRLEN);

                //set retVal to break loop
            retVal = 1;

            // Create a new thread or perform other operations on the clientSocket?

            // Break the loop, or this will continue attempting to accept more connections
            break;
        }
    }
}

// for smooth and controllable animation advance frames in a separate thread
// not currently used
void threadedFrameAdvance(int& run, int delay, int& counter, int maxCount) {
    while(!APP_KILLED && run){
        Sleep(delay);
        loopCount(counter, maxCount);
    }
}
