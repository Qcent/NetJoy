#pragma once
#include <iostream>
#include <conio.h>
#include <csignal>
#include <cwchar>
#include <codecvt>
#include <wchar.h>
#include <thread>

#include "TCPConnection.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

// GLOBAL VARIABLES
constexpr auto APP_NAME = "NetJoy";

std::string g_outputText;
int g_joystickSelected = -1;

volatile sig_atomic_t APP_KILLED = 0;
void signalHandler(int signal);

int RESTART_FLAG = 0;
int MAPPING_FLAG = 0;

// Function safely returns environment variables
std::string g_getenv(const char* variableName);

// Function to detect if keyboard key is pressed
bool getKeyState(int KEYCODE);
// Function to convert strings to wide strings
std::wstring g_toWide(std::string& str);
// Function that will prompt for and verify an ip address 
std::string getHostAddress();
// Function that takes string representing a float and fixes it's decimal places to numDigits
std::string formatDecimalString(const std::string& str, UINT8 numDigits);
// Function captures all input from the keyboard to clear buffer
void swallowInput();
// Function waits for no keyboard presses to be detected before returning
void wait_for_no_keyboard_input();
// Function determines if app if the active window
bool IsAppActiveWindow();
// Function determines if rumble values need to be updated
bool updateRumble(const char motor, BYTE val);
// Main Loop Function 
int joySender(Arguments& args);

#include "GamepadMapping.hpp"
#include "DS4Manager.hpp"

///////
/// tUI.h 
//////
#include "tUI/textUI.h"

void setErrorMsg(const wchar_t* text, size_t length);
void updateFPS(const wchar_t* text, size_t length);
void exitAppCallback(mouseButton& button);
void joystickSelectCallback(mouseButton& button);
void newControllerColorsCallback(mouseButton& button);
void printCurrentController(SDLJoystickData& activeGamepad);
void checkForQuit();
bool checkKey(int key, bool pressed);
char IpInputLoop();
int screenLoop(textUI& screen);
int tUISelectJoystickDialog(int numJoysticks, SDLJoystickData& joystick, textUI& screen);
int tUISelectDS4Dialog(std::vector<HidDeviceInfo>& devList, textUI& screen);
bool tUIConnectToDS4Controller(textUI& screen);
std::string tUIGetHostAddress(textUI& screen);
void threadedEstablishConnection(TCPConnection& client, int& retVal);
bool checkKey(int key, bool pressed);
tUIColorPkg colorSchemeToColorPkg(int version = 0);
void tUIMapTheseInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>& inputList);
int tUIRemapInputsScreen(SDLJoystickData& joystick, textUI& screen);


#define MAP_BUTTON_CLICKED 10000  // a value to add to an input index to indicate it was clicked on

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

const int consoleWidth = 72;
const int consoleHeight = 24;

#pragma warning(disable : 4996)
// Converts strings to wide strings and back again
std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

int g_mode = 1;
int g_currentColorScheme;
WORD g_BG_COLOR = 0;

wchar_t g_stringBuff[500];
wchar_t* errorPointer = g_stringBuff;   // max length 100
wchar_t* hostPointer = g_stringBuff + 100;    // max length 25
wchar_t* fpsPointer = hostPointer + 25;   // max length 10
wchar_t* msgPointer1 = fpsPointer + 10;   // max length 100
wchar_t* msgPointer2 = msgPointer1 + 100;   // max length 100
wchar_t* msgPointer3 = msgPointer2 + 100;   // max length 165

HANDLE g_hConsoleInput;

textUI g_screen;
textBox errorOut(4, 22, 45, 0, ALIGN_CENTER, errorPointer, BRIGHT_RED);

textBox hostMsg(1, 8, 20, 0, ALIGN_LEFT, hostPointer, BRIGHT_GREEN);
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

 // experimental global to extend callback functionality without redesigning the entire class and API up to this point
void* g_extraData = nullptr;

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

// for selecting joystick by mouse click, will set g_joystickSelected to button._id
void joystickSelectCallback(mouseButton& button) {
if (button.Status() & MOUSE_UP) {
    button.SetStatus(MOUSE_OUT);
    g_joystickSelected = button.GetId();
}
}

// will generate a new color scheme for program and controller face
void newControllerColorsCallback(mouseButton& button) {
if (button.Status() & MOUSE_UP) {
    button.SetStatus(MOUSE_OUT);

    WORD newRandomBG = generateRandomInt(0, 15) << 4;

    static ColorScheme randomScheme;
    randomScheme = createRandomScheme();
    g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES);

    if (generateRandomInt(0, 1001) % 2 == 0) {
        randomScheme = colorSchemes[g_currentColorScheme];
    }
    else {
        g_currentColorScheme = -1;
    }


    // force watermelon scheme for now
    g_currentColorScheme = 16;
    randomScheme = colorSchemes[16];
    newRandomBG = BRIGHT_BLUE AS_BG;
    
    // update global background for universal color consistency
    g_BG_COLOR = newRandomBG | randomScheme.outlineColor;


    if (g_mode == 2) {
        BuildDS4Face();
        SetControllerButtonPositions(g_mode);
        quitButton.SetPosition(consoleWidth / 2 - 5, 17);
    }
    else { // testing only XBOX for now no need to redo this 
        // BuildXboxFace();
        // SetButtonPositions(g_mode);
        // quitButton.SetPosition(consoleWidth / 2 - 5, 18);
    }

    // Draw Controller
    DrawControllerFace(g_screen, randomScheme, newRandomBG, g_mode);

    tUIColorPkg buttonColors(
        g_screen.GetBackdropColor(),
        button_Guide_highlight.getHighlightColor(),
        button_Guide_highlight.getSelectColor(),
        0 // unused
    );
    //


    // non controller buttons
    restartButton[3].SetColors(buttonColors);  // mode section of restart button
    g_screen.SetButtonsColors(buttonColors);
    g_screen.DrawButtons();
}
}

// handles status and color changes to the UI restart button array for restartButton[1] & [2]
void restartModeCallback(mouseButton& button) {
static int lastStatus = 0;
static std::wstring modeCheckValue(L"1");

// if the status has not changed do nothing
if (button.Status() == lastStatus) return;


if (button.Status() & (MOUSE_HOVERED)) {

    // Get mode value from button text
    std::wstring modetxt = button.getTextPtr();

    // if mouse released on number restart in that mode
    if (button.Status() & MOUSE_UP) {
        /*
        if (modetxt == modeCheckValue)
            RESTART_FLAG = 2;
        else
            RESTART_FLAG = 3;
        */
        RESTART_FLAG = (modetxt != modeCheckValue) + 2;  // non branching

        for (int i = 0; i < 3; ++i)
            restartButton[i].SetStatus(MOUSE_OUT);

        return;
    }


    // create new text for button
    std::wstring modeText = L"[mode " + modetxt + L"] ";
    // store text in msgPointer3 
    swprintf(msgPointer3, modeText.size() + 1, L"%s", modeText.c_str());
    // update button text
    restartButton[3].SetText(msgPointer3);


    // correct button colors
    WORD correctColor;

    if (button.Status() & MOUSE_DOWN || restartButton[0].Status() & MOUSE_DOWN)
        correctColor = restartButton[0].getCurrentColor();
    else
        correctColor = button.getCurrentColor();


    restartButton[3].Draw(correctColor);

    /*
    if (modetxt == modeCheckValue) {
        restartButton[2].Draw(correctColor);
    }
    else {
        restartButton[1].Draw(correctColor);
    }
    */
    restartButton[(modetxt == modeCheckValue) + 1].Draw(correctColor); // non branching, yay!

    restartButton[1].SetDefaultColor(correctColor);
    restartButton[2].SetDefaultColor(correctColor);
}

else if (button.Status() == MOUSE_OUT) {
    restartButton[3].Clear(g_BG_COLOR);
    restartButton[3].SetText(L"[mode] ");
    restartButton[3].Draw(restartButton[0].getCurrentColor());
}

lastStatus = button.Status();
}

