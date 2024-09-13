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

byte g_mode = 1;

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

// ^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^
// tUI Helper Defines

#define THEME_FILE    "recv.theme"


#define SET_tUI_CONSOLE_MODE(){ \
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
    wcscpy_s(winTitle, L"JoyReceiver++ tUI "); \
    wcscat_s(winTitle, APP_VERSION_NUM); \
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

#define INIT_tUI_SCREEN(){\
CoupleControllerButtons(); \
setErrorMsg(L"\0", 1); \
fpsMsg.SetPosition(55, 1, 7, 1, ALIGN_LEFT); \
quitButton.setCallback(&exitAppCallback); \
newColorsButton.setCallback(&newControllerColorsCallback); \
\
/* establish a color scheme */ \
GET_NEW_COLOR_SCHEME(); \
errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); \
fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3); } 

#define CX_EGG_BUTTS() \
mouseButton colorEgg(50, consoleHeight - 3, 1, L"©"); \
colorEgg.SetId(43); \
g_screen.AddButton(&colorEgg); \
colorEgg.setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        g_status |= COLOR_EGG_b; \
    } \
}); \
mouseButton borderEgg(23, 3, 3, L"tUI"); \
borderEgg.SetId(44); \
borderEgg.setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        if (!(g_status & BORDER_EGG_a)) { \
            MORPH_BORDER(); \
            if (generateRandomInt(1, 5) > 4) { \
                MAKE_PATTERNS(); \
                g_status |= PTRN_EGG_b; \
            } \
            SPEC_EGGS(); \
            g_status |= REDRAW_tUI_f; \
        } \
    } \
}); \
g_screen.AddButton(&borderEgg); 


#define COMMON_EGG_BUTTS()\
mouseButton* applyTheme = new mouseButton(3, 2, 1, (g_status & tUI_THEME_af) ? L"♦" : L"◊"); \
applyTheme->SetId(46); \
g_screen.AddButton(applyTheme); \
applyTheme->setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        if ((g_status & tUI_THEME_af)) { \
            g_status &= ~tUI_THEME_af; \
            btn.SetText(L"◊"); \
        } \
        else if ((g_status & tUI_THEME_f)) { \
            tUITheme theme; \
            if (theme.loadFromFile(THEME_FILE)) { \
                RESTORE_THEME(); \
                COLOR_EGGS(); \
                theme.drawPtrn(); \
                g_status |= REDRAW_tUI_f | tUI_THEME_af; \
                btn.SetText(L"♦"); \
            } \
        } \
    } \
}); \
mouseButton* saveTheme = new mouseButton(consoleWidth - 3, 2, 1, (g_status & tUI_THEME_f) ? L"♦" : L"◊"); \
saveTheme->SetId(47); \
g_screen.AddButton(saveTheme); \
saveTheme->setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        if (!(g_status & tUI_THEME_f)) { \
            g_status |= tUI_THEME_f | tUI_THEME_af | REFLAG_tUI_f; \
            tUITheme& theme = g_theme; \
            theme.recordTheme(fullColorSchemes[g_currentColorScheme], \
            g_status & (HEART_EGG_a | BORDER_EGG_a | PTRN_EGG_b \
            | tUI_THEME_af | tUI_THEME_f)); \
            theme.saveToFile(THEME_FILE); \
            btn.SetText(L"♦"); \
        } \
    else if (g_status & tUI_THEME_f) { \
        int result = MessageBox(NULL, L"Do you want to clear the custom theme?", L"Confirm Delete", MB_ICONQUESTION | MB_YESNO); \
        if (result == IDYES) { \
            remove(THEME_FILE); \
            g_status &= ~(tUI_THEME_f | tUI_THEME_af | tUI_LOADED_f); \
            g_status |= REFLAG_tUI_f; \
            btn.SetText(L"◊"); \
        } \
    } \
    } \
});  


// ********************************
// tUI Helper Functions

