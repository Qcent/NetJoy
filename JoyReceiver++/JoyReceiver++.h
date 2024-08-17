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
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>
#include <codecvt>
#include <thread>
#include <mutex>
std::mutex mtx;  // Mutex for synchronizing access to shared variables
std::string feedbackData; // For sending rumble data back to joySender

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

#define APP_VERSION_NUM     L"1.0.1.0"

volatile sig_atomic_t APP_KILLED = 0;
std::string g_outputText;

void signalHandler(int signal);
std::vector<std::string> split(const std::string& str, char delimiter);


// Define the callback functions for Rumble Support 
VOID CALLBACK xbox_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target, 
    UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData){
#if 0
    static int count = 1;
    repositionConsoleCursor(-2, 6);
    std::cout << "Rumble Calls:" << count++ << "\t ";
    repositionConsoleCursor(0, 22);
    std::cout << "LargeMotor:" << (int)LargeMotor << "   ";
    repositionConsoleCursor(0, 37);
    std::cout << "SmallMotor:" << (int)SmallMotor << "   " << "   ";
    repositionConsoleCursor(2);
#endif
    std::string& feedbackData = *reinterpret_cast<std::string*>(UserData);
    // Set the UserData with the desired value
    feedbackData = static_cast<char>(LargeMotor);
    feedbackData += static_cast<char>(SmallMotor);
}

VOID CALLBACK ds4_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor,
    UCHAR SmallMotor, DS4_LIGHTBAR_COLOR LightbarColor, LPVOID UserData){
#if 0
    repositionConsoleCursor(-2, 3);
    std::cout << "Rumble Data: ";
    std::cout << "L:" << (int)LargeMotor << "  ";
    repositionConsoleCursor(0, 22);
    std::cout << "R:" << (int)SmallMotor << "  ";
    repositionConsoleCursor(0, 28);
    std::cout << "Color: " << (int)LightbarColor.Red << "  ";
    repositionConsoleCursor(0, 38);
    std::cout << " | " << (int)LightbarColor.Green << "  ";
    repositionConsoleCursor(0, 44);
    std::cout << " | " << (int)LightbarColor.Blue << "        ";
    repositionConsoleCursor(2);
#endif
    std::string& feedbackData = *reinterpret_cast<std::string*>(UserData);
    // Set the UserData with the desired value
    feedbackData = static_cast<char>(LargeMotor);
    feedbackData += static_cast<char>(SmallMotor);

    feedbackData += static_cast<char>(LightbarColor.Red);
    feedbackData += static_cast<char>(LightbarColor.Green);
    feedbackData += static_cast<char>(LightbarColor.Blue);
};


std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
std::string formatDecimalString(const std::string& str, UINT8 numDigits) {
    size_t decimalPos = str.find('.');
    if (decimalPos == std::string::npos) {
        // No decimal point found, return the original 
        // string with decimal and padded zeros
        std::string paddedStr = " " + str + ".";
        paddedStr.append(numDigits, '0');
        return paddedStr;
    }
    size_t strLen = str.length();
    size_t numDecimals = strLen - decimalPos - 1;
    if (numDecimals <= numDigits) {
        // No need to truncate or pad, return the original string
        return str;
    }
    // Truncate the string to the desired number of decimal places
    std::string truncatedStr = " " + str.substr(0, decimalPos + numDigits + 1);
    // Pad with zeros if necessary
    if (numDecimals < numDigits) {
        truncatedStr.append(numDigits - numDecimals, '0');
    }

    return truncatedStr;
}

// Function safely return environment variables
std::string g_getenv(const char* variableName) {
    std::string value;
    size_t requiredSize = 0;
    getenv_s(&requiredSize, nullptr, 0, variableName);
    if (requiredSize > 0) {
        value.resize(requiredSize);
        getenv_s(&requiredSize, &value[0], requiredSize, variableName);
        value.pop_back(); // Remove the null-terminating character
    }
    return value;
}
// Function captures all input from the keyboard to clear buffer
void swallowInput()
{
    while (_kbhit())
    {
        _getch();
    }
}
// Function to detect if keyboard key is pressed
bool getKeyState(int KEYCODE) {
    return GetAsyncKeyState(KEYCODE) & 0x8000;
}
// Function determines if app is the active window
bool IsAppActiveWindow()
{
    HWND consoleWindow = GetConsoleWindow();
    HWND foregroundWindow = GetForegroundWindow();

    return (consoleWindow == foregroundWindow);
}
// Function to convert strings to wide strings
std::wstring g_toWide(std::string& str) {
    std::wstring wideString(str.begin(), str.end());
    return wideString;
}


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
/// tUI.h 
//////
#include "tUI/textUI.h"

