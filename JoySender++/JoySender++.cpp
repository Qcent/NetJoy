/*

Copyright (c) 2023 Dave Quinn <qcent@yahoo.com>

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


// Function safely return environment variables
std::string g_getenv(const char* variableName) {
    std::string value;
    size_t requiredSize = 0;
    getenv_s(&requiredSize, nullptr, 0, variableName);
    if (requiredSize > 0) {
        value.resize(requiredSize);
        getenv_s(&requiredSize, &value[0], requiredSize, variableName);
        value.pop_back(); // Remove the null-terminating character
    }
    return value;
}
// Some Console Output, ANSI helper functions

// Function to detect if keyboard key is pressed
bool getKeyState(int KEYCODE) {
    return GetAsyncKeyState(KEYCODE) & 0x8000;
}
// Function to convert strings to wide strings
std::wstring g_toWide(std::string& str) {
    std::wstring wideString(str.begin(), str.end());
    return wideString;
}
// Function that will prompt for and verify an ip address 
std::string getHostAddress() {
    // Lambda function for IP address validation
    auto validIPAddress = [](const std::string& ipAddress) {
        std::regex pattern(R"((\b\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\b))");
        std::smatch match;
        return std::regex_match(ipAddress, match, pattern);
    };
    swallowInput();
    std::string host_address;
    showConsoleCursor();
    while (!APP_KILLED && host_address.empty()) {
            std::cout << "Please enter the host IP address (or nothing for localhost): ";
            //std::cin.ignore(80, '\n'); // Flush the input stream
            std::getline(std::cin, host_address);

        if (host_address.empty()) {
            host_address = "127.0.0.1";
        }

        if (!validIPAddress(host_address)) {
            std::cout << "Invalid IP address. Please try again." << std::endl;
            host_address = "";
        }
    }
    hideConsoleCursor();
    return host_address;
}
// Function that takes string representing a float and fixes it's decimal places to numDigits
std::string formatDecimalString(const std::string& str, UINT8 numDigits) {
    size_t decimalPos = str.find('.');
    if (decimalPos == std::string::npos) {
        // No decimal point found, return the original 
        // string with decimal and padded zeros
        std::string paddedStr = str + ".";
        paddedStr.append(numDigits, '0');
        return paddedStr;
    }
    size_t strLen = str.length();
    size_t numDecimals = strLen - decimalPos - 1;
    if (numDecimals <= numDigits) {
        // No need to truncate or pad, return the original string
        return str;
    }
    // Truncate the string to the desired number of decimal places
    std::string truncatedStr = str.substr(0, decimalPos + numDigits + 1);
    // Pad with zeros if necessary
    if (numDecimals < numDigits) {
        truncatedStr.append(numDigits - numDecimals, '0');
    }

    return truncatedStr;
}
// Function captures all input from the keyboard to clear buffer
void swallowInput()
{
    while (_kbhit())
    {
        _getch();
    }
}
// Function waits for no keyboard presses to be detected before returning
void wait_for_no_keyboard_input() {
    bool input = true;
    while (input) {
        input = false;
        // Check if any key is currently being pressed
        // 31 - 111 are ascii codes 0-9, aA-zZ, and numpad
        for (int i = 31; i < 112; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                input = true;
            }
        }

        // some extra keys i want to check for
        if (getKeyState(VK_OEM_PERIOD) || getKeyState(VK_DECIMAL) || getKeyState(VK_BACK)) {
            input = true;
        }

    }
    swallowInput();
}
// Function determines if app is the active window
bool IsAppActiveWindow()
{
    HWND consoleWindow = GetConsoleWindow();
    HWND foregroundWindow = GetForegroundWindow();

    return (consoleWindow == foregroundWindow);
}
// Function determines if rumble values need to be updated
bool updateRumble(const char motor, BYTE val) {
    static BYTE L, R = 0;
    switch (motor) {
    case 'L':
    {
        if (val == 0 && L == val) return false;
        L = val;
        return true;
    }

    case 'R':
    {
        if (val == 0 && R == val) return false;
        R = val;
        return true;
    }
    }
    return false;
}

int joySender(Arguments& args) {
    FPSCounter fps_counter;
    FPSCounter latencyTimer;
    bool inConnection = false;
    int failed_connections = 0;
    std::string mapName;
    std::vector<SDLButtonMapping::ButtonName> activeInputs;
      
    int allGood;
    char buffer[128];
    int buffer_size = sizeof(buffer);
    int bytesReceived;
    int reportSize;

    SDLJoystickData activeGamepad;
    XUSB_REPORT xbox_report;
    BYTE* hid_report = ds4_InReportBuf;

    // Lambda Functions and variables for FPS and FPS Limiting calculations
    int TARGET_FPS = args.fps;
    double fpsAdjustment = 0.0;
    
    std::string fpsOutput;
    auto do_fps_counting = [&fps_counter, &TARGET_FPS, &fpsAdjustment](int report_frequency = 30) {
        // set up some static doubles we will use each frame
        static double averageFrameTime_ms, fps, target_sleep = 1000 / (TARGET_FPS);
        
        int count = fps_counter.increment_frame_count();

        // determine how long to sleep to limit frame rate
        averageFrameTime_ms = (fps_counter.get_elapsed_time() / count) * 1000;
        fps = fps_counter.get_fps();
        if (fps < TARGET_FPS)
            fpsAdjustment = target_sleep - averageFrameTime_ms;
        else
            fpsAdjustment = target_sleep;
    
        // output fps to caller
        if (count >= report_frequency) {
            fps_counter.reset();
            return formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
    };
    double latencyOutput = 0.0;
    const int latency_report_freq = 25;
    auto do_latency_timing = [&latencyTimer](int report_frequency = 15) {
        if (latencyTimer.increment_frame_count() >= report_frequency) {
            double elapsedTime = latencyTimer.get_elapsed_time();
            latencyTimer.reset();
            return elapsedTime / report_frequency; // average time per frame
        }
        return 0.0;
};


    //
    // Set Up for first tUI screen
    //
        // Get the console input handle to enable mouse input
    g_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    g_mode = args.mode;
    DWORD mode;
    GetConsoleMode(g_hConsoleInput, &mode);                     // Disable Quick Edit Mode
    SetConsoleMode(g_hConsoleInput, mode | ENABLE_MOUSE_INPUT & ~ENABLE_QUICK_EDIT_MODE);

        // Set the console to UTF-8 mode
    _setmode(_fileno(stdout), _O_U8TEXT);

    hideConsoleCursor();

    g_screen.SetBackdrop(JoySendMain_Backdrop);
    output1.SetPosition(15, 5, 50, 1, ALIGN_LEFT);
    output1.SetColor(WHITE|BLACK AS_BG);
    errorOut.SetPosition(consoleWidth/2, 4, 50, 0, ALIGN_CENTER);
    fpsMsg.SetPosition(52, 2, 7, 1, ALIGN_LEFT);
    quitButton.setCallback(&exitAppCallback);
    quitButton.SetPosition(10, 17);
    
    //###########################################################################
    //# User or auto select gamepad 
    switch (args.mode) {
    case 1: {   // SDL MODE
        if (!args.select) {
           allGood = tUISelectJoystickDialog(getJoystickList().size(), activeGamepad, g_screen);
           if (!allGood) {
               setErrorMsg(L"Failed to open joystick.", 25);
               errorOut.Draw();

               SDL_Quit();
               return -1;
           }
        }
        else {
            allGood = ConnectToJoystick(0, activeGamepad);
            if (!allGood) {
                setErrorMsg(L" Unable to connect to a device !! ", 35);
                errorOut.Draw();

                SDL_Quit();
                return -1;
            }
        }
    }
          break;
    case 2: {   // DS4 MODE
        allGood = tUIConnectToDS4Controller(g_screen);
        if (!allGood) {
            setErrorMsg(L" Unable to connect to a DS4 device !! ", 39);
            errorOut.Draw();
            return -1;
        }
    }
          break;
    default:
        setErrorMsg(L" ERROR:: Unsupported Mode !! ", 30);
        errorOut.Draw();
        return -1;
    }
    
    // refresh g_screen
    g_screen.DrawBackdrop();
    
    //###########################################################################
    // Init Settings for Operating Mode
    if (args.mode == 2) {  
        
        // Get a report from the device to determine what type of connection it has
        GetDS4Report();

        // first byte is used to determine where stick input starts
        ds4DataOffset = hid_report[0] == 0x11 ? DS4_VIA_BT : DS4_VIA_USB;

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

        allGood = SendDS4Update();       

        g_outputText = "DS4 "; 
        if(extReport) g_outputText += "Full Motion ";
        g_outputText += "Mode Activated : ";
        if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
        else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
        if (allGood) g_outputText += " Rumble On | ";
        
        setCursorPosition(5, 9);
        std::wcout << g_toWide(g_outputText);

        reportSize = DS4_REPORT_NETWORK_DATA_SIZE;

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
        }
    }
    else {
        //g_outputText = "SDL Mode Activated\r\n";
        BuildJoystickInputData(activeGamepad);
        // Convert joystick name to hex  
        mapName = encodeStringToHex(activeGamepad.name);
        reportSize = sizeof(xbox_report);
    }

    //###########################################################################
    //# If not in DS4 pass through mode look for Saved Mapping or create one
    if (args.mode != 2) {
        //# Look for an existing map for selected device
        OpenOrCreateMapping(activeGamepad, mapName);
        // Create a list of set joystick inputs
        activeInputs = activeGamepad.mapping.getSetButtonNames();
    }

    //###########################################################################
    //# Main Loop keeps client running
    //# asks for new host if connection fails 3 times
    while (!APP_KILLED) {
        //fps_counter.reset();
        printCurrentController(activeGamepad); // not yet
        
        // Acquire host address for connection attempt
        if (args.host.empty()) {
            args.host = tUIGetHostAddress(g_screen);
        }
        if (APP_KILLED) {
            return -1;
        }

        TCPConnection client(args.host, args.port);
        client.set_silence(true);
        allGood = client.establish_connection();

        // *******************
        // Attempt timing and mode setting handshake
        if (allGood < 0) {
            int len = args.host.size() + 31;
            swprintf(errorPointer, len, L"<< Connection To %s Failed: %d >>", std::wstring(args.host.begin(), args.host.end()).c_str(), failed_connections+1);
            //setErrorMsg(L"Failed to Connect :: Retry 1", 32);
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
        }
        else{

            g_screen.ClearButtons();
            if(mode == 2)
                g_screen.SetBackdrop(DS4_Backdrop);
            else
                g_screen.SetBackdrop(XBOX_Backdrop);

            LoadButtons(g_screen, mode);
            button_Guide_highlight.setCallback(testCallback); // will switch colors

            g_screen.AddButton(&quitButton);
            //quitButton.setCallback(&exitAppCallback); // will quit app

            wcsncpy_s(connectionPointer, 25, g_converter.from_bytes(args.host + " >>    ").c_str(), _TRUNCATE);
            connectionMsg.SetText(connectionPointer);
            connectionMsg.SetPosition(21, 1, 25, 1, ALIGN_LEFT);

            g_screen.DrawBackdrop();
            g_screen.DrawButtons();
            
            connectionMsg.Draw();
            
          
            // Send timing and mode data
            std::string txSettings = std::to_string(args.fps) + ":" + std::to_string(args.mode);
            allGood = client.send_data(txSettings.c_str(), static_cast<int>(txSettings.length()));
            if (allGood < 1) {              
                setErrorMsg(g_converter.from_bytes("<< Connection To: " + args.host + " Failed >>").c_str(), 46);

                client.~TCPConnection();
                break;
            }

            bytesReceived = client.receive_data(buffer, buffer_size);
            if (bytesReceived > 0) {
                inConnection = true;
            }
            else{
                setErrorMsg(g_converter.from_bytes("<< Connection To: " + args.host + " Lost >>").c_str(), 44);
                
                client.~TCPConnection();
                break;
            }

        }


        // ******************
        // Connection loop
        fps_counter.reset();
        while (inConnection){
            // Shift + R will Reset program allowing joystick reconnection / selection
            // Shift + M will reMap all buttons on an SDL device
            // Shift + Q will Quit the program
            // Shift + G will re output g_outputText
            if (getKeyState(VK_SHIFT)) {
                if (getKeyState('G')) {
                    // maybe change colors or something
                }
                if (getKeyState('R') || getKeyState('M') || getKeyState('Q')) {
                    if (IsAppActiveWindow()) {
                        client.~TCPConnection();
                        inConnection = false;
                    }
                }
            }
            if (!inConnection || APP_KILLED) {
                break; // Break out of the loop if inConnection is false
            }

            //###################################
            //# Read from Input

            if (args.mode == 2) {
                //# Read the next HID report for DS4 Passthrough
                allGood = GetDS4Report();
                // *hid_report will point to most recent data
#if 0
                repositionConsoleCursor(2);
                displayBytes(hid_report,61);
                repositionConsoleCursor(-4);
#endif
                if (!allGood) {
                    //g_outputText += "<< Device Disconnected >> \r\n";
                    //displayOutputText();
                    setErrorMsg(g_converter.from_bytes("<< Device Disconnected >>").c_str(), 26);
                    client.~TCPConnection();
                    inConnection = false;
                }
            }
            else {
                //# set the XBOX REPORT from SDL inputs
                get_xbox_report_from_SDLmap(activeGamepad, activeInputs, xbox_report);
            }

            // ###################################
            //# let's calculate some timing
            fpsOutput = do_fps_counting();

            if (!fpsOutput.empty()) {
                updateFPS(g_converter.from_bytes(fpsOutput+"   ").c_str(), 8);
            }
            //latencyOutput = do_latency_timing(latency_report_freq);
            //if (latencyOutput) {
                //overwriteLatency("Latency: " + formatDecimalString(std::to_string((latencyOutput*1000) - fpsAdjustment), 5) + " ms  ");
            //}
            
            // ###################################
                
            //####################################
            //# Send joystick input to server
            if (args.mode == 2) {
                //# Shift bytearray to index of first stick value
                allGood = client.send_data(reinterpret_cast<const char*>(hid_report+ds4DataOffset), reportSize);
#if 0
                    // for troubleshooting ds4 reports
                if (fps_counter.get_frame_count() % 5 == 0) {
                    repositionConsoleCursor(1);
                    displayBytes(hid_report, DS4_REPORT_NETWORK_DATA_SIZE);
                    repositionConsoleCursor(-3);
                }
#endif        
            }
            else {
                allGood = client.send_data(reinterpret_cast<const char*>(&xbox_report), reportSize);
#if 0
                // for troubleshooting xbox reports
                if (fps_counter.get_frame_count() % 5 == 0) {
                    displayBytes(reinterpret_cast<BYTE*>(&xbox_report), reportSize);
                    repositionConsoleCursor(-1);
                }
#endif  
            }
            //  Error ?
            if (allGood < 1) {
                //std::cout << "<< Connection Lost >> \r\n";
                setErrorMsg(g_converter.from_bytes("<< Connection To: " + args.host + " Failed >>").c_str(), 46);
                //displayOutputText();
                client.~TCPConnection();
                inConnection = false;
                break;
            }

            //###################################
            // # Wait for server response
            allGood = client.receive_data(buffer, buffer_size);                
            if (allGood < 1) {
                //std::cout << "<< Connection Lost >> \r\n";
                setErrorMsg(g_converter.from_bytes("<< Connection To: " + args.host + " Lost >>").c_str(), 44);
                //displayOutputText();
                client.~TCPConnection();
                inConnection = false;
                break;
            }

            //##################################
            // **  Process Rumble Feedback data
            if (updateRumble('L', byte(buffer[0])) || updateRumble('R', byte(buffer[1]))) {
#if 0
                if (args.latency) {
                    //repositionConsoleCursor(-2);
                    //std::cout << "Feedback: " << std::to_string(byte(buffer[0])) << " : " << std::to_string(byte(buffer[1])) << "     ";
                    //repositionConsoleCursor(2);
                }
#endif
                if (args.mode == 2){
                    SetDS4RumbleValue(byte(buffer[0]), byte(buffer[1]));
                    allGood = SendDS4Update();
                    if (!allGood) {
                        // USB always errors // not anymore right?
                       // outputLastError();
                       // displayBytes(ds4_OutReportBuf, 16);
                    }
                }
                else if (args.mode == 1) {
                    SDLRumble(activeGamepad, byte(buffer[0]), byte(buffer[1]));
                }
            }

            // Sleep to yield thread
            Sleep(max(fpsAdjustment, 0)); 
            if (args.mode == 2) {
                // make sure we get a recent report
                DS4manager.Flush();
            }
        }
        // ****************** \\
        // Connection ended    \\


        // Catch key presses that could have terminated connection
        //# Shift + R  Resets program allowing joystick reconnection / selection, holding a number will change op mode
        if (getKeyState('R')) {
            g_outputText += "<< Restarted >>\r\n";
            while (getKeyState('R')) {
                if (getKeyState('1'))
                    return 2;
                if (getKeyState('2'))
                    return 3;
                if (getKeyState('3'))
                    return 4;
            }
            return 1;
        }
        //# Shift + M  reMaps all inputs
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
        //# Shift + Q  will Quit
        if (getKeyState('Q') || APP_KILLED) {
            //g_outputText += "<< Exiting >>\r\n";
            //displayOutputText();
            return 0;
        }
        

        //###################################
        //# Connection has failed or been aborted
        ++failed_connections;
        if (failed_connections > 2){
            args.host = "";
            failed_connections = 0;
        }
        g_screen.ClearButtons();
        g_screen.SetBackdrop(JoySendMain_Backdrop);
        //g_screen.AddButton(&quitButton);
        errorOut.SetPosition(consoleWidth / 2, 7, 50, 0, ALIGN_CENTER);
        g_screen.ReDraw();
        printCurrentController(activeGamepad);
        errorOut.Draw();

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
    setCursorPosition(0, CONSOLE_HEIGHT);
    showConsoleCursor();

    return 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        APP_KILLED = 1;
        Sleep(5);
        setCursorPosition(0, CONSOLE_HEIGHT - 2);
        std::wcout << "Keyboard interrupt received. Exiting gracefully." << std::endl;
    }
}