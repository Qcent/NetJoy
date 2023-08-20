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
void testCallback(mouseButton& button);
void printCurrentController(SDLJoystickData& activeGamepad);
void checkForQuit();
int waitForEnterPress_runScreen(textUI& screen);
bool checkKey(int key, bool pressed);
char IpInputLoop();
int screenLoop(textUI& screen);
int tUISelectJoystickDialog(int numJoysticks, SDLJoystickData& joystick, textUI& screen);
int tUISelectDS4Dialog(std::vector<HidDeviceInfo>& devList, textUI& screen);
bool tUIConnectToDS4Controller(textUI& screen);
std::string tUIGetHostAddress(textUI& screen);
void threadedEstablishConnection(TCPConnection& client, int& retVal);

#pragma warning(disable : 4996)
// Converts strings to wide strings and back again
std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

int g_mode = 1;
int g_currentColorScheme;
WORD g_BG_COLOR = 0;

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

wchar_t g_stringBuff[500];
wchar_t* errorPointer = g_stringBuff;   // max length 100
wchar_t* connectionPointer = g_stringBuff + 100;    // max length 25
wchar_t* fpsPointer = connectionPointer + 25;   // max length 10
wchar_t* message1 = fpsPointer + 10;   // max length 100
wchar_t* message2 = message1 + 100;   // max length 100
wchar_t* message3 = message2 + 100;   // max length 165

//wcsncpy_s(message3, 10, L"Message 3", _TRUNCATE);

HANDLE g_hConsoleInput;

textUI g_screen;
textBox errorOut(4, 22, 45, 0, ALIGN_CENTER, errorPointer, BRIGHT_RED);

textBox connectionMsg(2, 8, 20, 0, ALIGN_LEFT, connectionPointer, BRIGHT_GREEN);
textBox fpsMsg(2, 52, 20, 0, ALIGN_LEFT, fpsPointer, BRIGHT_BLUE);

textBox output1(3, 19, 60, 0, ALIGN_LEFT, message1, BRIGHT_BLUE);
textBox output2(3, 21, 60, 0, ALIGN_LEFT, message2, BRIGHT_BLUE);
textBox output3(3, 24, 60, 0, ALIGN_LEFT, message3, BRIGHT_BLUE);

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

//
// mouse button callback functions

void exitAppCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        APP_KILLED = true;
    }
}

void joystickSelectCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        g_joystickSelected = button.GetId();
    }
}

void testCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        //screen.ClearButtons();

        WORD newRandomBG = generateRandomInt(0, 15) << 4;
        g_BG_COLOR = newRandomBG;

        static ColorScheme randomScheme;
        randomScheme = createRandomScheme();
        g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES);

        if (generateRandomInt(0, 1001) % 2 == 0) {
            randomScheme = colorSchemes[g_currentColorScheme];
        }
        else {
            g_currentColorScheme = -1;
        }

        if (g_mode == 2) {
            BuildDS4Face();
            SetButtonPositions(g_mode);
            quitButton.SetPosition(consoleWidth / 2 - 5, 17);
        }
        else {
            BuildXboxFace();
            SetButtonPositions(g_mode);
            quitButton.SetPosition(consoleWidth / 2 - 5, 18);
        }

        DrawControllerFace(g_screen, randomScheme, newRandomBG, g_mode);

        quitButton.SetDefaultColor(g_screen.getSafeColors());
        quitButton.SetHighlightColor(makeSafeColors(button_Guide_highlight.getHighlightColor()));
        quitButton.SetSelectColor(makeSafeColors(button_Guide_highlight.getSelectColor()));
        quitButton.Update();
    }
}

// Prints the current controller header to screen
void printCurrentController(SDLJoystickData& activeGamepad) {
    if (g_mode == 1)
        g_outputText = "Using: " + activeGamepad.name;
    else
    {
        g_outputText = "Using: DS4 Controller ";
        if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
        else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
    }
    setCursorPosition(9, 5);
    std::wcout << g_toWide(g_outputText);

    if (g_mode == 1)
        g_outputText = "Mode 1: (XBOX)";
    else
        g_outputText = "Mode 2: (DS4)";

    setCursorPosition(49, 5);
    std::wcout << g_toWide(g_outputText);
}

// checks if the Q button has been pressed and sets APP_KILLED to true
void checkForQuit() {
    if (checkKey(0x51, IS_PRESSED)) // Q for Quit
        APP_KILLED = true;
}

// loops until the user presses enter while running screen logic
int waitForEnterPress_runScreen(textUI& screen) {
    while (!APP_KILLED) {
        if (checkKey(VK_RETURN, IS_PRESSED))
            return 1;
        if (checkKey(VK_BACK, IS_PRESSED))
            return 0;
        if (checkKey(0x51, IS_PRESSED)) // Q for Quit
            APP_KILLED = true;
        screenLoop(screen);
        Sleep(15);
    }
    return 0;
}