const int consoleWidth = 72;
const int consoleHeight = 20;

HANDLE g_hConsoleInput;
textUI g_screen;

void setErrorMsg(const wchar_t* text, size_t length);
void updateFPS(const wchar_t* text, size_t length);
void exitAppCallback(mouseButton& button);
void newControllerColorsCallback(mouseButton& button);
void checkForQuit();
bool checkKey(int key, bool pressed);
int screenLoop(textUI& screen);

void loopCount(int& counter, int maxCount);
void countUpDown(int& counter, int maxCount);

#define IS_PRESSED      1
#define IS_RELEASED     0
std::unordered_map<int, bool> g_keyStateMap;
// function checks for KEYCODE state & logs to dict. returns true when state change mirrors the bool pressed
bool checkKey(int key, bool pressed) {
    bool state = GetAsyncKeyState(key) & 0x8000;
    if (state != g_keyStateMap[key]) {
        g_keyStateMap[key] = state;
        if (state) {
            // key is pressed
            if (pressed)
                return 1;
        }
        else {
            // key is released
            if (!pressed)
                return 1;
        }
    }
    return 0;
}

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
// FullColourScheme

// Package up all the colors needed for maximum color expression
struct FullColorScheme {
    const wchar_t* name;
    tUIColorPkg menuColors;
    tUIColorPkg controllerColors;
    WORD menuBg;
    WORD controllerBg;
};

#define NUM_COLOR_SCHEMES   5   // number of defined color schemes + a place for a random scheme
#define RANDOMSCHEME        0   // index to store a randomly generated color scheme

