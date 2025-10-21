/*

Copyright (c) 2025 Dave Quinn <qcent@yahoo.com>

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

#include <io.h>
#include <fcntl.h>
#include <map>
#include <set>
#include <vector>
#include <fstream>

#include "textUI.hpp"
#include "backdrops.h"
#include "uiButtons.h"
#include "controllerFaces.h"
#include "fullColorSchemes.h"


constexpr int consoleWidth = 72;
constexpr int consoleHeight = 20;

HANDLE g_hConsoleInput;
textUI g_screen;
byte g_mode = 1;

wchar_t g_stringBuff[500];      // for holding generated text data
wchar_t* errorPointer = g_stringBuff;   // max length 100
wchar_t* cxPointer = g_stringBuff + 100;    // max length 25
wchar_t* fpsPointer = cxPointer + 25;   // max length 10
wchar_t* msgPointer1 = fpsPointer + 10;   // max length 100
wchar_t* msgPointer2 = msgPointer1 + 100;   // max length 100
wchar_t* msgPointer3 = msgPointer2 + 100;   // max length 165

textBox errorOut(4, 22, 45, 0, ALIGN_CENTER, errorPointer, BRIGHT_RED);
textBox cxMsg(1, 8, 20, 0, ALIGN_LEFT, cxPointer, BRIGHT_GREEN);
textBox fpsMsg(1, 9, 20, 0, ALIGN_LEFT, fpsPointer, BRIGHT_CYAN);
textBox output1(3, 19, 60, 0, ALIGN_LEFT, msgPointer1, BRIGHT_BLUE);
textBox output2(3, 21, 60, 0, ALIGN_LEFT, msgPointer2, BRIGHT_BLUE);
textBox output3(3, 24, 60, 0, ALIGN_LEFT, msgPointer3, BRIGHT_BLUE);


#include "DrawUtils.h"
#include "animations.h"
#include "extraArt.h"
#include "theme.h"

//----------------------------------------------------------------------------
// lifted from : https://cplusplus.com/forum/windows/10731/
struct console
{
    console(unsigned width, unsigned height)
    {
        SMALL_RECT r;
        COORD      c;
        hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConOut, &csbi);

        r.Left = r.Top = 0;
        r.Right = width;
        r.Bottom = height;
        SetConsoleWindowInfo(hConOut, TRUE, &r);

        c.X = width;
        c.Y = height;
        SetConsoleScreenBufferSize(hConOut, c);

        HWND consoleWindow = GetConsoleWindow();
        SetWindowLong(consoleWindow, GWL_STYLE, GetWindowLong(consoleWindow, GWL_STYLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
    }

    ~console()
    {
        SetConsoleTextAttribute(hConOut, csbi.wAttributes);
        SetConsoleScreenBufferSize(hConOut, csbi.dwSize);
        SetConsoleWindowInfo(hConOut, TRUE, &csbi.srWindow);
    }

    HANDLE                     hConOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
};

#include "themeEditor.hpp"

// ********************************
// tUI Helper Functions
bool tUI_INIT_CONSOLE(){
    DWORD inMode = 0, outMode = 0;
    DetermineConsoleMode(inMode, outMode);

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    g_hConsoleInput = hIn;
    
    // Disable Quick Edit and enable mouse
    inMode &= ~ENABLE_QUICK_EDIT_MODE;
    inMode |= ENABLE_MOUSE_INPUT;
    SetConsoleMode(g_hConsoleInput, inMode);

    // UTF-8 output mode
    if (_setmode(_fileno(stdout), _O_U8TEXT) == -1)
        return false;

    if (CONSOLE_MODE == CONSOLE_HOST_MODE) {
        // Change font if using classic Console Host
        CONSOLE_FONT_INFOEX cfi = { sizeof(cfi) };
        if (GetCurrentConsoleFontEx(hOut, FALSE, &cfi)) {
            wcscpy_s(cfi.FaceName, L"Cascadia Code SemiBold");
            cfi.dwFontSize.Y = 20;
            SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
        }
    }
    else if(CONSOLE_MODE == CONSOLE_VT_MODE) {

        outMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, outMode);

        // turn off line buffering and echo
        inMode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
        inMode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
        SetConsoleMode(hIn, inMode);

        wprintf(L"\x1b[?1006h"); // Enable SGR mouse reporting
        wprintf(L"\x1b[?1003h"); // Any-motion mouse tracking
        wprintf(L"\x1b[?1004h"); // enable focus tracking

        fflush(stdout);


        /* VT NOT FULLY WORKING */
        return prompt_to_relaunch_with_consoleHost();
        /* -------------------- */
    }

    return true;
}