// to allow for screen loop and animation, wait for connection to client in a separate thread
void threadedAwaitConnection(TCPConnection& server, int& retVal, char* clientIP) {
    \

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


void BUILD_CONNECTION_tUI() {
    g_screen.SetBackdrop(JoyRecvMain_Backdrop);
    quitButton.SetPosition(11, 17);
    g_screen.AddButton(&quitButton);
    errorOut.SetPosition(consoleWidth / 2 + 1, 5, 50, 0, ALIGN_CENTER);
    output1.SetPosition(consoleWidth / 2 + 2, 7, 50, 1, ALIGN_CENTER);
    output1.SetText(L" Waiting For Connection ");
}

void COLOR_CX_tUI() {
    WORD headingColor = makeSafeColors(fullColorSchemes[g_currentColorScheme].controllerColors.col4);
    output2.SetColor(headingColor); /* for connection animation */
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
    output1.SetColor(headingColor);
}

void REDRAW_CX_TEXT() {
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));

    if (g_status & PTRN_EGG_b) {
        MAKE_PATTERNS();
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        no_whitespace_Draw(JoyRecvMain_Backdrop, 0, 0, 74, 1555);

        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        setCursorPosition(50, 17);
        std::wcout << L"©░Quinnco.░2024";
    }
    else {
        //clearPtrn(fullColorSchemes[g_currentColorScheme].menuBg);
        //setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        //no_whitespace_Draw(JoyRecvMain_Backdrop, 0, 0, 74, 1555);
        g_screen.DrawBackdrop();
    }

    errorOut.Draw();
    output1.Draw();
    COLOR_EGGS();
    g_screen.DrawButtons();

    /* Show PORT and IPs */
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
    setCursorPosition(26, 9);
    std::wcout << L" ";
    std::wcout << *(int*)feedbackData;
    std::wcout << L" ";

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
    setCursorPosition(28, 11);
    wprintf_s(L" %S ", localIP.c_str());

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
    setCursorPosition(28, 13);
    wprintf_s(L" %S ", externalIP.c_str());
}

void COLOR_AND_DRAW_CX_tUI() {
    COLOR_CX_tUI();
    REDRAW_CX_TEXT();
}

void JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION(TCPConnection& server, Arguments& args, int& allGood, char* connectionIP) {
    /* Set up animation variables */
    int frameDelay = 0;
    int footFrameNum = 0;
    bool no_theme = false;
    std::memcpy(feedbackData, (void*)&args.port, sizeof(int)); // you wanna get nuts? let's get nutz
    mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t");
    mouseButton rightBorderPiece(68, 18, 5, rsideFooter);
    leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
    rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);

    mouseButton ani2(3, 18, 10, FooterAnimation[footFrameNum]);
    ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
    int lastAni1Frame = g_frameNum - 1;
    int lastAni2Frame = footFrameNum - 1;

    auto roll_new_color = [&]() {
        GET_NEW_COLOR_SCHEME();
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
        };

    tUITheme& theme = g_theme;
    if (!(g_status & tUI_LOADED_f)) {
        if (theme.loadFromFile(THEME_FILE)) {
            g_currentColorScheme = RANDOMSCHEME;
            theme.restoreColors(fullColorSchemes[RANDOMSCHEME]);
            g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);
            g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
            g_screen.DrawBackdrop();
            theme.restoreState(g_status);
            if (g_status & BORDER_EGG_a) {
                g_status &= ~BORDER_EGG_a; // need to be off for morph border
                MORPH_BORDER();
                SPEC_EGGS();
            }
            if (g_status & HEART_EGG_a) {
                THE_HEARTENING();
            }
            if (g_status & CLUB_EGG_a) {
                THE_CLUBENING();
            }
            if (g_status & PTRN_EGG_b) {
                theme.drawPtrn();
            }
            g_status |= tUI_LOADED_f | REDRAW_tUI_f;
        }
        else
            no_theme = true;
    }
    else{
        g_status |= tUI_RESTART_f;
    }

    CX_EGG_BUTTS();
    COMMON_EGG_BUTTS();

    // auto activation
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch().count() % 69;
        if (!(g_status & tUI_LOADED_f) && (duration == 4 || duration == 20)) {
            MAKE_PATTERNS();
            g_status |= PTRN_EGG_b | REDRAW_tUI_f;
        }
        if (!(g_status & CLUB_EGG_a) && g_status & BORDER_EGG_a && generateRandomInt(1, 20) == 13) {
            THE_CLUBENING();
            g_status |= REDRAW_tUI_f;
        }
        if (!(g_status & BORDER_EGG_a) && generateRandomInt(1, 20) == 7) {
            MORPH_BORDER();
            g_status |= REDRAW_tUI_f;
        }
    }

    if (no_theme || (g_status & tUI_RESTART_f)) {
        if (g_status & BORDER_EGG_a) {
            SPEC_EGGS();
        }
        if (g_status & PTRN_EGG_b) {
            theme.drawPtrn();
        }
        g_status &= ~tUI_RESTART_f;
        g_status |= REDRAW_tUI_f;
    }

    //COLOR_AND_DRAW_CX_tUI();
    //COLOR_EGGS();

    /* Set up thread */
    allGood = WSAEWOULDBLOCK;
    std::thread connectThread(threadedAwaitConnection, std::ref(server), std::ref(allGood), std::ref(connectionIP));

    while (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
        /* Animation 1 */
        if (lastAni2Frame != g_frameNum) {
            lastAni2Frame = g_frameNum;
            output2.SetText(ConnectAnimationLeft[g_frameNum]);
            output2.SetPosition((consoleWidth / 2) - 16, 7);
            output2.Draw();

            output2.SetText(ConnectAnimationRight[g_frameNum]);
            output2.SetPosition((consoleWidth / 2) + 15, 7);
            output2.Draw();
        }
        if (frameDelay % 13 == 0) {
            loopCount(g_frameNum, CX_ANI_FRAME_COUNT);
        }

        /* Animation 2 */
        if (lastAni2Frame != footFrameNum) {
            lastAni2Frame = footFrameNum;
            ani2.SetText(FooterAnimation[footFrameNum]);

            for (int i = 0; i < 7; ++i) {
                ani2.SetPosition(3 + (i * 10), 18);
                ani2.Draw();
            }

            /* Redraw border pieces */
            leftBorderPiece.Draw();
            rightBorderPiece.Draw();
        }
        if (frameDelay % 4 == 0) {
            if ((frameDelay % 600) <= 300) {
                revLoopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT);
            }
            else {
                loopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT);
            }
        }

        /* Check input */
        checkForQuit();
        if (g_status & COLOR_EGG_b) {
            g_status &= ~COLOR_EGG_b;
            g_status &= ~tUI_THEME_af;
            roll_new_color();
            g_status |= RECOL_tUI_f | REFLAG_tUI_f;
        }
        if (g_status & RECOL_tUI_f) {
            g_status &= ~(RECOL_tUI_f | REDRAW_tUI_f);
            COLOR_CX_tUI();
            g_screen.DrawBackdrop();
            if (g_status & PTRN_EGG_b) {
                MAKE_PATTERNS();
            }
            REDRAW_CX_TEXT();
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            PRINT_EGG_X();
        }
        if (g_status & REDRAW_tUI_f) {
            g_status &= ~REDRAW_tUI_f;
            REDRAW_CX_TEXT();
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            PRINT_EGG_X();
        }
        if (g_status & REFLAG_tUI_f) {
            g_status &= ~REFLAG_tUI_f;
            if (!(g_status & tUI_THEME_af)) {
                applyTheme->SetText(L"◊");
                applyTheme->Update();
            }
            if (g_status & tUI_THEME_af) {
                applyTheme->SetText(L"♦");
                applyTheme->Update();
            }
        }
        screenLoop(g_screen);

        if (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
            Sleep(20);
            ++frameDelay;
        }
    }
    connectThread.detach();
    g_screen.RemoveButton(&colorEgg);
    g_screen.RemoveButton(&borderEgg);
    CLEAN_EGGS();
    memset(feedbackData, 0, sizeof(feedbackData));

    if (!APP_KILLED) {
        /* return to blocking mode */
        server.set_client_blocking(true);
        server.set_server_blocking(true);
        server.set_client_timeout(NETWORK_TIMEOUT_MILLISECONDS);
    }
}

void RECOLOR_MAIN_LOOP_tUI() {   
    output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); //"FPS:"
    //output2.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); // not used on this screen
    output3.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); //" << Connection .."
    clientMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1); // IP Address
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);  // fps value

    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
    /* Color non-controller buttons */
    g_screen.SetButtonsColors(buttonColors);
    // g_screen.SetBackdropColor(); i should probaby set this
}

void REDRAW_MAIN_LOOP_tUI() {
    setCursorPosition(0, 0);
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    std::wcout << JoyRecvMain_Backdrop;

    if (g_status & PTRN_EGG_b) {
        MAKE_PATTERNS();
        button_L1_outline.SetText(button_L1_outline2);
        button_L2_outline.SetText(button_L2_outline2);
        button_R1_outline.SetText(button_L1_outline2);
        button_R2_outline.SetText(button_L2_outline2);
    }
    else {
        clearPtrn(fullColorSchemes[g_currentColorScheme].controllerBg);
        button_L1_outline.SetText(button_L1_outline1);
        button_L2_outline.SetText(button_L2_outline1);
        button_R1_outline.SetText(button_R1_outline1);
        button_R2_outline.SetText(button_L2_outline1);
    }
    ReDrawControllerFace(g_screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, g_mode, (g_status & BORDER_EGG_a));
    COLOR_EGGS();
    g_screen.DrawButtons();
    output1.Draw();
    clientMsg.Draw();
    output3.Draw();
}