FullColorScheme fullColorSchemes[NUM_COLOR_SCHEMES + 1] = {
    { L"Random",
        {
        // Menu Colors
        WHITE | (MAGENTA AS_BG),			// Header
        WHITE | (RED AS_BG),		        // Error
        WHITE | (CYAN AS_BG),				// message 1
        WHITE | (BRIGHT_RED AS_BG)			// message 2
    },
    {
        // Controller Colors
        BLACK | (BRIGHT_MAGENTA AS_BG),	    // outline | faceColor 
        WHITE | (BRIGHT_MAGENTA AS_BG),		// buttonColor
        BLUE | (BRIGHT_MAGENTA AS_BG),	    // highlightColor
        WHITE | (CYAN AS_BG)	            // selectColor
    },
    // Menu background
    WHITE | (BRIGHT_RED AS_BG),
    // Controller background
    BLACK | (MAGENTA AS_BG),
},


{ L"Plum",
    {
        // Menu Colors
        WHITE | (MAGENTA AS_BG),			// Header
        WHITE | (RED AS_BG),		        // Error
        WHITE | (CYAN AS_BG),				// message 1
        WHITE | (BRIGHT_RED AS_BG)			// message 2
    },
        {
            // Controller Colors
            BLACK | (BRIGHT_MAGENTA AS_BG),	    // outline | faceColor 
            WHITE | (BRIGHT_MAGENTA AS_BG),		// buttonColor
            BLUE | (BRIGHT_MAGENTA AS_BG),	    // highlightColor
            WHITE | (CYAN AS_BG)	            // selectColor
        },
    // Menu background
    WHITE | (BRIGHT_RED AS_BG),
    // Controller background
    BLACK | (MAGENTA AS_BG),
},


 { L"Watermelon",
    {
        // Menu Colors
        BLACK | (GREEN AS_BG),			// Header
        WHITE | (RED AS_BG),		    // Error
        BLACK | (BRIGHT_CYAN AS_BG),	// message 1
        BLUE | (BRIGHT_GREEN AS_BG)		// message 2
    },
        {
            // Controller Colors
            BRIGHT_GREEN | (BRIGHT_RED AS_BG),	// outline | faceColor 
            BLACK | (BRIGHT_RED AS_BG),		    // buttonColor
            BRIGHT_GREEN | (BRIGHT_RED AS_BG),  // highlightColor
            BLACK | (BRIGHT_GREEN AS_BG)        // selectColor
        },
    // Menu background
    BLACK | (CYAN AS_BG),
    // Controller background
    BRIGHT_GREEN | (BRIGHT_BLUE AS_BG),
},


 { L"Blueberry",
    {
        // Menu Colors
        WHITE | (BRIGHT_MAGENTA AS_BG),		// Header
        WHITE | (RED AS_BG),		        // Error
        BLUE | (BRIGHT_CYAN AS_BG),			// message 1
        BRIGHT_CYAN | (MAGENTA AS_BG)		// message 2
    },
        {
            // Controller Colors
            CYAN | (CYAN AS_BG),	        // outline | faceColor 
            WHITE | (CYAN AS_BG),		    // buttonColor
            BLUE | (CYAN AS_BG),            // highlightColor
            BRIGHT_CYAN | (BRIGHT_MAGENTA AS_BG)  // selectColor
        },
    // Menu background
    CYAN | (MAGENTA AS_BG),
    // Controller background
    CYAN | (BLUE AS_BG),
},


{ L"Citrus",
   {
        // Menu Colors
        BRIGHT_BLUE | (BRIGHT_GREEN AS_BG),	// Header
        WHITE | (RED AS_BG),		        // Error
        BLACK | (BRIGHT_CYAN AS_BG),	// message 1
        WHITE | (CYAN AS_BG)			    // message 2
    },
    {
        // Controller Colors
        BRIGHT_GREEN | (BRIGHT_GREEN AS_BG),	// outline | faceColor 
        BLUE | (BRIGHT_GREEN AS_BG),		    // buttonColor
        WHITE | (BRIGHT_GREEN AS_BG),           // highlightColor
        WHITE | (YELLOW AS_BG)      // selectColor
    },
    // Menu background
    BRIGHT_BLUE | (BRIGHT_GREEN AS_BG),
    // Controller background
    BRIGHT_GREEN | (BRIGHT_RED AS_BG),
},


{ L"Emily's Bougie Baby",
    {
        // Menu Colors
        BLACK | (YELLOW AS_BG),			// Header
        WHITE | (RED AS_BG),		    // Error
        WHITE | (BLUE AS_BG),			// message 1
        BLUE | (BLACK AS_BG)			// message 2
    },
        {
            // Controller Colors
            YELLOW | (BLACK AS_BG),	        // outline | faceColor 
            YELLOW | (BLACK AS_BG),		    // buttonColor
            BRIGHT_YELLOW | (BLACK AS_BG),  // highlightColor
            WHITE | (YELLOW AS_BG)  // selectColor
        },
    // Menu background
    RED | (BLACK AS_BG),
    // Controller background
    YELLOW | (BLACK AS_BG),
}

};

// takes a color package representing controller button colors and converts to screen button colors for mouse hovering
tUIColorPkg controllerButtonsToScreenButtons(tUIColorPkg& inButs) {
    return tUIColorPkg(
        inButs.col2,    // default
        inButs.col3,    // highlight
        inButs.col4,    // select
        inButs.col3     // active // used for ip input
    );
}

// converts a simple color scheme to a full color scheme *random color scheme generation can be converted to a full scheme
FullColorScheme fullSchemeFromSimpleScheme(ColorScheme& simp, WORD bg) {
    FullColorScheme ret = {
        L"Random",
        {
            // Menu Colors
            BLACK | (GREY AS_BG),			// Header
            BLACK | (RED AS_BG),		        // Error
            BLACK | (BRIGHT_GREY AS_BG),		// message 1
            BLACK | (WHITE AS_BG)			// message 2
        },
        {
            // Controller Colors
            static_cast<WORD>(simp.outlineColor | simp.faceColor),	    // outline | faceColor 
            static_cast<WORD>(simp.buttonColor | simp.faceColor),		// buttonColor
            static_cast<WORD>(simp.highlightColor | simp.faceColor),	    // highlightColor
            simp.selectColor                         // selectColor 
        },
        // Menu background
        BLACK | (WHITE AS_BG),
        // Controller background
        static_cast<WORD>(simp.outlineColor | bg)
    };

    return ret;
}