// accepts keyboard input for ip addresses
char IpInputLoop() {
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

// loop looks for mouse input and compares to buttons on screen
int screenLoop(textUI& screen) {
    INPUT_RECORD inputRecord;
    DWORD eventsRead;

    if (!ReadConsoleInput(g_hConsoleInput, &inputRecord, 1, &eventsRead)) {
        return -1;
    }

    // Check for mouse input events
    if (inputRecord.EventType == MOUSE_EVENT) {
        MOUSE_EVENT_RECORD& mouseEvent = inputRecord.Event.MouseEvent;
        COORD mousePos = mouseEvent.dwMousePosition;

        if (mouseEvent.dwEventFlags == 0) {
            // Mouse button event
            if (mouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED) {
                screen.CheckMouseClick(MOUSE_DOWN);
            }
            else
                screen.CheckMouseClick(MOUSE_UP);
        }

        if (mouseEvent.dwEventFlags == MOUSE_MOVED) {
            // Check if mouse is hovering over the buttons
            screen.CheckMouseHover(mousePos);
        }

    }
    return 0;
}

int tUISelectJoystickDialog(int numJoysticks, SDLJoystickData& joystick, textUI& screen) {
    constexpr int START_LINE = 7;
    constexpr int START_COL = 20;

    mouseButton* availableJoystickBtn = new mouseButton[9];

    auto cleanMemory = [&]() {
        screen.ClearButtons();
        for (int i = 0; i < numJoysticks; ++i) {
            delete[] availableJoystickBtn[i].getTxtPtr();
        }

        delete[] availableJoystickBtn;
    };
 
    // populate selectable buttons and add to screen 
    for (int i = 0; i < numJoysticks && i < 9; ++i)
    {
        // Determine the length of the char string
        size_t charLen = strlen(SDL_JoystickNameForIndex(i));
        // Allocate memory for the wchar_t string
        wchar_t* wideStr = new wchar_t[charLen + 5]; // +4 for the index string and +1 null terminator
        // Convert the char string to a wchar_t string // store in message3 as temp location 
        mbstowcs(message3, SDL_JoystickNameForIndex(i), charLen + 1);

        // Copy to wideStr with a formatted index
        swprintf(wideStr, L"(%d) %s", i+1, message3);

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
    // Draw the screen with the available joysticks
    screen.ReDraw();
    // add messages to screen
    wcsncpy_s(message1, 19, L"Select a Joystick:", _TRUNCATE);
    output1.SetText(message1);
    output1.Draw();

    
      // Prompt the user to select a joystick
   g_joystickSelected = -1;

    setCursorPosition(START_COL+6 , 17);
    std::wcout << "(1";
    if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
    std::wcout << ") Select";

    // scan for mouse and keyboard input
    while (!APP_KILLED && g_joystickSelected < 0) {
        screenLoop(screen);
        if (getKeyState('Q'))
            APP_KILLED = true;
        else if (getKeyState(0x31) || getKeyState(VK_NUMPAD1))
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
    int selectedJoystickIndex = -1;

    constexpr int START_LINE = 7;
    constexpr int START_COL = 9;

    std::vector<mouseButton> gamepadButtons;
    auto cleanMemory = [&]() {
        screen.ClearButtons();
        for (std::vector<mouseButton>::iterator it = gamepadButtons.begin(); it != gamepadButtons.end(); ++it) {
            delete[] it->getTxtPtr();
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
    wcsncpy_s(message1, 27, L"Connected DS4 Controllers:", _TRUNCATE);
    output1.SetText(message1);
    output1.Draw();


    setCursorPosition(START_COL - 3, START_LINE + numJoysticks + 1);
    std::wcout << "Select a joystick (1";
    if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
    std::wcout << "): ";

    // scan for mouse and keyboard input
    while (!APP_KILLED || selectedJoystickIndex < 0) {
        screenLoop(screen);
        if (getKeyState('Q'))
            APP_KILLED = true;
        else if (getKeyState('1'))
            selectedJoystickIndex = 0;
        else if (getKeyState('2'))
            selectedJoystickIndex = 1;
        else if (getKeyState('3'))
            selectedJoystickIndex = 2;
        else if (getKeyState('4'))
            selectedJoystickIndex = 3;
        else if (getKeyState('5'))
            selectedJoystickIndex = 4;
        else if (getKeyState('6'))
            selectedJoystickIndex = 5;
        else if (getKeyState('7'))
            selectedJoystickIndex = 6;
        else if (getKeyState('8'))
            selectedJoystickIndex = 7;
        else if (getKeyState('9'))
            selectedJoystickIndex = 8;

        // if selection has been assigned
        if (selectedJoystickIndex > -1) {
            // Check if the selected index is valid
            if (selectedJoystickIndex < 0 || selectedJoystickIndex >= numJoysticks)
            {
                setErrorMsg(L"Invalid joystick index.", 24);
                errorOut.Draw();
                selectedJoystickIndex = -1;
            }
        }
    }

    if (APP_KILLED) {
        cleanMemory();
        return -1;
    }

    cleanMemory();

    return selectedJoystickIndex;
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
    std::string host_address;

    COORD octPos[4] = {
        {29, 10},
        {33, 10},
        {37, 10},
        {41, 10}
    };

    textInput octet[4] = {  //int x, int y, int w, int maxLength
        textInput(29, 10, 3, 3, ALIGN_CENTER),
        textInput(33, 10, 3, 3, ALIGN_CENTER),
        textInput(37, 10, 3, 3, ALIGN_CENTER),
        textInput(41, 10, 3, 3, ALIGN_CENTER)
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
            errorOut.SetColor(BRIGHT_GREEN);
            setErrorMsg(L"Valid IP !", 38);
            errorOut.Clear();
            errorOut.Draw();
            output1.Draw();
            errorOut.SetColor(RED);
            setOctetCursorPos();
            return 1;
        }
        else
        {
            errorOut.Clear();
            output1.Clear();
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

    errorOut.Draw();

    setCursorPosition(12, 8);
    std::wcout << L"Enter IP Address Of Host:";

    setCursorPosition(31, 10);
    std::wcout << L".   .   .";

    errorOut.SetPosition(consoleWidth / 2, 9, 50, 1, ALIGN_CENTER);
    output1.SetText(L"Press Enter to Connect");
    output1.SetPosition(consoleWidth / 2, 13, 23, 1, ALIGN_CENTER);
    
    screen.DrawInputs();
       
    setCursorPosition(29, 10);
    showConsoleCursor();
    bool warningShown = 0;
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
            errorOut.Clear();
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
        else if (octNum == 4 && !buildAndTestIpAddress()) {
                validIP = false;
                setErrorMsg(L"Invalid IP Address.", 20);
                errorOut.Draw();
                octNum--;
                setOctetCursorPos();       
        }
        else if (getKeyState(VK_RETURN)) {
            warningShown = true;
            setErrorMsg(L"Invalid IP Address.", 20);
            errorOut.Draw();
            setOctetCursorPos();
        }
        else if (warningShown && !getKeyState(VK_RETURN)) {
            warningShown = false;
            errorOut.Clear();
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

void threadedEstablishConnection(TCPConnection& client, int& retVal) {
    while (!APP_KILLED && retVal == WSAEWOULDBLOCK)
    {
        retVal = client.establish_connection();
        if (!APP_KILLED && retVal == WSAEWOULDBLOCK)
            Sleep(50);
    }
}

///  
/// ANIMATIONS.h
///  
int g_frameNum = 0;
const int CX_ANI_FRAME_COUNT = 12;

const wchar_t* ConnectAnimation[13] = {
    {L"(>                               )"},
    {L"(<>                              )"},
    {L"(   <>                           )"},
    {L"(      <>                        )"},
    {L"(         <>                     )"},
    {L"(            <>                  )"},
    {L"(               <>               )"},
    {L"(                  <>            )"},
    {L"(                     <>         )"},
    {L"(                        <>      )"},
    {L"(                           <>   )"},
    {L"(                              <>)"},
    {L"(                               <)"}
};


const wchar_t* ConnectAnimation2[11] = {
    {L"(                         )"},
    {L"(~                        )"},
    {L"(   ~                     )"},
    {L"(      ~                  )"},
    {L"(         ~               )"},
    {L"(            ~            )"},
    {L"(               ~         )"},
    {L"(                  ~      )"},
    {L"(                     ~   )"},
    {L"(                        ~)"},
    {L"(                         )"}
};

const wchar_t* ConnectAnimationOG[10] = {
    {L"|                      |"},
    {L"|=                     |"},
    {L"|   =                  |"},
    {L"|      =               |"},
    {L"|         =            |"},
    {L"|            =         |"},
    {L"|               =      |"},
    {L"|                  =   |"},
    {L"|                     =|"},
    {L"|                      |"}
};

// function will inc/dec &counter up to maxCount and down to 0
// for animating back and forth through frames
void countUpDown(int& counter, int maxCount) {
    static int dir = 1; // 1: up, -1: down

    if (counter == 0) {
        dir = -1;
    }
    
    if (counter < maxCount && counter > 0) {
        counter += dir;
    }
    else {
        dir *= -1;
        counter += dir;
    }
}

// function will loop from 0 to maxCount
// for animation cyclically through frames
void loopCount(int& counter, int maxCount) {
    counter = (counter + 1) % (maxCount+1);
}