void BUILD_MAIN_LOOP_tUI(char* connectionIP) {
    g_screen.ClearButtons();
    /* Backdrop setup */
    int QUITLINE;
    if (g_mode == 2) {
        g_screen.SetBackdrop(DS4_Backdrop);
        QUITLINE = DS4_QUIT_LINE;
        BuildDS4Face();
    }
    else {
        g_screen.SetBackdrop(XBOX_Backdrop);
        QUITLINE = XBOX_QUIT_LINE;
        BuildXboxFace();
    }
    /* Messages setup */
    swprintf(msgPointer1, 43, L" << Connection From: %S  >> ", connectionIP);
    output1.SetText(msgPointer1);
    output1.SetPosition(3, 1, 43, 1, ALIGN_LEFT);
    output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);
    swprintf(clientPointer, 18, L" %S ", connectionIP);
    clientMsg.SetText(clientPointer);
    clientMsg.SetPosition(23, 1, 38, 1, ALIGN_LEFT);
    clientMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
    /* Buttons setup */
    SetControllerButtonPositions(g_mode);
    quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE);
    newColorsButton.SetPosition(consoleWidth / 2 - 7, QUITLINE - 2);
    g_screen.AddButton(&quitButton);
    g_screen.AddButton(&newColorsButton);
    output3.SetPosition(50, 1);
    output3.SetText(L" FPS:");
    output3.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);
    /* Colors and drawing */
    tUITheme& theme = g_theme;
    if (g_status & BORDER_EGG_a) {
        COMMON_EGG_BUTTS();
        SPEC_EGGS();
        swprintf(msgPointer1, 43, L"<< Connection From: %S >>", connectionIP);
        output1.SetText(msgPointer1);
        output1.SetPosition(6, 1, 43, 1, ALIGN_LEFT);
        clientMsg.SetPosition(25, 1, 38, 1, ALIGN_LEFT);
        output3.SetPosition(55, 1);
        fpsMsg.SetPosition(60, 1);
    }
    if (g_status & PTRN_EGG_b) {
        MAKE_PATTERNS();
        button_L1_outline.SetText(button_L1_outline2);
        button_L2_outline.SetText(button_L2_outline2);
        button_R1_outline.SetText(button_L1_outline2);
        button_R2_outline.SetText(button_L2_outline2);
    }
    else {
        button_L1_outline.SetText(button_L1_outline1);
        button_L2_outline.SetText(button_L2_outline1);
        button_R1_outline.SetText(button_R1_outline1);
        button_R2_outline.SetText(button_L2_outline1);
    }
    SetControllerFace(g_screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, g_mode, (g_status & BORDER_EGG_a));
    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
    /* Color non-controller buttons and draw them */
    g_screen.SetButtonsColors(buttonColors);

    g_status |= REDRAW_tUI_f;
}

void EGG_LOOP() {
    if (g_status & RECOL_tUI_f) {
        g_status &= ~(RECOL_tUI_f | REDRAW_tUI_f);
        RECOLOR_MAIN_LOOP_tUI();
        g_screen.DrawBackdrop();
        if (g_status & PTRN_EGG_b) {
            MAKE_PATTERNS();
        }
        REDRAW_MAIN_LOOP_tUI();
        if (g_status & BORDER_EGG_a) {
            mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t");
            mouseButton rightBorderPiece(68, 18, 5, rsideFooter);
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.Update();
            leftBorderPiece.Update();
        }
        PRINT_EGG_X();
    }
    if (g_status & REDRAW_tUI_f) {
        g_status &= ~REDRAW_tUI_f;
        REDRAW_MAIN_LOOP_tUI();
        if (g_status & BORDER_EGG_a) {
            mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t");
            mouseButton rightBorderPiece(68, 18, 5, rsideFooter);
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.Update();
            leftBorderPiece.Update();
        }
        PRINT_EGG_X();
    }
    if (g_status & REFLAG_tUI_f) {
        g_status &= ~REFLAG_tUI_f;
        mouseButton* applyTheme = g_screen.GetButtonById(46);
        if (!(g_status & tUI_THEME_af)) {
            applyTheme->SetText(L"◊");
            applyTheme->Update();
        }
        if (g_status & tUI_THEME_af) {
            applyTheme->SetText(L"♦");
            applyTheme->Update();
        }
    }
}

