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


byte g_status = 0;
byte g_mode = 1;
byte g_currentColorScheme;
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


#define INIT_tUI_SCREEN() \
CoupleControllerButtons(); \
setErrorMsg(L"\0", 1); \
fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT); \
quitButton.setCallback(&exitAppCallback); \
newColorsButton.setCallback(&newControllerColorsCallback); \
\
/* establish a color scheme */ \
GET_NEW_COLOR_SCHEME(); \
errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); \
fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3); 


#define BUILD_CX_EGG() \
    auto roll_new_color = [&]() { \
        GET_NEW_COLOR_SCHEME(); \
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); \
        fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3); \
    }; \
    mouseButton colorEgg(50, consoleHeight - 3, 1, L"©"); \
    g_screen.AddButton(&colorEgg); \
    g_status = false; \
    colorEgg.setCallback([](mouseButton& btn) { \
        if (btn.Status() & MOUSE_UP) { \
            btn.SetStatus(MOUSE_OUT); \
            g_status |= 0x01; \
        } \
    }); \
    std::memcpy(feedbackData, (void*)&args.port, sizeof(int)); \
    mouseButton patrnEgg(23, 3, 3, L"tUI"); \
    g_screen.AddButton(&patrnEgg); \
    patrnEgg.setCallback([](mouseButton& btn) { \
        if (btn.Status() & MOUSE_UP) { \
            btn.SetStatus(MOUSE_OUT); \
            if(g_status & 0x02){ \
                g_status &= ~0x02; \
            } \
            else{ \
                PTRN_MAKER(); \
                g_status |= 0x02; \
            } \
        } \
        }); \
    {auto now = std::chrono::system_clock::now(); \
    auto duration = now.time_since_epoch().count() % 69; \
    if(duration == 4 || duration == 20) PTRN_MAKER(); }


#define COLOR_CX_EGG() \
{ \
    colorEgg.SetColors(GET_EGG_COLOR()); \
    patrnEgg.SetColors(colorEgg.GetColors()); \
    g_screen.SetButtonsColorsById(colorEgg.GetColors(), {42}); \
}
 

// ********************************
// tUI Helper Functions

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


void GET_NEW_COLOR_SCHEME() {
    g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES); /* will be used as index for fullColorSchemes */
    if (g_currentColorScheme == RANDOMSCHEME)
    {
        WORD newRandomBG = generateRandomInt(0, 15) AS_BG;
        static ColorScheme randomScheme;
        randomScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(randomScheme, newRandomBG);
    }
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]); /* Set for compatibility with DrawControllerFace */
}

void POPULATE_COLOR_LIST(std::vector<WORD>& colorList) {
    int colorseed = generateRandomInt(1100000000, 2147483647);
    for (int byteIndex = 0; byteIndex < sizeof(int); ++byteIndex) {
        unsigned char byteValue = (colorseed >> (byteIndex * 8)) & 0xFF;
        for (int nibbleIndex = 0; nibbleIndex < 2; ++nibbleIndex) {
            unsigned char nibble = (byteValue >> (nibbleIndex * 4)) & 0x0F;
            colorList.push_back(nibble);
        }
    }
}

tUIColorPkg GET_EGG_COLOR() {
    std::vector<WORD> colorList; \
        POPULATE_COLOR_LIST(colorList); \
        WORD col3 = CheckContrastMismatch(fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY, fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) ? \
        (findSafeFGColor(fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY, colorList, colorList.begin()) | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) : \
        (fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY); \
        return tUIColorPkg({ fullColorSchemes[g_currentColorScheme].menuBg, fullColorSchemes[g_currentColorScheme].menuBg, col3, col3 });
}

void BUILD_CONNECTION_tUI() {
    g_screen.SetBackdrop(JoyRecvMain_Backdrop);
    quitButton.SetPosition(11, 17);
    g_screen.AddButton(&quitButton);
    errorOut.SetPosition(consoleWidth / 2 + 1, 5, 50, 0, ALIGN_CENTER);
    output1.SetPosition(consoleWidth / 2 + 2, 7, 50, 1, ALIGN_CENTER);
    output1.SetText(L" Waiting For Connection ");
}