// handles status and color changes to the UI restart button array for restartButton[0]
void restartStatusCallback(mouseButton& button) {
static int lastStatus = 0;

if (button.Status() & MOUSE_UP) {
    RESTART_FLAG = 1;
    for (int i = 0; i < 3; ++i)
        restartButton[i].SetStatus(MOUSE_OUT);
    return;
}

if (button.Status() == lastStatus) return;

byte a = restartButton[1].Status();  // mode 1 button
byte b = restartButton[2].Status();  // mode 2 button

if ((a | b) & (MOUSE_DOWN | MOUSE_HOVERED)) {
    // mode 1 or 2 button is the target
    button.SetStatus(MOUSE_HOVERED);  // cancel mouse down on main restart button
}
else {
    // The status has changed // make colors match
    restartButton[1].SetDefaultColor(button.getCurrentColor());
    restartButton[1].Draw(button.getCurrentColor());

    restartButton[2].SetDefaultColor(button.getCurrentColor());
    restartButton[2].Draw(button.getCurrentColor());

    restartButton[3].Draw(button.getCurrentColor());
}

lastStatus = button.Status();
}

// callback for UI button that activates mapping screen
void mappingButtonCallback(mouseButton& button) {
if (button.Status() & MOUSE_UP) {
    button.SetStatus(MOUSE_OUT);
    MAPPING_FLAG = 1;
}

}

// callback for face buttons in mapping screen
void mappingControlerButtonsCallback(mouseButton& button) {
    static int msgDisplayed = 0;
    static textBox topMsg(CONSOLE_WIDTH / 2, 5, 15, 1, ALIGN_CENTER, L" Click To Map ", BRIGHT_MAGENTA);
    static textBox footerMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 1, 24, 1, ALIGN_CENTER, L" Click Button to Remap ", DEFAULT_TEXT);

    static tUIColorPkg colors(0,0,0,0);
    // update colors if needed
    if (!(colorSchemeToColorPkg() == colors)) {
        colors = colorSchemeToColorPkg();
        topMsg.SetColor(colors.col3);
        footerMsg.SetColor(colors.col1);
    }

    // use g_extraData as a pointer to int currentHoveredButton shared with calling function 
    int* currentHoveredButton = static_cast<int*>(g_extraData);

    // # 'Click To Map' messaging and currentHoveredButton logic
{
    // display 'click to map' message if not already displayed
    if (button.Status() & MOUSE_HOVERED && !msgDisplayed) {
        topMsg.Draw();
        footerMsg.SetColor(colors.col2);
        footerMsg.Draw();
        
        *currentHoveredButton = button.GetId();

        msgDisplayed = true;
    }
    else if (button.Status() == MOUSE_HOVERED && msgDisplayed == 1)
    {       // if it is displayed increment the msgDisplayed counter
        ++msgDisplayed; //covers conditions where buttons are right next to or overlapping

        *currentHoveredButton = button.GetId();
    }

    // decrement msgDisplayed counter and clear message from screen if 0
    else if (button.Status() == MOUSE_OUT && msgDisplayed) {
        --msgDisplayed;
        if (!msgDisplayed) {
            topMsg.Clear(colors.col2);
            footerMsg.SetColor(g_BG_COLOR);
            footerMsg.Draw();
            *currentHoveredButton = -1;
        }
    }
}

    // Share status with button partners // for highlighting
    auto other = button.GetPartner();
    if (other != nullptr) {
        other->SetStatus(button.Status());
        other->Update();
    }

    // If clicked, set a flag or something
    if (button.Status() & MOUSE_DOWN) {
        *currentHoveredButton += MAP_BUTTON_CLICKED; // add value higher then any conceivable input index
    }

    // if click released
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
    }
}

// used by Done button, callback for ending mapping process/screen
void mappingDoneButtonCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        MAPPING_FLAG = 0;
        g_screen.SetButtonsCallback(nullptr);
        CoupleControllerButtons();

        g_screen.ClearButtons();
    }
}

// used by All Inputs button, callback for mapping all inputs
void mappingAllButtonsCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        MAPPING_FLAG = 2;
    }
}


// ********************************
// tUI Helper Functions

std::unordered_map<std::string, int> getUIJoystickList() {
    // Check for available joysticks
    int numJoysticks = SDL_NumJoysticks();
    if (numJoysticks <= 0)
    {
        setErrorMsg(L" No Joysticks Connected! ", 26);

        // ## ERRORS to be handled by caller
    }
    // Create and Populate list
    std::unordered_map<std::string, int> joystickList;
    for (int i = 0; i < numJoysticks; ++i)
    {
        std::string joystickName = SDL_JoystickNameForIndex(i);
        joystickList[joystickName] = i;
    }
    return joystickList;
}

// to allow for smooth animation, establish connection to host in a separate thread
void threadedEstablishConnection(TCPConnection& client, int& retVal) {
    while (!APP_KILLED && retVal == WSAEWOULDBLOCK)
    {
        retVal = client.establish_connection();
        if (!APP_KILLED && retVal == WSAEWOULDBLOCK)
            Sleep(50);
    }
}

