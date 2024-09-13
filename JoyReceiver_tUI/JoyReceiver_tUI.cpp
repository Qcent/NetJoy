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

    auto do_fps_counting = [&fps_counter](int report_frequency = 30) {
        if (fps_counter.increment_frame_count() >= report_frequency) {
            double fps = fps_counter.get_fps();
            fps_counter.reset();
            return formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
        };

    // Register the signal handler
    std::signal(SIGINT, signalHandler);

    JOYRECEIVER_INIT_VIGEM_BUS();
    if (APP_KILLED) {
        return -1;
    }
    JOYRECEIVER_DETERMINE_IPS_START_SERVER();

    // Set Up Console
    SET_tUI_CONSOLE_MODE();
    INIT_tUI_SCREEN();
    ///********************************
    // Make Connection -> Receive Input Loop
    while (!APP_KILLED) {
        BUILD_CONNECTION_tUI();
        //COLOR_AND_DRAW_CX_tUI(args.port);

        // Await Connection in separate thread while animating the screen
        JOYRECEIVER_tUI_AWAIT_ANIMATED_CONNECTION(server, args, allGood, connectionIP);

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
        g_mode = op_mode;
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
        BUILD_MAIN_LOOP_tUI(connectionIP);
        fps_counter.reset();

        /* Start Receive Joystick Data Loop */
        while (!APP_KILLED) {
            EGG_LOOP();
            screenLoop(g_screen);

            // Catch hot key button presses
            if (getKeyState(VK_SHIFT) && IsAppActiveWindow()) {
                if (checkKey('C', IS_PRESSED)) {  // change colors
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
                if (WSAGetLastError() == WSAETIMEDOUT) {
                    bytesReceived = JOYRECEIVER_tUI_WAIT_FOR_CLIENT_MAPPING(server, buffer, buffer_size);
                    fps_counter.reset();
                    g_status |= REDRAW_tUI_f;
                }
                else {
                    int len = INET_ADDRSTRLEN + 31;
                    swprintf(errorPointer, len, L" << Connection From: %S Lost >> ", connectionIP);
                    errorOut.SetWidth(len);
                    errorOut.SetText(errorPointer);
                    break;
                }
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
            // Send response back to client :: Rumble + lightbar data
            lock.lock();
            allGood = server.send_data(feedbackData, 5);
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
        
        CLEAN_EGGS();
        if (!APP_KILLED) {
            g_screen.ClearButtons();
            g_status |= tUI_RESTART_f;
        }

        // Unregister rumble notifications // unplug virtual deveice
        JOYRECEIVER_UNPLUG_VIGEM_CONTROLLER();
    }

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