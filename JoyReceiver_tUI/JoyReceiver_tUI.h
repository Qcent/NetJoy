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
        ColorScheme menuScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(g_simpleScheme, menuScheme, newRandomBG);
        g_currentColorScheme = RANDOMSCHEME;

        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].controllerBg);

        // draw messages
        g_status |= RECOL_tUI_f | REFLAG_tUI_f;
        g_status &= ~tUI_THEME_af;
    }
}


// ^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^
// tUI Helper Functions

#define JOYRECEIVER_tUI_INIT_FOOTER_ANIMATION() \
mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t"); \
mouseButton rightBorderPiece(68, 18, 5, rsideFooter); \
leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
int frameDelay = 0; \
int footFrameNum = 0; \
mouseButton ani2(3, 18, 10, FooterAnimation[footFrameNum]); \
ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
int lastAni1Frame = g_frameNum - 1; \
int lastAni2Frame = footFrameNum - 1; 


#define JOYRECEIVER_tUI_ANIMATE_CONNECTION(){ \
        /* Animation 1 */ \
if (lastAni2Frame != g_frameNum) { \
    lastAni2Frame = g_frameNum; \
    output2.SetText(ConnectAnimationLeft[g_frameNum]); \
    output2.SetPosition((consoleWidth / 2) - 16, 7); \
    output2.Draw(); \
    output2.SetText(ConnectAnimationRight[g_frameNum]); \
    output2.SetPosition((consoleWidth / 2) + 15, 7); \
    output2.Draw(); \
} \
if (frameDelay % 13 == 0) { \
    loopCount(g_frameNum, CX_ANI_FRAME_COUNT); \
} \
        /* Animation 2*/ \
if (lastAni2Frame != footFrameNum) { \
    lastAni2Frame = footFrameNum; \
    ani2.SetText(FooterAnimation[footFrameNum]); \
    for (int i = 0; i < 7; ++i) { \
        ani2.SetPosition(3 + (i * 10), 18); \
        ani2.Draw(); \
    } \
    leftBorderPiece.Draw(); \
    rightBorderPiece.Draw(); \
} \
if (frameDelay % 4 == 0) {    \
    if ((frameDelay % 600) <= 300) { revLoopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT); } \
    else { loopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT); } \
}\
++frameDelay; \
}


// to allow for screen loop and animation, wait for connection to client in a separate thread
void threadedAwaitConnection(TCPConnection& server, int& retVal, char* clientIP) {
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

void SET_tUI_CONSOLE_MODE(){ 
    /* Get the console input handle to enable mouse input */ 
    g_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode; 
    GetConsoleMode(g_hConsoleInput, &mode); 
    /* Disable Quick Edit Mode */ 
    SetConsoleMode(g_hConsoleInput, ENABLE_MOUSE_INPUT | (mode & ~ENABLE_QUICK_EDIT_MODE)); 

    /* Set the console to UTF-8 mode */ 
    auto _ = _setmode(_fileno(stdout), _O_U8TEXT); 

    /* Set Version into window title */ 
    wchar_t winTitle[30]; 
    wcscpy_s(winTitle, L"JoyReceiver++ tUI "); 
    wcscat_s(winTitle, APP_VERSION_NUM); 
    SetConsoleTitleW(winTitle); 

    hideConsoleCursor(); 

    /* Set Version into backdrop */ 
    constexpr int versionStartPoint = 73 * 3 + 31; 
    const int verLength = wcslen(APP_VERSION_NUM); 
    for (int i = 0; i < verLength; i++) { 
        JoyRecvMain_Backdrop[versionStartPoint + i] = APP_VERSION_NUM[i]; } 
}

void JOYRECEIVER_INIT_tUI_SCREEN(){
    CoupleControllerButtons(); 
    setErrorMsg(L"\0", 1); 
    fpsMsg.SetPosition(50, 1, 7, 1, ALIGN_LEFT); 
    quitButton.setCallback(&exitAppCallback); 
    newColorsButton.setCallback(&newControllerColorsCallback); 

    /* establish a color scheme */ 
    GET_NEW_COLOR_SCHEME(); 
    errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); 
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
} 

void JOYRECEIVER_tUI_BUILD_CONNECTION() {
    g_screen.SetBackdrop(JoyRecvMain_Backdrop);
    quitButton.SetPosition(11, 17);
    g_screen.AddButton(&quitButton);
    errorOut.SetPosition(consoleWidth / 2 + 1, 5, 50, 0, ALIGN_CENTER);
    output1.SetPosition(consoleWidth / 2 + 2, 7, 50, 1, ALIGN_CENTER);
    output1.SetText(L" Waiting For Connection ");
}

void REDRAW_CX_TEXT() {
    tUI_DRAW_BG_AND_BUTTONS();
    errorOut.Draw();
    output1.Draw();
    g_screen.DrawButtons();
    PRINT_EGG_X();

    /* Show PORT and IPs */
    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
    setCursorPosition(26, 9);
    wprintf_s(L" %d ", *(int*)feedbackData);

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
    setCursorPosition(28, 11);
    wprintf_s(L" %S ", localIP.c_str());

    setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
    setCursorPosition(28, 13);
    wprintf_s(L" %S ", externalIP.c_str());
}

void JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION(TCPConnection& server, Arguments& args, int& allGood, char* connectionIP) {
    /* Set up animation variables */
    JOYRECEIVER_tUI_INIT_FOOTER_ANIMATION();

    std::memcpy(feedbackData, (void*)&args.port, sizeof(int)); // you wanna get nuts? let's get nutz
    
    auto roll_new_color = [&]() {
        GET_NEW_COLOR_SCHEME();
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
        };

    auto re_color = [&]() {
        WORD headingColor = makeSafeColors(fullColorSchemes[g_currentColorScheme].controllerColors.col4);
        output2.SetColor(headingColor); /* for connection animation */
        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
        g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        output1.SetColor(headingColor);
        COLOR_EGGS();
        leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        };

    tUI_LOAD_THEME();
    tUI_THEME_BUTTONS();
    mouseButton* applyTheme = g_screen.GetButtonById(46);
    mouseButton* saveTheme = g_screen.GetButtonById(47);
    mouseButton colorEgg(50, consoleHeight - 3, 1, L"©");
    mouseButton borderEgg(23, 3, 3, L"tUI");
    {
        colorEgg.SetId(43);
        g_screen.AddButton(&colorEgg);
        colorEgg.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {

                btn.SetStatus(MOUSE_OUT);
                g_status |= COLOR_EGG_b;
            }
            });
        
        borderEgg.SetId(44);
        borderEgg.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                if (!(g_status & BORDER_EGG_a)) {

                    MORPH_BORDER();
                    if (generateRandomInt(1, 5) > 4) {

                        MAKE_PATTERNS();
                        g_status |= PTRN_EGG_b;
                    }
                    HEARTS_N_SPADES_BUTTONS();
                    g_status |= RECOL_tUI_f | REDRAW_tUI_f;
                }
            }
            });
        g_screen.AddButton(&borderEgg);
    }

    tUI_THEME_AUTO_ACTIVATION();
    tUI_THEME_RESTART();

    g_screen.DrawBackdrop(); // for clear whitespace updates we need to laydown some color

    /* Set up thread */
    allGood = WSAEWOULDBLOCK;
    std::thread connectThread(threadedAwaitConnection, std::ref(server), std::ref(allGood), std::ref(connectionIP));

    while (!APP_KILLED && allGood == WSAEWOULDBLOCK) {

        JOYRECEIVER_tUI_ANIMATE_CONNECTION();

        /* Check input */
        checkForQuit();
        /*
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
        */

        screenLoop(g_screen);
        tUI_UPDATE_INTERFACE(re_color, REDRAW_CX_TEXT);

        if (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
            Sleep(20);
        }
    }
    connectThread.detach();
    g_screen.ClearButtonsExcept(HEAP_BTN_IDs);
    memset(feedbackData, 0, sizeof(feedbackData));

    if (!APP_KILLED) {
        /* return to blocking mode */
        server.set_client_blocking(true);
        server.set_server_blocking(true);
        server.set_client_timeout(NETWORK_TIMEOUT_MILLISECONDS);
    }
}

int JOYRECEIVER_tUI_WAIT_FOR_CLIENT_MAPPING(TCPConnection& server, char* buffer, int buffer_size) {
    int bytesReceived = 0, counter = 0, len = 43;
    int lastAniFrame = g_frameNum - 1, bgDrawCount = 0;

    // Setup
    swprintf(errorPointer, len, L" << Client Is Mapping Their Controller >> ", connectionIP);
    errorOut.SetWidth(len);
    errorOut.SetText(errorPointer);
    errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
    COORD oldErrorPos = errorOut.GetPosition();
    errorOut.SetPosition(oldErrorPos.X, oldErrorPos.Y+2);

    swprintf(fpsPointer, 8, L"  --  ");
    fpsMsg.SetText(fpsPointer);

    tUITheme bgPtrn;
    bgPtrn.setColors(fullColorSchemes[g_currentColorScheme]);
    
    auto cleanUp = [&]() {
        server.set_client_blocking(true);
        errorOut.SetPosition(oldErrorPos);
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        setErrorMsg(L"\0", 1);
        if (!(g_status & BORDER_EGG_a)) {
            tUI_CLEAR_PTRN_AREA(fullColorSchemes[g_currentColorScheme].controllerBg);
        }
        };

    server.set_client_blocking(false);

    // Draw bg
    setTextColor(DIAMOND_COLOR());
    drawPatternedArea({ 0,0,consoleWidth, consoleHeight });
    bgPtrn.recordBlocks();

    auto draw_objects = [&]() {
        errorOut.Draw();
        fpsMsg.Draw();
        output1.Draw();
        cxMsg.Draw();
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
            tUI_RECOLOR_MAIN_LOOP();
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
                bgPtrn.drawPtrnDiag(bgDrawCount, {0,0,consoleWidth, consoleHeight+1});
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