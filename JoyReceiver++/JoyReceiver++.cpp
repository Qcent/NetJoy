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

#include "ArgumentParser.hpp"
#include "TCPConnection.h"
#include "FPSCounter.hpp"
#include "JoyReceiver++.h"

void overwriteFPS(const std::string& text) {
    // Move the cursor to the beginning of the last line
    std::cout << "\033[F";
    // Write the new text
    std::cout << text + "  " << std::endl;
}
void overwriteLatency(const std::string& text) {
    repositionConsoleCursor(-1, 27);
    std::cout << text << std::endl;
}


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
    double latencyOutput;
    auto do_latency_timing = [&latencyTimer](int report_frequency = 25) {
        if (latencyTimer.increment_frame_count() >= report_frequency) {
            double elapsedTime = latencyTimer.get_elapsed_time();
            latencyTimer.reset();
            return elapsedTime / report_frequency;
        }
        return 0.0;
    };

    // Register the signal handler
    std::signal(SIGINT, signalHandler);

    JOYRECEIVER_INIT_VIGEM_BUS();
    JOYRECEIVER_DETERMINE_IPS_START_SERVER();

    // Set Up Console
    hideConsoleCursor();
    std::system("cls");
    ///********************************
    // Make Connection -> Receive Input Loop
    while (!APP_KILLED) {
        std::cout << "Waiting for Connection on port : " << args.port
            << "\n\t\t LAN : " << localIP << "\n\t\t WAN : " << externalIP << std::endl;

        // Await Connection in Non Blocking Mode
        JOYRECEIVER_CONSOLE_AWAIT_CONNECTION();

        if (APP_KILLED) break;

        // Receive Operating Mode and Client Timing
        bytesReceived = server.receive_data(buffer, buffer_size);
        if (bytesReceived < 1) {
            std::cout << "<< Connection (" << connectionIP << ") Failed >> " << std::endl;
            break;
        }
        
        JOYRECEIVER_GET_MODE_AND_TIMING_FROM_BUFFER(buffer, bytesReceived, client_timing, op_mode, expectedFrameDelay);
        if (op_mode == -1) break;
        std::cout << "<< Connection (" << connectionIP << ") Received >> \r\n";
        std::cout << "  Emulating " << ((op_mode == 2) ? "DS4" : "XBOX") << " Controller @ " << client_timing << "fps" << std::endl;
        JOYRECEIVER_PLUGIN_VIGEM_CONTROLLER();

        // Send response back to client
        allGood = server.send_data("Go for Joy!", 12);
        if (allGood < 1) {
            std::cout << "<< Connection (" << connectionIP << ") Failed >>" << std::endl;
            break;
        }
        
        // Prep UI for loop
        std::cout << std::endl << std::endl;
        fps_counter.reset();

        /* Start Receive Joystick Data Loop */
        while (!APP_KILLED) {
            //*****************************
            // Receive joystick input from client to the buffer
            bytesReceived = server.receive_data(buffer, buffer_size);
            if (bytesReceived < 1) {
                break;
            }

            //******************************
            // Update virtual gamepad
            if (op_mode == 2) {
                // Cast the buffer to an DS4_REPORT_EX pointer
                ds4_report_ex = *reinterpret_cast<DS4_REPORT_EX*>(buffer);
                vigem_target_ds4_update_ex(vigemClient, gamepad, ds4_report_ex);
            }
            else {
                // Cast the buffer to an XUSB_REPORT pointer
                xbox_report = *reinterpret_cast<XUSB_REPORT*>(buffer);
                vigem_target_x360_update(vigemClient, gamepad, xbox_report);
            }

            //*******************************
            // Send response back to client :: Rumble + lightbar data
            lock.lock();
            allGood = server.send_data(feedbackData, 5);
            lock.unlock();
            if (allGood < 1) {
                break;
            }

            // FPS output
            if (args.latency) {
                fpsOutput = do_fps_counting();
                if (!fpsOutput.empty()) {
                    overwriteFPS("FPS: " + fpsOutput);
                }
                latencyOutput = do_latency_timing();
                if (latencyOutput) {
                    overwriteLatency("Latency: " + formatDecimalString(std::to_string(((latencyOutput * 1000) - expectedFrameDelay) / 2), 5) + " ms    ");
                }
            }
        }
        /* End of Receive Joystick Data Loop */
        
        if (!APP_KILLED) {
            std::system("cls");
            std::cout << "<< Connection (" << connectionIP << ") Lost >>" << std::endl;
        }

        // Unregister rumble notifications // unplug virtual deveice
        JOYRECEIVER_UNPLUG_VIGEM_CONTROLLER();
    }

    JOYRECEIVER_SHUTDOWN_VIGEM_BUS();
    swallowInput();
    showConsoleCursor();
    
    return 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\r\n<< Exiting >>" << std::endl;
        APP_KILLED = 1;
        Sleep(5);
    }
}