// used to display current joystick inputs via on-screen controller face
void buttonStatesFromXboxReport(XUSB_REPORT& xboxReport) {
    byte shoulderRedraw = 0;  // 0x01: left, 0x08: right
    auto updateButtonStatus = [&](mouseButton& button, bool condition, byte special = 0) {
        if (condition && button.Status() == MOUSE_OUT) {
            button.SetStatus(MOUSE_HOVERED | MOUSE_DOWN);
            if (!special)
                // newlyActive.push_back(&button);
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
    updateButtonStatus(button_LStickLeft_highlight, xboxReport.sThumbLX < 0);   // left
    updateButtonStatus(button_LStickRight_highlight, xboxReport.sThumbLX > 0);   // right
    updateButtonStatus(button_LStickDown_highlight, xboxReport.sThumbLY < 0);   // down
    updateButtonStatus(button_LStickUp_highlight, xboxReport.sThumbLY > 0);   // up

    // Right Stick
    updateButtonStatus(button_RStickLeft_highlight, xboxReport.sThumbRX < 0);   // left
    updateButtonStatus(button_RStickRight_highlight, xboxReport.sThumbRX > 0);   // right
    updateButtonStatus(button_RStickDown_highlight, xboxReport.sThumbRY < 0);   // down
    updateButtonStatus(button_RStickUp_highlight, xboxReport.sThumbRY > 0);   // up

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

// Prints the current controller header to screen
void printCurrentController(SDLJoystickData& activeGamepad) {
    if (g_mode == 1)
        g_outputText = " Using: " + activeGamepad.name + " ";
    else
    {
        g_outputText = " Using: DS4 Controller ";
        if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
        else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
    }
    g_outputText += " ";
    setCursorPosition(8, 5);
    std::wcout << g_toWide(g_outputText);

    if (g_mode == 1)
        g_outputText = " Mode 1: (XBOX) ";
    else
        g_outputText = " Mode 2: (DS4) ";

    setCursorPosition(48, 5);
    std::wcout << g_toWide(g_outputText);
}

// checks if the Q button has been pressed and sets APP_KILLED to true
void checkForQuit() {
    if (IsAppActiveWindow() && checkKey(0x51, IS_PRESSED) && getKeyState(VK_SHIFT)) // Q for Quit
        APP_KILLED = true;
}

// accepts keyboard input for ip addresses
char IpInputLoop() {
    if (!IsAppActiveWindow()) { 
        return 'X'; 
    }

    if (checkKey(0x30, IS_PRESSED) || checkKey(VK_NUMPAD0, IS_PRESSED))
        return '0';
    else if (checkKey(0x31, IS_PRESSED) || checkKey(VK_NUMPAD1, IS_PRESSED))
        return '1';
    else if (checkKey(0x32, IS_PRESSED) || checkKey(VK_NUMPAD2, IS_PRESSED))
        return '2';
    else if (checkKey(0x33, IS_PRESSED) || checkKey(VK_NUMPAD3, IS_PRESSED))
        return '3';
    else if (checkKey(0x34, IS_PRESSED) || checkKey(VK_NUMPAD4, IS_PRESSED))
        return '4';
    else if (checkKey(0x35, IS_PRESSED) || checkKey(VK_NUMPAD5, IS_PRESSED))
        return '5';
    else if (checkKey(0x36, IS_PRESSED) || checkKey(VK_NUMPAD6, IS_PRESSED))
        return '6';
    else if (checkKey(0x37, IS_PRESSED) || checkKey(VK_NUMPAD7, IS_PRESSED))
        return '7';
    else if (checkKey(0x38, IS_PRESSED) || checkKey(VK_NUMPAD8, IS_PRESSED))
        return '8';
    else if (checkKey(0x39, IS_PRESSED) || checkKey(VK_NUMPAD9, IS_PRESSED))
        return '9';
    else if (checkKey(VK_BACK, IS_PRESSED))
        return 'B';
    else if (checkKey(VK_DELETE, IS_PRESSED))
        return 'D';
    else if (checkKey(VK_TAB, IS_PRESSED))
        return 'N';
    else if (checkKey(VK_OEM_PERIOD, IS_PRESSED) || checkKey(VK_DECIMAL, IS_PRESSED))
        return '.';

    return 'X';
}

// returns a list of active joystick inputs
std::vector<SDLButtonMapping::ButtonMapInput> get_sdljoystick_input_list(const SDLJoystickData& joystick) {
    std::vector<SDLButtonMapping::ButtonMapInput> inputs;

    SDLButtonMapping::ButtonMapInput input;

    // Get the joystick state
    SDL_JoystickUpdate();

    // Iterate over all joystick axes
    for (int i = 0; i < joystick.num_axes; i++) {
        int axis_value = SDL_JoystickGetAxis(joystick._ptr, i); // / 32767.0f;
        if (std::abs(axis_value - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD) {
            input.set(SDLButtonMapping::ButtonType::STICK, i, axis_value < 0 ? -1 : 1);
            inputs.push_back(input);
        }
    }

    // Iterate over all joystick buttons
    for (int i = 0; i < joystick.num_buttons; i++) {
        if (SDL_JoystickGetButton(joystick._ptr, i)) {
            input.set(SDLButtonMapping::ButtonType::BUTTON, i, 1);
            inputs.push_back(input);
        }
    }

    // Iterate over DPad hats and record their value
    for (int i = 0; i < joystick.num_hats; i++) {
        int hat_direction = SDL_JoystickGetHat(joystick._ptr, i);
        if (hat_direction != 0) {
            input.set(SDLButtonMapping::ButtonType::HAT, i, hat_direction);
            inputs.push_back(input);
        }
    }

    return inputs;
}

// should return colorScheme package for screen elements that matches current g_currentColorScheme
tUIColorPkg colorSchemeToColorPkg(int version) {
    ColorScheme& colorsScheme = colorSchemes[g_currentColorScheme];
    WORD bg;
    switch(version){
    case 1:
        bg = inverseFGColor(colorsScheme.faceColor >> 4) AS_BG;
        return tUIColorPkg( // inverted face  // normal text with new bg color
            static_cast<WORD>(colorsScheme.buttonColor | bg),
            static_cast<WORD>(colorsScheme.faceColor >> 4 | bg),
            static_cast<WORD>(colorsScheme.selectColor >> 4 | bg),
            static_cast<WORD>(colorsScheme.faceColor >> 4 | g_BG_COLOR BG_ONLY)
        );

    case 2:
        bg = inverseFGColor(colorsScheme.selectColor >> 4) AS_BG;
        return tUIColorPkg( // inverted select // stand out message
            static_cast<WORD>(colorsScheme.buttonColor | bg),
            static_cast<WORD>(colorsScheme.faceColor >> 4 | bg),
            static_cast<WORD>(g_BG_COLOR >> 4 | bg),
            static_cast<WORD>(colorsScheme.outlineColor | bg) // good on watermelon
        );

    case 3:
        return tUIColorPkg( // with g_BG_COLOR AS BG
            static_cast<WORD>(colorsScheme.buttonColor | g_BG_COLOR BG_ONLY),
            static_cast<WORD>(colorsScheme.faceColor >> 4 | g_BG_COLOR BG_ONLY),
            static_cast<WORD>(inverseFGColor(g_BG_COLOR FG_ONLY) | g_BG_COLOR BG_ONLY),
            static_cast<WORD>(colorsScheme.outlineColor | g_BG_COLOR BG_ONLY)
        );

    case 4:
        bg = (g_BG_COLOR FG_ONLY) AS_BG;
        return tUIColorPkg( // different HEADDING text colors
            static_cast<WORD>(colorsScheme.buttonColor | bg),
            static_cast<WORD>(colorsScheme.faceColor >> 4 | bg),
            static_cast<WORD>(inverseFGColor(g_BG_COLOR FG_ONLY) | bg),
            static_cast<WORD>(colorsScheme.outlineColor | bg)
        );

    case 5:
        return tUIColorPkg( // select default
            colorsScheme.selectColor,
            static_cast<WORD>(colorsScheme.highlightColor | colorsScheme.faceColor),
            static_cast<WORD>(colorsScheme.buttonColor | inverseFGColor(colorsScheme.faceColor >> 4) AS_BG),
            0x0000  // unused
        );

    default:    // same as controller face buttons
        return tUIColorPkg(
            static_cast<WORD>(colorsScheme.buttonColor | colorsScheme.faceColor), // default
            static_cast<WORD>(colorsScheme.highlightColor | colorsScheme.faceColor), // highlight
            colorsScheme.selectColor,                                               // select
            0x0000  // unused
        );
    }
}

// will look for a saved controller mapping and open it or initiate the mapping process
void uiOpenOrCreateMapping(SDLJoystickData& joystick, std::string& mapName, textUI& screen) {
    // Check for a saved Map for selected joystick
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;
    if (result.first) {
        // File exists, try and load data
        joystick.mapping.loadMapping(filePath.string());
    }
    else {
        // File does not exist
        
        // set up UI for xbox controller face
        BuildXboxFace();
        SetControllerButtonPositions(1);

            // set mapping flag = 2 to indicate that all inputs should be mapped
        MAPPING_FLAG = 2;
            // launch the mapping inputs screen
        tUIRemapInputsScreen(joystick, screen);
    }
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



// ********************************
// tUI Screens 

int tUISelectJoystickDialog(int numJoysticks, SDLJoystickData& joystick, textUI& screen) {
    constexpr int START_LINE = 7;
    constexpr int START_COL = 20;

    if (!numJoysticks) {
        //APP_KILLED = true;
        errorOut.SetPosition(consoleWidth / 2, START_LINE+1, 50, 0, ALIGN_CENTER);
    }
    else {
        errorOut.SetPosition(consoleWidth / 2, 4, 50, 0, ALIGN_CENTER);
    }

    mouseButton* availableJoystickBtn = new mouseButton[9];  // max of 9 joysticks can be recognized 

    auto cleanMemory = [&]() {
        screen.ClearButtons();
        for (int i = 0; i < numJoysticks; ++i) {
            delete[] availableJoystickBtn[i].getTextPtr();
        }

        delete[] availableJoystickBtn;
    };
 
    // populate selectable buttons and add to screen 
    for (int i = 0; i < numJoysticks && i < 9; ++i)
    {
        // Determine the length of the char string
        size_t charLen = strlen(SDL_JoystickNameForIndex(i));
        // Allocate memory for the wchar_t string
        wchar_t* wideStr = new wchar_t[charLen + 6]; // +4 for the index string, +1 for a space and +1 null terminator
        // Convert the char string to a wchar_t string // store in message3 as temp location 
        mbstowcs(msgPointer3, SDL_JoystickNameForIndex(i), charLen + 1);

        // Copy to wideStr with a formatted index
        swprintf(wideStr, L"(%d) %s ", i+1, msgPointer3);

        // create a button with joystick index and name
        availableJoystickBtn[i] = mouseButton(START_COL, START_LINE + i, charLen + 5, wideStr);
        // set the id for selection index
        availableJoystickBtn[i].SetId(i);
        // set callback function to enable selection
        availableJoystickBtn[i].setCallback(&joystickSelectCallback);
        // add the button to the screen
        screen.AddButton(&availableJoystickBtn[i]);
    }
    screen.AddButton(&quitButton);

    // set colors
    screen.SetBackdropColor(g_BG_COLOR);
    screen.SetButtonsColors(colorSchemeToColorPkg(0));   // matches controller face buttons
    output1.SetColor(colorSchemeToColorPkg(1).col1);    // title/heading text // inverted face color for bg
    errorOut.SetColor(colorSchemeToColorPkg(0).col3);   // selected button colors
    // alternate colors
    /*
    screen.SetBackdropColor(colorSchemeToColorPkg(2).col1);
    screen.SetButtonsColors(colorSchemeToColorPkg(0));
    output1.SetColor(colorSchemeToColorPkg(3).col1);
    */

    // Draw the screen with the available joysticks
    screen.ReDraw();
    
    // add messages to screen
    errorOut.Draw();

    wcsncpy_s(msgPointer1, 19, L"Select a Joystick:", _TRUNCATE);
    output1.SetText(msgPointer1);
    output1.Draw();
    

    // Prompt the user to select a joystick
    g_joystickSelected = -1;

    if (numJoysticks) {
        setCursorPosition(START_COL + 7, 17);
        std::wcout << " (1";
        if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
        std::wcout << ") Select  ";
    }

    // scan for mouse and keyboard input
    while (!APP_KILLED && g_joystickSelected < 0) {
        if (IsAppActiveWindow()) {
            screenLoop(screen);
            checkForQuit();

            if (getKeyState(0x31) || getKeyState(VK_NUMPAD1))
                g_joystickSelected = 0;
            else if (getKeyState(0x32) || getKeyState(VK_NUMPAD2))
                g_joystickSelected = 1;
            else if (getKeyState(0x33) || getKeyState(VK_NUMPAD3))
                g_joystickSelected = 2;
            else if (getKeyState(0x34) || getKeyState(VK_NUMPAD4))
                g_joystickSelected = 3;
            else if (getKeyState(0x35) || getKeyState(VK_NUMPAD5))
                g_joystickSelected = 4;
            else if (getKeyState(0x36) || getKeyState(VK_NUMPAD6))
                g_joystickSelected = 5;
            else if (getKeyState(0x37) || getKeyState(VK_NUMPAD7))
                g_joystickSelected = 6;
            else if (getKeyState(0x38) || getKeyState(VK_NUMPAD8))
                g_joystickSelected = 7;
            else if (getKeyState(0x39) || getKeyState(VK_NUMPAD9))
                g_joystickSelected = 8;
        }

        // if selection has been assigned
        if (g_joystickSelected > -1) {
            // Check if the selected index is invalid
            if (g_joystickSelected < 0 || g_joystickSelected >= numJoysticks)
            {
                setErrorMsg(L"Invalid joystick index.", 24);
                errorOut.Draw();
                g_joystickSelected = -1;
            }
        }
    }

    if (APP_KILLED) {
        cleanMemory();
        return 0;
    }

    // Open the selected joystick
    joystick._ptr = SDL_JoystickOpen(g_joystickSelected);
    if (joystick._ptr == nullptr)
    {
        setErrorMsg(L"Failed to open joystick.", 25);
        errorOut.Draw();
        APP_KILLED = true;
        cleanMemory();
        return 0;
    }

    cleanMemory();

    return 1;
}

int tUISelectDS4Dialog(std::vector<HidDeviceInfo>& devList, textUI& screen) {
    int numJoysticks = devList.size();
    g_joystickSelected = -1;

    constexpr int START_LINE = 7;
    constexpr int START_COL = 9;

    std::vector<mouseButton> gamepadButtons;
    auto cleanMemory = [&]() {
        screen.ClearButtons();
        for (std::vector<mouseButton>::iterator it = gamepadButtons.begin(); it != gamepadButtons.end(); ++it) {
            delete[] it->getTextPtr();
        }
    };

    // populate selectable buttons and add to screen 
    for (int i = 0; i < numJoysticks; ++i)
    {
        // Calculate the length of the formatted string
        int bufferSize = swprintf(nullptr, 0, L"%d: %ls %ls: %ls", 1 + i, devList[i].manufacturer.c_str(), devList[i].product.c_str(),
            devList[i].serial.empty() ? L" Wired" : L" Wireless");

        // Allocate memory for the wchar_t string
        wchar_t* wideStr = new wchar_t[bufferSize + 1]; // +1 for the null terminator

        // Format the string and store it in wideStr
        swprintf(wideStr, bufferSize + 1, L"%d: %ls %ls: %ls", 1 + i, devList[i].manufacturer.c_str(), devList[i].product.c_str(),
            devList[i].serial.empty() ? L" Wired" : L" Wireless");


        mouseButton btn(START_COL, START_LINE, bufferSize, wideStr);
        btn.SetId(i);
        gamepadButtons.push_back(btn);

        screen.AddButton(&gamepadButtons.back());
    }

    // Draw the screen with the available joysticks
    screen.ReDraw();
    // add messages to screen
    wcsncpy_s(msgPointer1, 27, L"Connected DS4 Controllers:", _TRUNCATE);
    output1.SetText(msgPointer1);
    output1.Draw();


    setCursorPosition(START_COL - 3, START_LINE + numJoysticks + 1);
    std::wcout << "Select a joystick (1";
    if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
    std::wcout << "): ";

    // scan for mouse and keyboard input
    while (!APP_KILLED || g_joystickSelected < 0) {
        if (IsAppActiveWindow()) {
            screenLoop(screen);
            checkForQuit();

            if (getKeyState(0x31) || getKeyState(VK_NUMPAD1))
                g_joystickSelected = 0;
            else if (getKeyState(0x32) || getKeyState(VK_NUMPAD2))
                g_joystickSelected = 1;
            else if (getKeyState(0x33) || getKeyState(VK_NUMPAD3))
                g_joystickSelected = 2;
            else if (getKeyState(0x34) || getKeyState(VK_NUMPAD4))
                g_joystickSelected = 3;
            else if (getKeyState(0x35) || getKeyState(VK_NUMPAD5))
                g_joystickSelected = 4;
            else if (getKeyState(0x36) || getKeyState(VK_NUMPAD6))
                g_joystickSelected = 5;
            else if (getKeyState(0x37) || getKeyState(VK_NUMPAD7))
                g_joystickSelected = 6;
            else if (getKeyState(0x38) || getKeyState(VK_NUMPAD8))
                g_joystickSelected = 7;
            else if (getKeyState(0x39) || getKeyState(VK_NUMPAD9))
                g_joystickSelected = 8;
        }

        // if selection has been assigned
        if (g_joystickSelected > -1) {
            // Check if the selected index is valid
            if (g_joystickSelected < 0 || g_joystickSelected >= numJoysticks)
            {
                setErrorMsg(L"Invalid joystick index.", 24);
                errorOut.Draw();
                g_joystickSelected = -1;
            }
        }
    }

    if (APP_KILLED) {
        cleanMemory();
        return -1;
    }

    cleanMemory();

    return g_joystickSelected;
}

bool tUIConnectToDS4Controller(textUI& screen) {
    std::vector<HidDeviceInfo> devList;
    HidDeviceInfo* selectedDev = nullptr;

    devList = DS4manager.scanDevices(
        ANY,                // vendor id  // Sony:: 1356
        ANY,                // product id // DS4 v1:: 2508
        ANY,                // serial
        ANY,                // manufacturer
        L"Wireless Controller",                // product string // L"Wireless Controller"
        ANY,                // release
        ANY,                // usage page
        ANY                 // usage
    );

    if (devList.size() > 1) {
        // User selects from connected DS4 devices
        int idx = tUISelectDS4Dialog(devList, screen);
        selectedDev = &devList[idx];
    }
    else if (devList.size()) {
        selectedDev = &devList[0];
    }

    if (selectedDev != nullptr) {
        if (DS4manager.OpenHidDevice(selectedDev)) {
            //std::wcout << "Connected to : " << selectedDev->manufacturer << " " << selectedDev->product << std::endl;
            return true;
        }
    }

    return false;
}

std::string tUIGetHostAddress(textUI& screen) {
    int octNum = 0;
    bool validIP = 0;
    bool firstDot = 0;
    bool makeConnection = false;
    bool warningShown = 0;
    bool errorShown = 0;
    std::string host_address;

    //set Colors values
    tUIColorPkg ColorPack1 = colorSchemeToColorPkg(1); 
    tUIColorPkg ColorPack2 = colorSchemeToColorPkg(2);
    //WORD colorGood = ColorPack1.col1;
    WORD colorError = ColorPack2.col4;;

    COORD octPos[4] = {
        {29, 10},
        {33, 10},
        {37, 10},
        {41, 10}
    };

    textInput octet[4] = {  //int x, int y, int w, int maxLength
        textInput(29, 10, 3, 3, ALIGN_CENTER, ColorPack1.col1),
        textInput(33, 10, 3, 3, ALIGN_CENTER, ColorPack1.col1),
        textInput(37, 10, 3, 3, ALIGN_CENTER, ColorPack1.col1),
        textInput(41, 10, 3, 3, ALIGN_CENTER, ColorPack1.col1)
    };

    // Lambda for setting octet cursor position
    auto setOctetCursorPos = [&]() {
        setCursorPosition(octPos[octNum].X + (octet[octNum].getCursorPosition() > 0) * 1, octPos[octNum].Y);
    };
    // Lambda function for IP address validation
    auto validIPAddress = [](const std::string& ipAddress) {
        std::regex pattern(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[1-9][0-9]?)$)");
        std::smatch match;
        return std::regex_match(ipAddress, match, pattern);
    };
    // Lambda for building host address
    auto buildAndTestIpAddress = [&]() {
        // Construct the IP address string
        std::wstring ipAddress = octet[0].getText();
        for (int i = 1; i < 4; ++i) {
            ipAddress += L"." + std::wstring(octet[i].getText());
        }
        host_address = g_converter.to_bytes(ipAddress);

        if (validIPAddress(host_address)) {
            errorOut.Clear(g_BG_COLOR);
            errorOut.SetColor(g_BG_COLOR);
            setErrorMsg(L" Valid IP ! ", 38);
            errorOut.Draw();
            output1.Draw();
            errorOut.SetColor(colorError);
            setOctetCursorPos();
            errorShown = true;
            return 1;
        }
        else if(errorShown)
        {
            errorShown = false;
            errorOut.Clear(g_BG_COLOR);
            output1.Clear(g_BG_COLOR);
            setOctetCursorPos();
        }
        return 0;
    };

    for (int i = 0; i < 4; ++i) {
        octet[i].insert(L'0');
        octet[i].cursorToBegin();
        screen.AddInput(&octet[i]);
    }

    screen.AddButton(&quitButton);

    // set colors
    screen.SetBackdropColor(g_BG_COLOR);
    screen.SetButtonsColors(colorSchemeToColorPkg(0));

    screen.SetInputsColors(colorSchemeToColorPkg(1));

    output1.SetColor(g_BG_COLOR);           //  Press Enter to Connect
    errorOut.SetColor(colorError);
    

    screen.DrawButtons();
    errorOut.Draw();

    //setTextColor(makeSafeColors(g_BG_COLOR)); // was screen.getSafeColors()
    setTextColor(ColorPack1.col1);
    setCursorPosition(12, 8);
    std::wcout << L" Enter IP Address Of Host: ";

    //setTextColor(g_BG_COLOR);
    setCursorPosition(28, 10);
    std::wcout << L"   .   .   .   ";

    errorOut.SetPosition(consoleWidth / 2, 9, 50, 1, ALIGN_CENTER);
    output1.SetText(L" Press Enter to Connect ");
    output1.SetPosition(consoleWidth / 2, 13, 25, 1, ALIGN_CENTER);
    
    screen.DrawInputs();
       
    setCursorPosition(29, 10);
    showConsoleCursor();
    
    while (!APP_KILLED && !makeConnection) {
        checkForQuit();
        screenLoop(screen);
        for (int i = 0; i < 4; ++i) {
            if (octet[i].Status() & ACTIVE_INPUT) {
                validIP = buildAndTestIpAddress();
                octNum = i;        
            }
        }
        setOctetCursorPos();

        char num = IpInputLoop();
        // B: backspace, D: delete, N: next octet, .: dot, X: nothing
        if (num == '.') {
            if (!octet[octNum].getCursorPosition() && !firstDot) {
                firstDot = 1;
            }
            else {
                num = 'N';
            }
        }
        else if (num == 'B') {
            errorShown = false;
            errorOut.Clear(g_BG_COLOR);
            output1.Clear(g_BG_COLOR);
            if (octet[octNum].getCursorPosition() == 0) {
                if (!octet[octNum].getLength()) {
                    octet[octNum].insert(L'0');
                    octet[octNum].Draw();
                }
                octNum = max(0, octNum - 1);
                setOctetCursorPos();
            }
            
            octet[octNum].Clear();
            octet[octNum].back();
            octet[octNum].Draw();         

            validIP = buildAndTestIpAddress();
        }
        else if (num == 'D') {
            octet[octNum].Clear();
            octet[octNum].del();
            octet[octNum].Draw();

            validIP = buildAndTestIpAddress();
        }
        else if (octNum < 4 && num != 'N' && num != 'X') {
            octet[octNum].overwrite(num);
            octet[octNum].Draw();

            validIP = buildAndTestIpAddress();
        }
        if (num == 'N' || octet[octNum].getCursorPosition() > 2) {
            if (!octet[octNum].getLength()) {
                octet[octNum].insert(L'0');
                octet[octNum].Draw();
            }
                    
            firstDot = 0;
            octNum = min(octNum+1,4);
            octet[octNum].cursorToBegin();
            setOctetCursorPos();
        }
                
        if (validIP) {
            if (checkKey(VK_RETURN, IS_PRESSED)) {
                makeConnection = true;
            }
        }
        else if (octNum == 4 && !errorShown && !buildAndTestIpAddress()) {
                validIP = false;   
                setErrorMsg(L" Invalid IP Address ", 21);
                errorOut.Draw();
                errorShown = true;
                octNum--;
                setOctetCursorPos();       
        }
        else if (getKeyState(VK_RETURN)) {
            warningShown = true;
            setErrorMsg(L" Invalid IP Address ", 21);
            errorOut.Draw();
            setOctetCursorPos();
        }
        else if (warningShown && !getKeyState(VK_RETURN)) {
            warningShown = false;
            errorOut.Clear(g_BG_COLOR);
            errorShown = false;
            setOctetCursorPos();
        }

        if (num == 'X')
            Sleep(15);
    }
    
    hideConsoleCursor();
    screen.ClearInputs();
    screen.ClearButtons();
    buildAndTestIpAddress();
    return host_address;
}

int tUIRemapInputsScreen(SDLJoystickData& joystick, textUI& screen) {

    //appendWindowTitle(g_hWnd, "Remap Buttons!"));
    std::vector<SDLButtonMapping::ButtonName> inputList = std::vector<SDLButtonMapping::ButtonName>();
    static bool changes = false;

    // Convert joystick name to hex mapfile name   
    std::string mapName = encodeStringToHex(joystick.name);
    // Get file path for a mapfile
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;
    bool mapExists = result.first;

    // Set up screen elements
    // ****************************
    screen.ClearButtons();
    screen.ClearInputs();
    screen.SetBackdrop(XBOX_Backdrop);

    textUI otherButtons;

    mouseButton dummyBtn; // used to spoof a callback button click
    dummyBtn.SetStatus(MOUSE_UP);

    //
    // set screen title to msgPointer1
    const int maxTitleLength = consoleWidth - 8;
    //   message + prefix + null terminator
    int titleLength = 20 + (mapExists ? 3 : 0) + 1;
    swprintf(msgPointer1, titleLength, L" %sMapping Inputs For: ", mapExists ? L"Re-" : L"");

    wchar_t* gamepadName_ptr = msgPointer1 + titleLength-1;
    // convert joystick.name to wchar_t* and store in gamepadName_ptr pointer
    mbstowcs(msgPointer3, joystick.name.c_str(), joystick.name.size());
    swprintf(gamepadName_ptr, joystick.name.size()+3, L" %s ", msgPointer3);

    // create a separate text area for the joystick name // for color change possibilities
    textBox gamepadName(consoleWidth/2 - (titleLength+joystick.name.size())/2 + titleLength-1, 1, joystick.name.size()+3, 1, ALIGN_LEFT, gamepadName_ptr, DEFAULT_TEXT);

    titleLength += joystick.name.size();

    // Set screen title position and text
    output1.SetPosition(consoleWidth / 2, 1, maxTitleLength, 1, ALIGN_CENTER);
    output1.SetText(msgPointer1);

    textBox& screenTitle = output1;

    //
    // Set up a current mapped Input area
    textBox& currentInputTitle = output2;
    currentInputTitle.SetPosition(9, 2, 17, 1, ALIGN_CENTER);
    swprintf(msgPointer2, 17, L" Current Input: "); // title
    currentInputTitle.SetText(msgPointer2);
    currentInputTitle.SetColor(BRIGHT_BLUE);

    // an area to update with the hovered button input value
    textBox currentInput = output2;
    wchar_t* currentInputText = msgPointer2 + 16;
    currentInput.SetColor(YELLOW);
    currentInput.SetPosition(9, 3, 15, 1, ALIGN_CENTER);

    // set global g_extraData to hoveredButton for access by callback function
    int hoveredButton = -1;
    g_extraData = static_cast<void*>(&hoveredButton);

    //
    // Set up a Noisy/Detected Input area
    output3.SetPosition(63, 2, 18, 1, ALIGN_CENTER);
    output3.SetColor(BRIGHT_RED | BLUE AS_BG);
    swprintf(msgPointer3, 18, L" Detected Input: "); // title
    output3.SetText(msgPointer3);

    textBox& detectedInputTitle = output3;

    const int MAX_INPUTS_SHOWN = 5;
    textBox detectedInput = output3;
    detectedInput.SetPosition(CONSOLE_WIDTH -1, 3, 14, MAX_INPUTS_SHOWN, ALIGN_RIGHT);

    wchar_t* detectedInputs = msgPointer3 + 16;
    swprintf(detectedInputs, 9, L" (NONE) "); // data display    
    detectedInput.SetText(detectedInputs);

    //
    // Set up and add buttons to screen
    AddControllerButtons(screen);
    screen.SetButtonsCallback(mappingControlerButtonsCallback);

    quitButton.SetPosition(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE + 1);
    screen.AddButton(&quitButton);
    otherButtons.AddButton(&quitButton);

    // Map All button
    mouseButton mapAllButton(CONSOLE_WIDTH / 2 - 8, XBOX_QUIT_LINE - 2, 17, L" (A) All Inputs  ");
    mapAllButton.setCallback(mappingAllButtonsCallback);
    screen.AddButton(&mapAllButton);
    otherButtons.AddButton(&mapAllButton);

    // Done button
    mouseButton doneMappingButton(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE, 11, L" (D) Done  ");
    doneMappingButton.setCallback(mappingDoneButtonCallback);
    screen.AddButton(&doneMappingButton);
    otherButtons.AddButton(&doneMappingButton);

    // Save and Cancel Buttons
    mouseButton cancelButton(CONSOLE_WIDTH / 2 + 7, XBOX_QUIT_LINE, 13, L" (C) Cancel  ");
    mouseButton saveMapButton(CONSOLE_WIDTH / 2 - 17, XBOX_QUIT_LINE, 11, L" (S) Save  ");
    screen.AddButton(&cancelButton);
    otherButtons.AddButton(&cancelButton);
    screen.AddButton(&saveMapButton);
    otherButtons.AddButton(&saveMapButton);

    // Save conformation message
    textBox saveMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 3, 24, 1, ALIGN_CENTER, L" Mapping Saved! ", colorSchemeToColorPkg(1).col1);

    // create Click To Map footer highlight area
    textBox footerMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 1, 24, 1, ALIGN_CENTER, L" Click Button to Remap ", DEFAULT_TEXT);

    // create a # characters to redraw a portion of the controller outline
    mouseButton outlineRedraw(57, 6, 4, L"##\t\t\t##\t\t\t##");
    outlineRedraw.SetDefaultColor(g_BG_COLOR);
    // create a space character to redraw a portion of the controller face
    mouseButton faceRedraw(58, 8, 1, L" ");
    faceRedraw.SetDefaultColor(colorSchemeToColorPkg(0).col1);

    // ****************************
    
    // this should provide the ability to make saveMsg message disappear
    FPSCounter timer;
    bool timing = false;
    auto vanishingMessage = [&]() {
        // get_elapsed time returns a double representing seconds
        if (timing && timer.get_elapsed_time() > 3) {
            timing = false;
            saveMsg.Clear(g_BG_COLOR);
        }
    };

    // 
    // Draw UI elements
    
    // Set button and controller colors
        // Sets controller to color scheme colors with some contrast correction for bg color then 
        // draws screen backdrop and face
    DrawControllerFace(screen, colorSchemes[g_currentColorScheme], g_BG_COLOR, 1);
        // get color package of contrast corrected button colors
    tUIColorPkg buttonColors(
        screen.GetBackdropColor(),
        button_Guide_highlight.getHighlightColor(),
        button_Guide_highlight.getSelectColor(),
        0 // unused
    );
    
    screenTitle.SetColor(colorSchemeToColorPkg(1).col1);
    gamepadName.SetColor(buttonColors.col1);
    footerMsg.SetColor(buttonColors.col1);
    otherButtons.SetButtonsColors(buttonColors);

    otherButtons.DrawButtons();

    screenTitle.Draw();
    gamepadName.Draw();

    currentInputTitle.Draw();
    detectedInputTitle.Draw(); 
    detectedInput.Draw();
    footerMsg.Draw();
    // *********

    std::vector<SDLButtonMapping::ButtonMapInput> activeInputs, lastInputs;
    int inputPtr = 0;
    int lastHovered = -1;
    int inputsListed = 0;
    int mouseState = 0;
    //
    // Logic Loop
    while (!APP_KILLED && MAPPING_FLAG) {

        vanishingMessage();

        // get activeInputs to identify noisy Inputs // temporary?
        activeInputs = get_sdljoystick_input_list(joystick);

        // clear active input area
        if (lastInputs != activeInputs) {
            detectedInput.Clear(g_BG_COLOR);
            detectedInput.SetText(L" (NONE) ");
            if (inputsListed > 3) { // after four inputs controller could get drawn over
                outlineRedraw.Draw(); // redraw controller edge
                faceRedraw.Draw();  // and face
            }

            
            inputPtr = 0;
            // write active inputs text data to memory
            int i = 0;
            inputsListed = i;
            for (auto input : activeInputs) {
                inputsListed = i+1;
                // get formatted wstring
                auto txt = SDLButtonMapping::displayInput(input);
                // add formatted input to wchar_t pointer
                swprintf(detectedInputs + inputPtr, txt.size() + 4, L"%s %s ", i ? L"\n" : L"", txt.c_str());
                // update inputPtr
                inputPtr += txt.size() + 2 + (i>0);

                if (++i > MAX_INPUTS_SHOWN) // limit inputs displayed on screen // also prevents buffer overflow
                    break;
            }
            
            // display active inputs on screen
            if (activeInputs.size()) {
                // update display
                detectedInput.SetText(detectedInputs);    
            }

            detectedInput.Draw();
            lastInputs = activeInputs;
        }    
        
        mouseState = screenLoop(screen); // a button callback, called through screenLoop, will set hoveredButton

        // display current mapping for hovered button
        if (hoveredButton != lastHovered) {
            currentInput.Clear(g_BG_COLOR);
            if (hoveredButton > -1) {
                
                auto txt = SDLButtonMapping::displayInput(
                    joystick.mapping.buttonMaps[static_cast<SDLButtonMapping::ButtonName>(hoveredButton - (hoveredButton > MAP_BUTTON_CLICKED - 1) * MAP_BUTTON_CLICKED)]
                );
                swprintf(currentInputText, txt.size() + 3, L" %s  ", txt.c_str());
                currentInput.SetText(currentInputText);
                currentInput.Draw();
            }

        }
        lastHovered = hoveredButton;

        // Check for screen Hot Keys
        if (IsAppActiveWindow() && getKeyState(VK_SHIFT)) {
            // Map All
            if (checkKey('A', IS_RELEASED)) {
                dummyBtn.SetStatus(MOUSE_UP);
                mappingAllButtonsCallback(dummyBtn);
            }
            // Done
            else if (checkKey('D', IS_RELEASED)) {
                dummyBtn.SetStatus(MOUSE_UP);
                mappingDoneButtonCallback(dummyBtn);
            }
            // Cancel
            else if (checkKey('C', IS_RELEASED)) {
                cancelButton.SetStatus(MOUSE_UP);
                mouseState = MOUSE_UP;
            }
            // Save
            else if (checkKey('S', IS_RELEASED)) {
                saveMapButton.SetStatus(MOUSE_UP);
                mouseState = MOUSE_UP;
            }
            // Quit
            else if (checkKey('Q', IS_RELEASED)) {
                dummyBtn.SetStatus(MOUSE_UP);
                exitAppCallback(dummyBtn);
            }
        }
        else {
            // still check all keys to keep state current
            checkKey('A', IS_RELEASED);
            checkKey('D', IS_RELEASED);
            checkKey('C', IS_RELEASED);
            checkKey('S', IS_RELEASED);
            checkKey('Q', IS_RELEASED);
        }

        // Save & Cancel buttons
        if (mouseState == MOUSE_UP) {
            if (saveMapButton.Status() & MOUSE_UP)
            {
                if (changes) {
                    joystick.mapping.saveMapping(filePath.string());
                    saveMsg.SetText(L" Mapping Saved! ");
                }
                else {
                    saveMsg.SetText(L" Nothing has Changed! ");
                }
                changes = false;
                saveMsg.Draw();
                timer.reset_timer();
                timing = true;

                saveMapButton.SetStatus(MOUSE_OUT);
                saveMapButton.Update();
            }
            else if (cancelButton.Status() & MOUSE_UP)
            {
                if (changes && mapExists) {
                    joystick.mapping.loadMapping(filePath.string());
                }
                cancelButton.SetStatus(MOUSE_OUT);

                mouseButton dummyBtn;
                dummyBtn.SetStatus(MOUSE_UP);
                mappingDoneButtonCallback(dummyBtn);
            }
        }

        // Map the clicked button
        if (MAPPING_FLAG && mouseState == MOUSE_UP && hoveredButton > MAP_BUTTON_CLICKED -1) {
            saveMapButton.Clear();
            cancelButton.Clear();

            changes = true;
            std::vector<SDLButtonMapping::ButtonName> inputList;
            inputList.push_back(static_cast<SDLButtonMapping::ButtonName>(hoveredButton - MAP_BUTTON_CLICKED));

            tUIMapTheseInputs(joystick, inputList);

            // re draw everything
            ReDrawControllerFace(screen, colorSchemes[g_currentColorScheme], g_BG_COLOR, 1);
            screen.DrawButtons();
            otherButtons.DrawButtons();

            screenTitle.Draw();
            gamepadName.Draw();

            currentInputTitle.Draw();
            detectedInputTitle.Draw();
            footerMsg.Draw();  // click to map message bottom
        }

        // Map all inputs
        if (MAPPING_FLAG == 2) {
            saveMapButton.Clear();
            cancelButton.Clear();

            changes = true;
            std::vector<SDLButtonMapping::ButtonName> inputList;
            // Create a list of all inputs
            inputList.insert(inputList.end(), joystick.mapping.stickButtonNames.begin(), joystick.mapping.stickButtonNames.end());
            inputList.insert(inputList.end(), joystick.mapping.shoulderButtonNames.begin(), joystick.mapping.shoulderButtonNames.end());
            inputList.insert(inputList.end(), joystick.mapping.triggerButtonNames.begin(), joystick.mapping.triggerButtonNames.end());
            inputList.insert(inputList.end(), joystick.mapping.thumbButtonNames.begin(), joystick.mapping.thumbButtonNames.end());
            inputList.insert(inputList.end(), joystick.mapping.dpadButtonNames.begin(), joystick.mapping.dpadButtonNames.end());
            inputList.insert(inputList.end(), joystick.mapping.genericButtonNames.begin(), joystick.mapping.genericButtonNames.end());
            
            tUIMapTheseInputs(joystick, inputList);

            // turn down the mapping flag to indicate not to map all inputs again
            MAPPING_FLAG = 1;

            // re draw everything
            ReDrawControllerFace(screen, colorSchemes[g_currentColorScheme], g_BG_COLOR, 1);
            screen.DrawButtons();
            otherButtons.DrawButtons();

            screenTitle.Draw();
            gamepadName.Draw();

            currentInputTitle.Draw();
            detectedInputTitle.Draw();
            footerMsg.Draw();  // click to map message bottom


            // if no saved mapping exists auto save current mapping
            if (!mapExists && !APP_KILLED) {
                mapExists = joystick.mapping.saveMapping(filePath.string());
                if (mapExists) {
                    changes = false;
                    saveMsg.Draw();
                    timer.reset_timer();
                    timing = true;
                }
            }
        }

        Sleep(50);
    }

    if (APP_KILLED) {
        return 0;
    }


    return 1;
}

void tUIMapTheseInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>& inputList) {

    //
    // Set up UI elements
    const int OVERLAY_START_COL = 24;
    const int OVERLAY_START_LINE = 13;
    textUI mapOverlay;

    tUIColorPkg UIcolors(
        button_Guide_highlight.getDefaultColor(),
        button_Guide_highlight.getHighlightColor(),
        button_Guide_highlight.getSelectColor(),
        (g_BG_COLOR FG_ONLY) | (button_Guide_highlight.getDefaultColor() BG_ONLY)
    );

    tUIColorPkg buttonColors(
        g_screen.GetBackdropColor(),
        button_Guide_highlight.getHighlightColor(),
        button_Guide_highlight.getSelectColor(),
        0 // unused
    );

    mouseButton cancelButton(CONSOLE_WIDTH / 2 - 6, XBOX_QUIT_LINE, 20, L" (C) Cancel  ");
    mouseButton skipButton(CONSOLE_WIDTH / 2 - 9, XBOX_QUIT_LINE - 1, 20, L" (Esc) Skip Input  ");
    quitButton.SetPosition(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE + 1);

    mapOverlay.AddButton(&cancelButton);
    mapOverlay.AddButton(&skipButton);
    mapOverlay.AddButton(&quitButton);

    mapOverlay.SetButtonsColors(buttonColors);

    // make this overlay out of mouse buttons because they support the '\t' special character to advance the cursor
    mouseButton MapScreen_Overlay_outlineTop(OVERLAY_START_COL, OVERLAY_START_LINE, 25, L"\t\t\t___________________\t\t\t");
    mouseButton MapScreen_Overlay_outline(OVERLAY_START_COL, OVERLAY_START_LINE + 1, 25, L"\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t//\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\\\{\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t}");
    mouseButton MapScreen_Overlay_body(OVERLAY_START_COL, OVERLAY_START_LINE + 1, 25, L"\t\t\t   Set Input For   \t\t\t\t\t                     \t\t\t                       \t");
    mouseButton MapScreen_Overlay_bodyBottom(OVERLAY_START_COL, OVERLAY_START_LINE + 4, 25, L"                                                  ");
    MapScreen_Overlay_outlineTop.SetDefaultColor(UIcolors.col4);
    MapScreen_Overlay_outline.SetDefaultColor(g_BG_COLOR);
    MapScreen_Overlay_body.SetDefaultColor(UIcolors.col1);
    MapScreen_Overlay_bodyBottom.SetDefaultColor(g_BG_COLOR);

    // areas for inputToMap name to be displayed
    textInput inputID(CONSOLE_WIDTH / 2, OVERLAY_START_LINE + 2, 25, 25, ALIGN_CENTER, UIcolors.col3);
    textInput topMsg(CONSOLE_WIDTH / 2, 5, 30, 30, ALIGN_CENTER, UIcolors.col3);

    // Draw the Overlay
    MapScreen_Overlay_outlineTop.Draw();
    MapScreen_Overlay_outline.Draw();
    MapScreen_Overlay_body.Draw();
    MapScreen_Overlay_bodyBottom.Draw();
    mapOverlay.DrawButtons();


    // Map all inputs in inputList
    for (const auto& inputToMap : inputList) {
        if (APP_KILLED) return;
        std::string inputName = SDLButtonMapping::getButtonNameString(inputToMap);
        SDLButtonMapping::ButtonType inputType = get_input_type(inputName);

        // Update current input name
        g_outputText = " " + input_verb(inputType) + " " + format_input_name(inputName) + " ";
        // Directly write to topMsg wchar_t array //
        mbstowcs(topMsg.getText(), g_outputText.c_str(), g_outputText.size() + 1);
        topMsg.SetText(topMsg.getText());

        g_outputText = " " + format_input_name(inputName) + " ";
        // Directly write to inputID wchar_t array //
        mbstowcs(inputID.getText(), g_outputText.c_str(), g_outputText.size() + 1);
        inputID.SetText(inputID.getText());

        topMsg.Draw();
        inputID.Draw();

        // draw the button as if it were selected
        mouseButton* buttonToMap = buttonIdMap[inputToMap];
        buttonToMap->SetStatus(MOUSE_DOWN);
        buttonToMap->Update();


        std::vector<SDLButtonMapping::ButtonMapInput> received_input;
        bool settingInput = true;

        while (settingInput && !APP_KILLED) {

            screenLoop(mapOverlay);


            // Check for screen Hot Keys
            if (IsAppActiveWindow()) {
                // Cancel
                if ( (getKeyState(VK_SHIFT) && checkKey('C', IS_RELEASED)) || cancelButton.Status() & MOUSE_UP) {
                    cancelButton.SetStatus(MOUSE_OUT);
                    MAPPING_FLAG = 1;

                    buttonToMap->SetStatus(MOUSE_OUT);
                    buttonToMap->Update();

                    topMsg.Clear(UIcolors.col1);
                    inputID.Clear(UIcolors.col1);

                    return;
                }
                // Skip
                else if (checkKey(VK_ESCAPE, IS_RELEASED) || skipButton.Status() & MOUSE_UP ) {
                    received_input = { SDLButtonMapping::ButtonMapInput() };
                    skipButton.SetStatus(MOUSE_OUT);
                }
                // Quit
                checkForQuit();
            }
            else {
                // still check all keys to keep state current
                checkKey('C', IS_RELEASED);
                checkKey(VK_ESCAPE, IS_RELEASED);
            }

            if (!received_input.size()) {
                // Receive an input signature
                received_input = get_sdljoystick_input_list(joystick);
                wait_for_no_sdljoystick_input(joystick);
            }

            if (received_input.size() == 1) {
                settingInput = false;
            }
            else {
                Sleep(30);
            }
        }

        if (received_input.size() == 1) {
            // Map received input to inputID
            joystick.mapping.buttonMaps[inputToMap].set(received_input[0].input_type, received_input[0].index, received_input[0].value);

            if (received_input[0].input_type == SDLButtonMapping::ButtonType::UNSET) {
                //g_outputText = " << Input " + format_input_name(inputName) + " has been skipped! >> ";
            }

            buttonToMap->SetStatus(MOUSE_OUT);
            buttonToMap->Update();

            topMsg.Clear(UIcolors.col1);
            inputID.Clear(UIcolors.col1);
        }
    }

    // ********************  

    return;
}

///  
/// ANIMATIONS.h
///  
int g_frameNum = 0;  //
const int CX_ANI_FRAME_COUNT = 12;

const wchar_t* ConnectAnimation[CX_ANI_FRAME_COUNT+1] = {
    {L" (>                               ) "},
    {L" (<>                              ) "},
    {L" (   <>                           ) "},
    {L" (      <>                        ) "},
    {L" (         <>                     ) "},
    {L" (            <>                  ) "},
    {L" (               <>               ) "},
    {L" (                  <>            ) "},
    {L" (                     <>         ) "},
    {L" (                        <>      ) "},
    {L" (                           <>   ) "},
    {L" (                              <>) "},
    {L" (                               <) "}
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

void countUpDownOG(int& counter, int maxCount) { // * a counter value of maxCount or higher could/will cause issues
    static int dir = 1; // 1: up, -1: down

    // this non intuitive block makes sure we always count up from 0
    if (counter == 0) {
        dir = -1;
    }

    // counter values of maxCount and 0 will fail
    if (counter < maxCount && counter > 0) {
        counter += dir; // count by one in 'direction' +1 or -1
    }
    // direction of counting will be reversed
    else {
        dir *= -1;
        counter += dir; // count by one in new 'direction'
    }
}


// function will loop from 0 to maxCount
// for animation cyclically through frames
void loopCount(int& counter, int maxCount) {
    counter = (counter + 1) % (maxCount+1);
}