int JOYRECEIVER_tUI_WAIT_FOR_CLIENT_MAPPING(TCPConnection& server, char* buffer, int buffer_size) {
    int bytesReceived = 0, counter = 0, len = 43;
    int lastAniFrame = g_frameNum - 1, bgDrawCount = 0;

    // Setup
    swprintf(errorPointer, len, L" << Client Is Mapping Their Controller >> ", connectionIP);
    errorOut.SetWidth(len);
    errorOut.SetText(errorPointer);
    COORD oldErrorPos = errorOut.GetPosition();
    errorOut.SetPosition(oldErrorPos.X, oldErrorPos.Y+2);

    swprintf(fpsPointer, 8, L"  --  ", fpsOutput.c_str());
    fpsMsg.SetText(fpsPointer);

    tUITheme bgPtrn;
    bgPtrn.setColors(fullColorSchemes[g_currentColorScheme]);
    
    auto cleanUp = [&]() {
        server.set_client_blocking(true);
        errorOut.SetPosition(oldErrorPos);
        setErrorMsg(L"\0", 1);
        if (!(g_status & BORDER_EGG_a)) {
            clearPtrn(fullColorSchemes[g_currentColorScheme].controllerBg);
        }
        };

    server.set_client_blocking(false);

    // Draw
    MAKE_PATTERNS();
    bgPtrn.recordBlocks();

    auto draw_objects = [&]() {
        errorOut.Draw();
        fpsMsg.Draw();
        output1.Draw();
        clientMsg.Draw();
        output3.Draw();
        g_screen.DrawButtons();
        };
    draw_objects();

    // loop
    while (!APP_KILLED) {
        // check on network traffic
        bytesReceived = server.receive_data(buffer, buffer_size);
        if (bytesReceived != -WSAEWOULDBLOCK) {
            // An error or disconnect occurred
            cleanUp();
            return -1;
        }
        if (bytesReceived > -1) {
            cleanUp();
            return bytesReceived;
        }

        // do stuff here

        if (g_status & RECOL_tUI_f) {
            g_status &= ~RECOL_tUI_f;
            RECOLOR_MAIN_LOOP_tUI();
            bgPtrn.setColors(fullColorSchemes[g_currentColorScheme]);
            g_status |= REDRAW_tUI_f;
        }
        if (g_status & REDRAW_tUI_f) {
            g_status &= ~REDRAW_tUI_f;
            COLOR_EGGS();

            if (g_status & BORDER_EGG_a) {
                setCursorPosition(0,0);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << JoyRecvMain_Backdrop;
            }

            bgPtrn.drawPtrn();
            bgPtrn.shiftPtrnLeft();
            bgDrawCount = 0;
            draw_objects();
            PRINT_EGG_X();
        }
        if (g_status & REFLAG_tUI_f) {
            g_status &= ~REFLAG_tUI_f;
            mouseButton* applyTheme = g_screen.GetButtonById(46);
            if (applyTheme != nullptr) {
                if (!(g_status & tUI_THEME_af)) {
                    applyTheme->SetText(L"◊");
                    applyTheme->Update();
                }
                if (g_status & tUI_THEME_af) {
                    applyTheme->SetText(L"♦");
                    applyTheme->Update();
                }
            }
        }


        /* Animation */
        if (lastAniFrame != g_frameNum) {
            lastAniFrame = g_frameNum;

            if ((counter - 1) % 26 == 0) {
                bgPtrn.drawPtrnDiag(bgDrawCount);
                if(++bgDrawCount > 9){
                    bgDrawCount = 0;
                    bgPtrn.shiftPtrnLeft();
                }
                draw_objects();
            }

            
            output2.SetText(ConnectAnimationLeft[g_frameNum]);
            output2.SetPosition((consoleWidth / 2) - (len/2) - 5, 7);
            output2.Draw();

            output2.SetText(ConnectAnimationRight[g_frameNum]);
            output2.SetPosition((consoleWidth / 2) + (len / 2) + 2, 7);
            output2.Draw();


        }
        if (counter % 13 == 0) {
            loopCount(g_frameNum, CX_ANI_FRAME_COUNT);
        }
        

        screenLoop(g_screen);
        Sleep(20);
        counter++;
    }
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
        ColorScheme menuScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = trueFullSchemeFromSimpleScheme(g_simpleScheme, menuScheme, newRandomBG);
        g_currentColorScheme = RANDOMSCHEME;

        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].controllerBg);
        //RECOLOR_MAIN_LOOP_tUI();

        // draw messages
        g_status |= RECOL_tUI_f | REFLAG_tUI_f;
        g_status &= ~tUI_THEME_af;
    }
}