void COLOR_AND_DRAW_CX_tUI(int port) {
    WORD headingColor = makeSafeColors(fullColorSchemes[g_currentColorScheme].controllerColors.col4);
    output2.SetColor(headingColor); /* for connection animation */
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
    g_screen.ReDraw();
    errorOut.Draw();
    output1.SetColor(headingColor);
    output1.Draw();

    /* Show PORT and IPs */
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
    setCursorPosition(26, 9);
    std::wcout << L" ";
    std::wcout << port;
    std::wcout << L" ";

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
    setCursorPosition(28, 11);
    wprintf(L" %S ", localIP);

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
    setCursorPosition(28, 13);
    wprintf(L" %S ", externalIP);
}

void REDRAW_MAIN_BACKDROP_TEXT() {
    no_whitespace_Draw(JoyRecvMain_Backdrop, 0, 0, 74, 1555);
    setCursorPosition(50, 17);
    std::wcout << L"©░Quinnco.░2024";

    errorOut.Draw();
    output1.Draw();
    g_screen.DrawButtons();

    /* Show PORT and IPs */
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
    setCursorPosition(26, 9);
    std::wcout << L" ";
    std::wcout << *(int*)feedbackData;
    std::wcout << L" ";

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
    setCursorPosition(28, 11);
    wprintf(L" %S ", localIP);

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
    setCursorPosition(28, 13);
    wprintf(L" %S ", localIP);
}

void MAKE_PATTERNS() {
    srand(static_cast<unsigned int>(time(0)));
    std::vector<const wchar_t*> blocks = { L"▓", L"▒", L"█", L"▓", L"░", L"▒" };
    std::vector<int> numbers = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(numbers.begin(), numbers.end(), rng);
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    std::shuffle(blocks.begin(), blocks.end(), rng);
    int dir = (numbers[2] > 13) ? 0 : -1;
    printDiagonalPattern(2, 19, 5, numbers[0] + dir * 7, 68, -7, L"▒", dir);
    printDiagonalPattern(2, 19, 5, numbers[1] + dir * 7, 68, -7, L"▒", dir);
    printDiagonalPattern(2, 19, 5, numbers[2] + dir * 7, 68, -7, L" ", dir);
    printDiagonalPattern(2, 19, 5, numbers[3] + dir * 7, 68, -7, L"▓", dir);
    printDiagonalPattern(2, 19, 5, numbers[4] + dir * 7, 68, -7, L"▒", dir);
    printDiagonalPattern(2, 19, 5, numbers[5] + dir * 7, 68, -7, L"▒", dir);
    printDiagonalPattern(2, 19, 5, numbers[6] + dir * 7, 68, -7, L"▒", dir);
    printDiagonalPattern(2, 19, 5, numbers[7] + dir * 7, 68, -7, blocks[3], dir);
    printDiagonalPattern(2, 19, 5, numbers[8] + dir * 7, 68, -7, L"░", dir);
    printDiagonalPattern(2, 19, 5, numbers[9] + dir * 7, 68, -7, L"▒", dir);
    if (!(g_status & 0x10)) {
        g_status |= 0x10;
        wchar_t* rightBorderTextPtr = (wchar_t*)g_extraData;
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"= ", L"══", 30);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"=", L"═", 1);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"= ", L"┉┉", 31);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"=", L"┉", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 2], sizeof(JoyRecvMain_Backdrop), L"||", L"◊┋", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth) * 3], sizeof(JoyRecvMain_Backdrop), L"||", L"┋◊", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 3], sizeof(JoyRecvMain_Backdrop), L"||", L"╣┋", 25, 5);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth - 1) * 4], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╠", 19, 4);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 17], sizeof(JoyRecvMain_Backdrop), L"||", L"♥┋", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth) * 18], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╳", 1);
        if (numbers[7] < 11) {
            replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 3], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╠", 25, 3);
            replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth - 1) * 4], sizeof(JoyRecvMain_Backdrop), L"||", L"╣┋", 19);
            replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 2], sizeof(JoyRecvMain_Backdrop), L"◊", L"♣", 2);
            replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 2], sizeof(JoyRecvMain_Backdrop), L"┋", L"╠", 1);
            replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth) * 3], sizeof(JoyRecvMain_Backdrop), L"┋", L"╣", 1);
        }
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 19], sizeof(JoyRecvMain_Backdrop), L"-{", L"◄∑", 1);
        replaceXEveryNth(&FooterAnimation[1][2], 31, L".", L"+", 1);
        replaceXEveryNth(FooterAnimation[2], 31, L"*", L"┄", 1);
        replaceXEveryNth(FooterAnimation[8], 31, L"*", L"+", 1);
        replaceXEveryNth(rightBorderTextPtr, 20, L"}-", L"∫►", 1);
        mouseButton* hartEgg = new mouseButton(3, consoleHeight-3, 3, L"♥"); 
        g_screen.AddButton(hartEgg);
        hartEgg->SetId(42);
        hartEgg->SetColors(GET_EGG_COLOR());
        hartEgg->setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                if (!(g_status & 0x04)) {
                    g_status |= 0x04;
                    replaceXEveryNth(&FooterAnimation[1][2], 31, L"+", L"♥", 1);
                    replaceXEveryNth(FooterAnimation[2], 31, L"┄", L"+", 1);
                    replaceXEveryNth(FooterAnimation[8], 31, L"+", L"♥", 1);
                    for (int i = 0; i < FOOTER_ANI_FRAME_COUNT; i++) {
                        replaceXEveryNth(&FooterAnimation[i][10], 31, L"\\", L"♥", 1);
                    }
                }
            }
            });
    }
}