// converts a a full color scheme to a simple scheme *needed for draw and redraw controller face functions
ColorScheme simpleSchemeFromFullScheme(FullColorScheme& full) {
    return ColorScheme{
        static_cast<WORD>(full.controllerColors.col1 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col1 BG_ONLY),
        static_cast<WORD>(full.controllerColors.col2 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col3 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col4),
        L" "
    };
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

// checks if the Q button has been pressed and sets APP_KILLED to true
void checkForQuit() {
    if (IsAppActiveWindow() && checkKey('Q', IS_PRESSED) && getKeyState(VK_SHIFT)) // Q for Quit
        APP_KILLED = true;
}

// used in a loop, looks for mouse input and compares to buttons on screen
int screenLoop(textUI& screen) {
    int retVal = 0;
    DWORD eventsAvailable;
    GetNumberOfConsoleInputEvents(g_hConsoleInput, &eventsAvailable);

    if (eventsAvailable > 0) {
        INPUT_RECORD inputRecord;
        DWORD eventsRead;

        if (!ReadConsoleInput(g_hConsoleInput, &inputRecord, 1, &eventsRead)) {
            return -1;
        }

        // Check for mouse input events
        if (inputRecord.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD& mouseEvent = inputRecord.Event.MouseEvent;
            COORD mousePos = mouseEvent.dwMousePosition;

            if (mouseEvent.dwEventFlags == MOUSE_MOVED) {
                // Check if mouse is hovering over the buttons
                screen.CheckMouseHover(mousePos);
            }

            if (mouseEvent.dwEventFlags == 0) {
                // Mouse button event
                if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                    screen.CheckMouseClick(MOUSE_DOWN);
                    retVal = MOUSE_DOWN;
                }
                else {
                    screen.CheckMouseClick(MOUSE_UP);
                    retVal = MOUSE_UP;
                }
            }
        }
    }
    return retVal;
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


///  
/// ANIMATIONS
///  
int g_frameNum = 0;  //
const int CX_ANI_FRAME_COUNT = 12;

const wchar_t* ConnectAnimationLeft[CX_ANI_FRAME_COUNT + 1] = {
    {L"     "},
    {L"   . "},
    {L"  .. "},
    {L" ... "},
    {L" ... "},
    {L"  .. "},
    {L"   . "},
    {L"     "},
    {L" * * "},
    {L"  *  "},
    {L" * * "},
    {L"  *  "},
    {L" *** "},
};

const wchar_t* ConnectAnimationRight[CX_ANI_FRAME_COUNT + 1] = {
    {L"     "},
    {L" .   "},
    {L" ..  "},
    {L" ... "},
    {L" ... "},
    {L" ..  "},
    {L" .   "},
    {L"     "},
    {L" * * "},
    {L"  *  "},
    {L" * * "},
    {L"  *  "},
    {L" *** "},
};

const int FOOTER_ANI_FRAME_COUNT = 9;
const wchar_t* FooterAnimation[CX_ANI_FRAME_COUNT + 1] = {
{L" .--.     :::::.\\:::'      `--"},
{L".--.      ::::.\\::::      `--'"},
{L"--.      .:::.\\:::::     `--' "},
{L"-.      .-::.\\::::::    `--'  "},
{L".      .--:.\\:::::::   `--'   "},
{L"      .--..\\::::::::  `--'    "},
{L"     .--. \\::::::::. `--'     "},
{L"    .--.  ::::::::.\\`--'      "},
{L"   .--.   :::::::.\\:--'      `"},
{L"  .--.    ::::::.\\::-'      `-"},
};

// function will inc/dec &counter up to maxCount and down to 0
// for animating back and forth through frames
void countUpDown(int& counter, int maxCount) { // * a counter value outside of (0 - maxCount) will cause issues
    static int dir = 1; // 1: up, -1: down

    // this non intuitive block makes sure we always count up from 0
    if (!counter) {
        dir = -1;
    }

    // if 0 or maxCount, direction of counting will be reversed
    if (!(counter < maxCount && counter > 0)) {
        dir *= -1;
    }

    counter += dir; // count by one in direction +1 or -1
}

// function will loop from 0 to maxCount
// for animation cyclically through frames
void loopCount(int& counter, int maxCount) {
    counter = (counter + 1) % (maxCount + 1);
}

// function will loop from maxCount to 0
// for animation cyclically through frames
void revLoopCount(int& counter, int maxCount) {
    --counter;
    if (counter < 0)
        counter = maxCount-1;
}