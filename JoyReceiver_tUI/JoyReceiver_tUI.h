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

    SDLButtonMapping(){}
};

///////
/// textUI 
//////
#include "tUI/textUI.h"

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
// text buffer setting function
void setErrorMsg(const wchar_t* text, size_t length) {
    wcsncpy_s(errorPointer, length, text, _TRUNCATE);
    errorOut.SetText(errorPointer);
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


// ^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^
// tUI Helper Defines

#define SET_tUI_CONSOLE_MODE() \
{ \
    /* Get the console input handle to enable mouse input */ \
    g_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE); \
    DWORD mode; \
    GetConsoleMode(g_hConsoleInput, &mode); \
    /* Disable Quick Edit Mode */ \
    SetConsoleMode(g_hConsoleInput, ENABLE_MOUSE_INPUT | (mode & ~ENABLE_QUICK_EDIT_MODE)); \
\
    /* Set the console to UTF-8 mode */ \
    auto _ = _setmode(_fileno(stdout), _O_U8TEXT); \
\
    /* Set Version into window title */ \
    wchar_t winTitle[30]; \
    wcscpy(winTitle, L"JoyReceiver++ tUI "); \
    wcscat(winTitle, APP_VERSION_NUM); \
    SetConsoleTitleW(winTitle); \
\
    hideConsoleCursor(); \
\
    /* Set Version into backdrop */ \
    constexpr int versionStartPoint = 73 * 3 + 31; \
    const int verLength = wcslen(APP_VERSION_NUM); \
    for (int i = 0; i < verLength; i++) { \
        JoyRecvMain_Backdrop[versionStartPoint + i] = APP_VERSION_NUM[i]; } \
}

#define POPULATE_COLOR_LIST() \
{ \
    int colorseed = generateRandomInt(1100000000, 2147483647); \
    \
    for (int byteIndex = 0; byteIndex < sizeof(int); ++byteIndex) { \
        unsigned char byteValue = (colorseed >> (byteIndex * 8)) & 0xFF; \
        for (int nibbleIndex = 0; nibbleIndex < 2; ++nibbleIndex) { \
            unsigned char nibble = (byteValue >> (nibbleIndex * 4)) & 0x0F; \
            colorList.push_back(nibble); \
        } \
    }\
}

#define GET_NEW_COLOR_SCHEME() \
{\
    g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES);       /* will be used as index for fullColorSchemes */ \
    if (g_currentColorScheme == RANDOMSCHEME) \
    { \
        WORD newRandomBG = generateRandomInt(0, 15) AS_BG; \
        static ColorScheme randomScheme; \
        randomScheme = createRandomScheme(); \
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(randomScheme, newRandomBG); \
    } \
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);  /* Set for compatibility with DrawControllerFace */ \
}


#define BUILD_CONNECTION_tUI() \
{ \
    screen.SetBackdrop(JoyRecvMain_Backdrop); \
    quitButton.SetPosition(11, 17); \
    screen.AddButton(&quitButton); \
\
    errorOut.SetPosition(consoleWidth / 2, 5, 50, 0, ALIGN_CENTER); \
\
    output1.SetPosition(consoleWidth / 2 + 2, 7, 50, 1, ALIGN_CENTER); \
    output1.SetText(L" Waiting For Connection "); \
\
}

#define COLOR_AND_DRAW_CX_tUI() \
{ \
    WORD headingColor = makeSafeColors(fullColorSchemes[g_currentColorScheme].controllerColors.col4); \
    output2.SetColor(headingColor); /* for connection animation */ \
    screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg); \
    screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors)); \
    screen.ReDraw(); \
\
    errorOut.Draw(); \
\
    output1.SetColor(headingColor); \
    output1.Draw(); \
\
    /* Show PORT and IPs */ \
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); \
    setCursorPosition(26, 9); \
    std::wcout << L" "; \
    std::wcout << args.port; \
    std::wcout << L" "; \
\
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1); \
    setCursorPosition(28, 11); \
    std::wcout << L" " +  g_converter.from_bytes(localIP) + L" "; \
\
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3); \
    setCursorPosition(28, 13); \
    std::wcout << L" " +  g_converter.from_bytes(externalIP) + L" "; \
}

#define BUILD_CX_EGG() \
    auto roll_new_color = [&]() { \
        GET_NEW_COLOR_SCHEME(); \
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); \
        fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3); \
    }; \
    auto redraw_cx_tUI = [&]() { \
        COLOR_AND_DRAW_CX_tUI(); \
    }; \
    mouseButton colorEgg(50, consoleHeight - 3, 1, L"©"); \
    \
    screen.AddButton(&colorEgg); \
    g_mode = false; /* hijack this (at the moment) unused global int */ \
    colorEgg.setCallback([](mouseButton& btn) { \
        if (btn.Status() & MOUSE_UP) { \
            btn.SetStatus(MOUSE_OUT); \
            g_mode = true; \
        } \
    }); 

