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
#define NetJoyTUI

#include "./../JoyReceiver++/ArgumentParser.hpp"
#include "TCPConnection.h"
#include "FPSCounter.hpp"
#include "JoyReceiver_tUI.h"

console con(consoleWidth, consoleHeight);

int main(int argc, char* argv[]) {
    JOYRECEIVER_INIT_VARIABLES();

    char connectionIP[INET_ADDRSTRLEN];

    auto do_fps_counting = [&fps_counter](int report_frequency = 30) {
        if (fps_counter.increment_frame_count() >= report_frequency) {
            double fps = fps_counter.get_fps();
            fps_counter.reset();
            return formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
        };

    // Register the signal handler function
    std::signal(SIGINT, signalHandler);

    // Initialize ViGEM Bus connection
    JOYRECEIVER_INIT_VIGEM_BUS();

    // Quit if error occurred
    if (APP_KILLED) {
        return -1;
    }

    // Get IPs and start server
    JOYRECEIVER_DETERMINE_IPS_START_SERVER();

    // Set Up Console Window
    SET_tUI_CONSOLE_MODE();

    //
    // Set up for tUI screen
        // set element properties
    textUI& screen = g_screen;
    CoupleControllerButtons(); // sets up controller outline/highlight coupling for nice looks & button ids
    setErrorMsg(L"\0", 1); // initialize error message as empty string
    fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT);                // fps output
    quitButton.setCallback(&exitAppCallback);
    newColorsButton.setCallback(&newControllerColorsCallback);

        // establish a color scheme
    GET_NEW_COLOR_SCHEME();
    errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
        
    ///********************************

    // Make Connection Loop
    while (!APP_KILLED) {
        // Set and Draw connection screen
        BUILD_CONNECTION_tUI();
        COLOR_AND_DRAW_CX_tUI();

        // Await Connection in separate thread while animating the screen
        JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION();

        // Possible errors handling
        if (allGood == WSAEINVAL) {
            swprintf(errorPointer, 52, L" Unable to use port : %d \r\n Program will exit", args.port);
            MessageBox(NULL, errorPointer, L"JoyReceiver Error", MB_ICONERROR | MB_OK);
        }
        if (APP_KILLED) break;
        if (allGood < 0) {
            setErrorMsg(L" << Error While Establishing Connection >> ", 44);
            break;
        }

        // Receive Operating Mode and Client Timing
        bytesReceived = server.receive_data(buffer, buffer_size);
        if (bytesReceived < 1) {
            int len = INET_ADDRSTRLEN + 33;
            swprintf(errorPointer, len, L" << Connection From: %S Failed >> ", connectionIP);
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
            break;
        }

        JOYRECEIVER_GET_MODE_AND_TIMING_FROM_BUFFER();

        JOYRECEIVER_PLUGIN_VIGEM_CONTROLLER();

        // Send response back to client
        allGood = server.send_data("Go for Joy!", 12);
        if (allGood < 1) {
            int len = INET_ADDRSTRLEN + 30;
            swprintf(errorPointer, len, L" << Connection To: %S Failed >> ", connectionIP);
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
            break;
        }

        // Prep UI for loop
        BUILD_MAIN_LOOP_tUI();
        fps_counter.reset();

        /* Start Receive Joystick Data Loop */
        while (!APP_KILLED) {
            screenLoop(screen);

            // Catch hot key button presses
            if (IsAppActiveWindow() && getKeyState(VK_SHIFT)) {
                if (checkKey('C', IS_PRESSED)) {
                    // change colors
                    button_Guide_highlight.SetStatus(MOUSE_UP);
                    newControllerColorsCallback(button_Guide_highlight);
                }
                if (getKeyState('Q')) {
                    APP_KILLED = true;
                    break;
                }
            }

            //*****************************
            // Receive joystick input from client to the buffer
            bytesReceived = server.receive_data(buffer, buffer_size);
            if (bytesReceived < 1) {
                int len = INET_ADDRSTRLEN + 31;
                swprintf(errorPointer, len, L" << Connection From: %S Lost >> ", connectionIP);
                errorOut.SetWidth(len);
                errorOut.SetText(errorPointer);
                break;
            }

            //******************************
            // Update virtual gamepad and screen
            if (op_mode == 2) {
                // Cast the buffer to an DS4_REPORT_EX pointer
                ds4_report_ex = *reinterpret_cast<DS4_REPORT_EX*>(buffer);
                vigem_target_ds4_update_ex(vigemClient, gamepad, ds4_report_ex);

                // activate screen buttons from ds4_report_ex
                buttonStatesFromDS4Report(reinterpret_cast<BYTE*>(buffer));
            }
            else {
                // Cast the buffer to an XUSB_REPORT pointer
                xbox_report = *reinterpret_cast<XUSB_REPORT*>(buffer);
                vigem_target_x360_update(vigemClient, gamepad, xbox_report);

                // activate screen buttons from xbox report
                buttonStatesFromXboxReport(xbox_report);
            }

            //*******************************
            // Send response back to client :: Rumble data
            lock.lock();
            allGood = server.send_data(feedbackData, 2);
            lock.unlock();
            if (allGood < 1) {
                int len = INET_ADDRSTRLEN + 29;
                swprintf(errorPointer, len, L" << Connection To: %S Lost >> ", connectionIP);
                errorOut.SetWidth(len);
                errorOut.SetText(errorPointer);
                break;
            }

            // FPS output
            fpsOutput = do_fps_counting();
            if (!fpsOutput.empty()) {
                swprintf(fpsPointer, 8, L" %S   ", fpsOutput.c_str());
                fpsMsg.SetText(fpsPointer);
                fpsMsg.Draw();
            }
        }
        /* End of Receive Joystick Data Loop */

        if (!APP_KILLED) {
            screen.ClearButtons();
        }

        // Unregister rumble notifications // unplug virtual deveice
        JOYRECEIVER_UNPLUG_VIGEM_CONTROLLER();
    }

    // Release connection to the ViGEM Bus
    JOYRECEIVER_SHUTDOWN_VIGEM_BUS();

    swallowInput();
    setCursorPosition(0, consoleHeight);
    showConsoleCursor();
    Sleep(7);

    return 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        APP_KILLED = 1;
        Sleep(5);
    }
}