void PTRN_MAKER() {
    MAKE_PATTERNS();
    REDRAW_MAIN_BACKDROP_TEXT();
}

void JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION(TCPConnection& server, Arguments& args, int& allGood, char* connectionIP) {
    /* Set up animation variables */
    int frameDelay = 0;
    int footFrameNum = 0;
    wchar_t rside[] = L"\t/   \t\t:}-     ";
    mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t");
    mouseButton rightBorderPiece(68, 18, 5, rside);
    leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
    rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);

    mouseButton ani2(3, 18, 10, FooterAnimation[footFrameNum]);
    ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
    int lastAni1Frame = g_frameNum - 1;
    int lastAni2Frame = footFrameNum - 1;

    g_extraData = (void*)&rside;
    BUILD_CX_EGG();
    COLOR_CX_EGG();
    colorEgg.Draw();
    patrnEgg.Draw();

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
        if (g_status & 0x01) {
            g_status &= ~0x01;
            roll_new_color();
            COLOR_AND_DRAW_CX_tUI(args.port);

            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);

            COLOR_CX_EGG();
            if (g_status & 0x02) PTRN_MAKER();
        }
        screenLoop(g_screen);

        if (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
            Sleep(20);
            ++frameDelay;
        }
    }
    connectThread.detach();
    g_screen.RemoveButton(&colorEgg);
    g_screen.RemoveButton(&patrnEgg);
    g_screen.DeleteButton(42);
    //g_status = g_status & 0x12;
    memset(feedbackData, 0, sizeof(feedbackData));

    if (!APP_KILLED) {
        /* return to blocking mode */
        server.set_client_blocking(true);
        server.set_server_blocking(true);
    }
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
    newColorsButton.SetPosition(consoleWidth / 2 - 8, QUITLINE - 2);
    g_screen.AddButton(&quitButton);
    g_screen.AddButton(&newColorsButton);
    /* Colors and drawing */
    if (g_status & 0x02) MAKE_PATTERNS();
    DrawControllerFace(g_screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, g_mode, (g_status & 0x02));
    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
    /* Color non-controller buttons and draw them */
    g_screen.SetButtonsColors(buttonColors);
    g_screen.DrawButtons();
    output1.Draw();
    clientMsg.Draw();
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
        if (g_status & 0x02) MAKE_PATTERNS();
        DrawControllerFace(g_screen, g_simpleScheme, newRandomBG, g_mode, (g_status & 0x02));

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