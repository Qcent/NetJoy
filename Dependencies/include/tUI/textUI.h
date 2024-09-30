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