struct MouseEvent {
    byte type;
    int x;
    int y;
    int button; // 0=left, 1=middle, 2=right
};

bool ReadVTMouseEvent(MouseEvent& evt)
{
    char buf[64];
    int bytesRead = _read(0, buf, sizeof(buf) - 1); // file descriptor 0 = stdin

    if (bytesRead <= 0)
        return false;

    buf[bytesRead] = '\0';
    std::string s(buf, bytesRead);

    // Focus
    if (s.find("\x1b[I") != std::string::npos) { g_hasFocus = true;  return false; }
    if (s.find("\x1b[O") != std::string::npos) { g_hasFocus = false; return false; }

    // --- SGR mouse: ESC [ < b ; x ; y M/m ---
    std::smatch m;
    std::regex re("\x1b\\[<([0-9]+);([0-9]+);([0-9]+)([Mm])");

    if (std::regex_search(s, m, re)) {
        int b = std::stoi(m[1]);
        evt.x = std::stoi(m[2])-1;
        evt.y = std::stoi(m[3])-1;
        bool release = (m[4] == "m");
        bool motion = (b & 32);

        if (motion)       evt.type = MOUSE_MOVED;
        else if (release) evt.type = MOUSE_UP;
        else              evt.type = MOUSE_DOWN;

        evt.button = (b & 3);
        return true;
    }

    return false;
}

int screenLoopVirtualTerminal(textUI& screen)
{
    int retVal = 0;
    MouseEvent evt;
    if (ReadVTMouseEvent(evt)) {
        COORD mousePos = { (SHORT)evt.x, (SHORT)evt.y };

        if (evt.type == MOUSE_MOVED) {
            screen.CheckMouseHover(mousePos);
        }
        else if (evt.type == MOUSE_DOWN) {
            screen.CheckMouseClick(MOUSE_DOWN);
            retVal = MOUSE_DOWN;
        }
        else if (evt.type == MOUSE_UP) {
            screen.CheckMouseClick(MOUSE_UP);
            retVal = MOUSE_UP;
        }
    }
    return retVal;    
}

int screenLoopConsoleHost(textUI& screen) {
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

// used in a loop, looks for mouse input and compares to buttons on screen
inline int screenLoop(textUI& screen)
{
    if (CONSOLE_MODE == CONSOLE_VT_MODE)
        return screenLoopVirtualTerminal(screen);
    else
        return screenLoopConsoleHost(screen);
}

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
    updateButtonStatus(button_LStickRight_highlight, ds4_report[0] > 156);  // right

    ds4_report++;
    updateButtonStatus(button_LStickDown_highlight, ds4_report[0] > 156);  // down
    updateButtonStatus(button_LStickUp_highlight, ds4_report[0] < 100);   // up

    // Right Stick
    ds4_report++;
    updateButtonStatus(button_RStickLeft_highlight, ds4_report[0] < 100);   // left
    updateButtonStatus(button_RStickRight_highlight, ds4_report[0] > 156);  // right

    ds4_report++;
    updateButtonStatus(button_RStickDown_highlight, ds4_report[0] > 156);  // down
    updateButtonStatus(button_RStickUp_highlight, ds4_report[0] < 100);   // up

    ds4_report++;
    // Main Face Buttons
    updateButtonStatus(button_A_outline, ds4_report[0] & 0x20);   // A
    updateButtonStatus(button_B_outline, ds4_report[0] & 0x40);   // B
    updateButtonStatus(button_X_outline, ds4_report[0] & 0x10);   // X
    updateButtonStatus(button_Y_outline, ds4_report[0] & 0x80);   // Y

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
    updateButtonStatus(button_Guide_highlight, ds4_report[0] & 0x01);  // ps button
    updateButtonStatus(button_DS4_TouchPad_highlight, ds4_report[0] & 0x02);  // ds4 touchpad



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