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

#include "JoySender++.h"
#include "GamepadMapping.hpp"
#include "DS4Manager.hpp"

// For FPS and Latency Output
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

int joySender(Arguments& args) {
    int allGood;
    char buffer[24];
    int buffer_size = sizeof(buffer);
    bool inConnection = false;
    int failed_connections = 0;

    SDLJoystickData activeGamepad;
    std::vector<SDLButtonMapping::ButtonName> activeInputs;
    XUSB_REPORT xbox_report = {0};
    BYTE* hid_report = ds4_InReportBuf;

    // Lambdas and variables for fps/fps-limiting and latency calculations
    FPSCounter fps_counter;
    FPSCounter latencyTimer;
    std::string fpsOutput;
    double loop_delay = 0.0;
    auto do_fps_counting = [&fps_counter, &args, &loop_delay](int report_frequency = 30) {
        // set up some static doubles we will use each frame
        static double averageFrameTime_ms, fps, target_sleep = 1000 / (args.fps);

        int count = fps_counter.increment_frame_count();

        // determine how long to sleep to limit frame rate
        averageFrameTime_ms = (fps_counter.get_elapsed_time() / count) * 1000;
        fps = fps_counter.get_fps();
        if (fps < args.fps)
            loop_delay = target_sleep - averageFrameTime_ms;
        else
            loop_delay = target_sleep;
    
        // output fps to caller
        if (count >= report_frequency) {
            fps_counter.reset();
            return "FPS: " + formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
    };
    double latencyOutput = 0.0;
    auto do_latency_timing = [&latencyTimer](int report_frequency = 25) {
        if (latencyTimer.increment_frame_count() >= report_frequency) {
            double elapsedTime = latencyTimer.get_elapsed_time();
            latencyTimer.reset();
            return elapsedTime / report_frequency; // average time per frame
        }
        return 0.0;
};

    //##########################################################################
    // User or auto select gamepad 
    switch (args.mode) {
    case 1: {   // SDL MODE
        if (!args.select) {
           showConsoleCursor();
           allGood = ConsoleSelectJoystickDialog(activeGamepad);
           if (!allGood) {
               std::cout << " Unable to connect to that device !! " << std::endl;
               return -1;
           }
            hideConsoleCursor();
        }
        else {
            allGood = ConnectToJoystick(0, activeGamepad);
            if (!allGood) {
                std::cout << " Unable to connect to a device !! " << std::endl;
                return -1;
            }
        }
    }
          break;
    case 2: {   // DS4 MODE
        showConsoleCursor();
        allGood = ConnectToDS4Controller();
        if (!allGood) {
            std::cout << " Unable to connect to a DS4 device !! " << std::endl;
            return -1;
        }
        hideConsoleCursor();
    }
          break;
    default:
        std::cout << " ERROR:: Unsupported Mode !! " << std::endl;
        return -1;
    }

    //##########################################################################
    // Init Settings for Operating Mode
    if (args.mode == 2) {      
        // Get a report from the device to determine what type of connection it has
        GetDS4Report();

        // first byte is used to determine where stick input starts
        ds4DataOffset = hid_report[0] == 0x11 ? DS4_VIA_BT : DS4_VIA_USB;

        int attempts = 0;   // DS4 fails to properly initialize when connecting to pc (after power up) via BT so lets hack in multiple attempts
        while (attempts < 2) {
            attempts++;

            Sleep(5); // lets slow things down
            bool extReport = ActivateDS4ExtendedReports();

            // Set up feedback buffer with correct headers for connection mode
            InitDS4FeedbackBuffer();

            // Set new LightBar color with update to confirm rumble/lightbar support
            switch (ds4DataOffset) {
            case(DS4_VIA_BT):
                SetDS4LightBar(105, 4, 32); // hot pink
                break;
            case(DS4_VIA_USB):
                SetDS4LightBar(180, 188, 5); // citrus yellow-green
            }

            Sleep(5); // update fails if controller is bombarded with read/writes, so take a rest bud
            allGood = SendDS4Update();

            g_outputText = "DS4 ";
            if (extReport) g_outputText += "Full Motion ";
            g_outputText += "Mode Activated : ";
            if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
            else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
            if (allGood) g_outputText += " Rumble On | ";
            g_outputText += "\r\n";

            // Rumble the Controller
            if (allGood) {
                // jiggle it
                SetDS4RumbleValue(12, 200);
                SendDS4Update();
                Sleep(110);
                SetDS4RumbleValue(165, 12);
                SendDS4Update();
                // stop the rumble
                SetDS4RumbleValue(0, 0);
                Sleep(130);
                SendDS4Update();
                break; // break out of attempt loop
            }
            // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
            // Problem is probably related to not getting the correct report and assigning ds4DataOffset = DS4_VIA_USB
            // taking the lazy route and just set ds4DataOffset = DS4_VIA_BT this works for me 100% of the time and hasn't led to problems yet ...
            Sleep(10);
            if (ds4DataOffset == DS4_VIA_USB)
                ds4DataOffset = DS4_VIA_BT;
            else
                ds4DataOffset = DS4_VIA_USB;
        }
    }
    else {
        g_outputText = "SDL Mode Activated\r\n";
        BuildJoystickInputData(activeGamepad);
    }

    //##########################################################################
    // If not in DS4 Passthrough mode look for Saved Mapping or create one
    if (args.mode != 2) {
        // Look for an existing map for selected device
        OpenOrCreateMapping(activeGamepad);
        // Create a list of set joystick inputs
        activeInputs = activeGamepad.mapping.getSetButtonNames();
    }
    displayOutputText();
    //##########################################################################
    // Main Loop keeps client running
    // asks for new host if connection fails 3 times
    while (!APP_KILLED) {
        fps_counter.reset();

        // Aquire host address for connection attempt
        if (args.host.empty()) {
            args.host = getHostAddress();
        }
        TCPConnection client(args.host, args.port);
        client.set_silence(true);
        allGood = client.establish_connection();

        // *******************
        // Attempt timing and mode setting handshake
        if (allGood > 0) {
            g_outputText += "<< Connected To : " + args.host + " >>  "; //  \r\n";
            displayOutputText();
            std::cout << std::endl;


            // Send timing and mode data
            std::string txSettings = std::to_string(args.fps) + ":" + std::to_string(args.mode);
            allGood = client.send_data(txSettings.c_str(), static_cast<int>(txSettings.length()));
            if (allGood < 1) {
                g_outputText += "<< Connection Failed >> \r\n";
                displayOutputText();
                break;
            }

            allGood = client.receive_data(buffer, buffer_size);
            if (allGood > 0) {
                inConnection = true;
            }
            else{
                g_outputText += "<< Connection Failed >> \r\n";
                displayOutputText();
                break;
            }

            // Set Lines for FPS and Latency output
            std::cout << std::endl << std::endl;
        }


        // *****************\\
        // Connection loop   ||
        while (inConnection){
            // Shift + R will Reset program allowing joystick reconnection / selection
            // Shift + M will reMap all buttons on an SDL device
            // Shift + Q will Quit the program
            // Shift + G will re output g_outputText
            if (getKeyState(VK_SHIFT)) {
                if (getKeyState('G')) {
                    displayOutputText();
                    // Set Lines for FPS and Latency output
                    std::cout << std::endl << std::endl;
                }
                if (getKeyState('R') || getKeyState('M') || getKeyState('Q')) {
                    if (IsAppActiveWindow()) {
                        inConnection = false;
                    }
                }
            }
            if (!inConnection || APP_KILLED) {
                break; // Break out of the loop if inConnection is false
            }

            //##################################
            // Read from Input
            if (args.mode == 2) {
                // Read the next HID report for DS4 Passthrough
                allGood = GetDS4Report();
                // *hid_report will point to most recent data 
            }
            else {
                // set the XBOX REPORT from SDL events
                allGood = get_xbox_report_from_SDL_events(activeGamepad, xbox_report);
#if 0
                if (fps_counter.get_frame_count() % 10 == 0) {
                    g_outputText = "";
                    printXusbReport(xbox_report);
                    std::cout << g_outputText;
                    repositionConsoleCursor(-7);
                }
#endif
            }
            if (!allGood) {
                g_outputText += "<< Device Disconnected >> \r\n";
                displayOutputText();
                inConnection = false;
                return args.mode + 1;
            }

            // ###################################
            // let's calculate some timing
            fpsOutput = do_fps_counting();
            if (args.latency) {
                if (!fpsOutput.empty()) {
                    overwriteFPS(fpsOutput + " fps  ");
                }
                latencyOutput = do_latency_timing();
                if (latencyOutput) {
                    overwriteLatency("Latency: " + formatDecimalString(std::to_string(((latencyOutput*1000) - loop_delay) / 2), 5) + " ms  ");
                }
            }
            // ###################################
                
            //###################################
            // Send joystick input to server
            if (args.mode == 2) {
                // Shift bytearray to index of first stick value
                allGood = client.send_data(reinterpret_cast<const char*>(hid_report+ds4DataOffset), DS4_REPORT_NETWORK_DATA_SIZE);
            }
            else {
                allGood = client.send_data(reinterpret_cast<const char*>(&xbox_report), sizeof(xbox_report));
            }
            // Error check
            if (allGood < 1) {
                g_outputText += "<< Connection Lost >> \r\n";
                displayOutputText();
                inConnection = false;
                break;
            }

            //##################################
            // # Wait for server response
            allGood = client.receive_data(buffer, buffer_size);                
            if (allGood < 1) {
                g_outputText += "<< Connection Lost >> \r\n";
                displayOutputText();
                inConnection = false;
                break;
            }

            //#################################
            // **  Process Rumble Feedback data
            if (updateRumble('L', byte(buffer[0])) || updateRumble('R', byte(buffer[1]))) {
#if 0
                if (args.latency) {
                    repositionConsoleCursor(-2);
                    std::cout << "Feedback: " << std::to_string(byte(buffer[0])) << " : " << std::to_string(byte(buffer[1])) << "     ";
                    repositionConsoleCursor(2);
                }
#endif
                if (args.mode == 2){
                    SetDS4RumbleValue(byte(buffer[0]), byte(buffer[1]));
                    allGood = SendDS4Update();
                    if (!allGood) {
                        outputLastError();
                        displayBytes(ds4_OutReportBuf, 16);
                    }
                }
                else if (args.mode == 1) {
                    SDLRumble(activeGamepad, byte(buffer[0]), byte(buffer[1]));
                }
            }

            // Sleep to yield thread
            Sleep(loop_delay > 0 ? loop_delay : 0);
            if (args.mode == 2) {
                // make sure we get a recent report
                DS4manager.Flush();
            }
        }
        // ****************** \\
        // Connection ended    \\


        // Catch key presses that could have terminiated connection
        // Shift + R  Resets program allowing joystick reconnection / selection, holding a number will change op mode
        if (getKeyState('R')) {
            g_outputText += "<< Restarted >>\r\n";
            while (getKeyState('R')) {
                if (getKeyState('1'))
                    return 2;
                if (getKeyState('2'))
                    return 3;
            }
            return 1;
        }
        // Shift + M  reMaps all inputs
        if (getKeyState('M')) {
            if (args.mode == 3) {
                //set_hid_mapping(gamepad, buttons, []);
            }
            else if(args.mode == 1) {
                // REMAP STUFF
                activeGamepad.mapping = SDLButtonMapping();
                std::vector<SDLButtonMapping::ButtonName> blankList;
                RemapInputs(activeGamepad, blankList);
                --failed_connections;
            }
        }
        // Shift + Q  will Quit
        if (getKeyState('Q') || APP_KILLED) {
            g_outputText += "<< Exiting >>\r\n";
            displayOutputText();
            return 0;
        }
        

        //##################################
        // Connection has failed or been aborted
        ++failed_connections;
        if (failed_connections > 2){
            args.host = "";
            failed_connections = 0;
        }

        //Clear any keyboard input
        swallowInput();
    }

    return 1;
}

int main(int argc, char **argv)
{
    Arguments args = parse_arguments(argc, argv);
    
    // Register the signal handler function
    std::signal(SIGINT, signalHandler);

    int err = InitJoystickInput();
    if (err) return -1;

    enableANSI();
    hideConsoleCursor();

    int RUN = 1;
    while (RUN > 0) {
        RUN = joySender(args);

        if (RUN > 1){
            args.mode = RUN - 1;
            args.select = false;
        }
        wait_for_no_keyboard_input();
    }

    // Cleanup and quit
    swallowInput();
    SDL_Quit();
    showConsoleCursor();

    return 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        APP_KILLED = 1;
        Sleep(5);
    }
}