#define COLOR_CX_EGG() \
{ \
    std::vector<WORD> colorList; \
    POPULATE_COLOR_LIST(); \
    colorEgg.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
    colorEgg.SetHighlightColor(fullColorSchemes[g_currentColorScheme].menuBg); \
    colorEgg.SetSelectColor( \
        CheckContrastMismatch(fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY, fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) ? \
        (findSafeFGColor(fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY, colorList, colorList.begin()) | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) : \
        (fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) \
    ); \
}

#define JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION() \
{ \
    BUILD_CX_EGG(); \
    COLOR_CX_EGG(); \
    colorEgg.Draw(); \
\
    /* Set up animation variables */ \
    int frameDelay = 0; \
    int footFrameNum = 0; \
    mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t"); \
    mouseButton rightBorderPiece(68, 18, 5, L"\t/   \t\t:}-     "); \
    leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
    rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
\
    mouseButton ani2(3, 18, 10, FooterAnimation[footFrameNum]); \
    ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
    int lastAni1Frame = g_frameNum - 1; \
    int lastAni2Frame = footFrameNum - 1; \
\
    /* Set up thread */ \
    allGood = WSAEWOULDBLOCK; \
    std::thread connectThread(threadedAwaitConnection, std::ref(server), std::ref(allGood), std::ref(connectionIP)); \
\
    while (!APP_KILLED && allGood == WSAEWOULDBLOCK) { \
        /* Animation 1 */ \
        if (lastAni2Frame != g_frameNum) { \
            lastAni2Frame = g_frameNum; \
            output2.SetText(ConnectAnimationLeft[g_frameNum]); \
            output2.SetPosition((consoleWidth / 2) - 16, 7); \
            output2.Draw(); \
\
            output2.SetText(ConnectAnimationRight[g_frameNum]); \
            output2.SetPosition((consoleWidth / 2) + 15, 7); \
            output2.Draw(); \
        } \
        if (frameDelay % 13 == 0) { \
            loopCount(g_frameNum, CX_ANI_FRAME_COUNT); \
        } \
\
        /* Animation 2 */ \
        if (lastAni2Frame != footFrameNum) { \
            lastAni2Frame = footFrameNum; \
            ani2.SetText(FooterAnimation[footFrameNum]); \
\
            for (int i = 0; i < 7; ++i) { \
                ani2.SetPosition(3 + (i * 10), 18); \
                ani2.Draw(); \
            } \
\
            /* Redraw border pieces */ \
            leftBorderPiece.Draw(); \
            rightBorderPiece.Draw(); \
        } \
        if (frameDelay % 4 == 0) { \
            if ((frameDelay % 600) <= 300) { \
                revLoopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT); \
            } else { \
                loopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT); \
            } \
        } \
\
        /* Check input */ \
        checkForQuit(); \
        if (g_mode == true) { \
            g_mode = false; \
            roll_new_color(); \
            redraw_cx_tUI(); \
\
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
            ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
\
            COLOR_CX_EGG(); \
        } \
        screenLoop(screen); \
\
        if (!APP_KILLED && allGood == WSAEWOULDBLOCK) { \
            Sleep(20); \
            ++frameDelay; \
        } \
    } \
    connectThread.detach(); \
    screen.RemoveButton(&colorEgg); \
\
    if (!APP_KILLED) { \
        /* return  to blocking mode */ \
        server.set_client_blocking(true); \
        server.set_server_blocking(true); \
        /* set wstring clientIP */ \
        clientIP = g_converter.from_bytes(connectionIP); \
    } \
}

#define BUILD_MAIN_LOOP_tUI() \
{ \
    screen.ClearButtons(); \
\
    /* Backdrop setup */ \
    int QUITLINE; \
    if (op_mode == 2) { \
        screen.SetBackdrop(DS4_Backdrop); \
        QUITLINE = DS4_QUIT_LINE; \
        BuildDS4Face(); \
    } else { \
        screen.SetBackdrop(XBOX_Backdrop); \
        QUITLINE = XBOX_QUIT_LINE; \
        BuildXboxFace(); \
    } \
\
    /* Messages setup */ \
    swprintf(msgPointer1, 43, L" << Connection From: %s  >> ", clientIP.c_str()); \
    output1.SetText(msgPointer1); \
    output1.SetPosition(3, 1, 43, 1, ALIGN_LEFT); \
    output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg); \
\
    swprintf(clientPointer, 18, L" %s ", clientIP.c_str()); \
    clientMsg.SetText(clientPointer); \
    clientMsg.SetPosition(23, 1, 38, 1, ALIGN_LEFT); \
    clientMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); \
\
    /* Buttons setup */ \
    SetControllerButtonPositions(op_mode); \
    quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE); \
    newColorsButton.SetPosition(consoleWidth / 2 - 8, QUITLINE - 2); \
    screen.AddButton(&quitButton); \
    screen.AddButton(&newColorsButton); \
\
    /* Colors and drawing */ \
    DrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, op_mode); \
\
    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors); \
\
    /* Color non-controller buttons and draw them */ \
    screen.SetButtonsColors(buttonColors); \
    screen.DrawButtons(); \
\
    output1.Draw(); \
    clientMsg.Draw(); \
}
