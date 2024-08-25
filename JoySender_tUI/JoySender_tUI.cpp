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

#include "JoySender_tUI.h"

int joySender(Arguments& args) {
    // general error values
    int allGood;        

    // for network data/communication
    char buffer[128];
    int buffer_size = sizeof(buffer);
    int bytesReceived;
    int reportSize;
    bool inConnection = false;
    int failed_connections = 0;

    // for joystick mapping and sharing joystick data between functions
    SDLJoystickData activeGamepad;
    std::string mapName;
    std::vector<SDLButtonMapping::ButtonName> activeInputs;

    XUSB_REPORT xbox_report;            // xbox
    BYTE* hid_report = ds4_InReportBuf; // ds4

    // Lambda Functions and variables for FPS and FPS Limiting calculations
    FPSCounter fps_counter;
    int TARGET_FPS = args.fps;
    double fpsAdjustment = 0.0;
    std::string fpsOutput;
    auto do_fps_counting = [&fps_counter, &TARGET_FPS, &fpsAdjustment](int report_frequency = 30) {
        // set up some static doubles we will use each frame
        static double averageFrameTime_ms, fps, target_sleep = 1000.0 / (TARGET_FPS);
        
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

    // global flags set by ui and callback functions
    RESTART_FLAG = 0;
    MAPPING_FLAG = 0;
    g_mode = args.mode;

    //
    // Set Up for first tUI screen
    
        // establish a color scheme
    g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES);       // will be used as index for fullColorSchemes
    if (g_currentColorScheme == RANDOMSCHEME)
    {
        WORD newRandomBG = generateRandomInt(0, 15) AS_BG;

        static ColorScheme randomScheme;
        randomScheme = createRandomScheme();

        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(randomScheme, newRandomBG);
    }
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);  // Set for compatibility with DrawControllerFace

        // set element properties
    textUI& screen = g_screen;
    CoupleControllerButtons(); // sets up controller outline/highlight coupling for nice looks & button ids
    screen.SetBackdrop(JoySendMain_Backdrop);
    screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    output1.SetPosition(14, 5, 50, 1, ALIGN_LEFT); // output1 is a textarea used for screen headers/titles
    errorOut.SetPosition(consoleWidth/2, 4, 50, 0, ALIGN_CENTER); // used for error messaging
    fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT);                // fps output
    quitButton.setCallback(&exitAppCallback);
    newColorsButton.setCallback(&newControllerColorsCallback);
    quitButton.SetPosition(10, 17);
    

    //############################################################
    //# User or Auto Select Gamepad 
    switch (args.mode) {
    case 1: {   // SDL MODE
        if (!args.select) { // not auto select
           allGood = tUISelectJoystickDialog(activeGamepad, screen);
           if (!allGood) {
               SDL_Quit();
               return -1;
           }
        }
        else {
            allGood = ConnectToJoystick(0, activeGamepad); //select the first gamepad: index 0;
            if (!allGood) {
                screen.DrawBackdrop();
                errorOut.SetPosition(consoleWidth / 2, 7);
                setErrorMsg(L" Unable to connect to a device !! ", 35);
                errorOut.Draw();

                SDL_Quit();
                return -1;
            }
        }
    }
          break;
    case 2: {   // DS4 MODE
        screen.DrawBackdrop();
        allGood = tUISelectDS4Dialog(getDS4ControllersList(), screen);
        if (!allGood) {
            screen.DrawBackdrop();
            errorOut.SetPosition(consoleWidth / 2, 7);
            setErrorMsg(L" Unable to connect to a DS4 device !! ", 39);
            errorOut.Draw();
            return -1;
        }
    }
          break;
    default:
        screen.DrawBackdrop();
        errorOut.SetPosition(consoleWidth / 2, 7);
        setErrorMsg(L" ERROR:: Unsupported Mode !! ", 30);
        errorOut.Draw();
        return -1;
    }
    
    if (APP_KILLED) { // if user has quit stop execution
        return 0;
    }

    //############################################################
    // Initial Settings for Operating Mode:  DS4 / XBOX
    if (args.mode == 2) {         

        // Get a report from the device to determine what type of connection it has
        Sleep(5); // a fresh report should be generated every 4ms from the ds4 device
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

            // i like this but it doesnt show up here as the screen changes too quickly
            // it should be implemented in printControllerHeader()
            /*
            g_outputText = "DS4 Controller";
            if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless | "; }
            else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB | "; }
            if (extReport) { g_outputText += "Gyro/Accel | "; }
            if (allGood) {g_outputText += "Rumble | "; }

            setCursorPosition(5, 9);
            std::wcout << g_toWide(g_outputText);
            */

            reportSize = DS4_REPORT_NETWORK_DATA_SIZE;

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
        BuildJoystickInputData(activeGamepad);
        // Convert joystick name to hex  
        mapName = encodeStringToHex(activeGamepad.name);
        reportSize = sizeof(xbox_report);

        // Look for an existing map for selected device
        uiOpenOrCreateMapping(activeGamepad, mapName, screen);
        // Create a list of set joystick inputs
        activeInputs = activeGamepad.mapping.getSetButtonNames();
    }

    // UI resets
    screen.SetBackdrop(JoySendMain_Backdrop); // i think this is only need if a mapping occurred
    setErrorMsg(L"\0", 1); // clear error output in memory

    //############################################################
    //# Main Loop keeps client running
    //# asks for new host if, inner Connection Loop, fails 3 times
    while (!APP_KILLED) {
        
        //
        // Acquire host address for connection attempt
        if (args.host.empty()) {       

            screen.ReDraw();
            setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
            printCurrentController(activeGamepad);

            args.host = tUIGetHostAddress(screen);
            if (APP_KILLED) {
                return -1;
            }
            setErrorMsg(L"\0", 1); // clear errors in memory
        }
     
        TCPConnection client(args.host, args.port);
        client.set_silence(true);

        // 
        // Establish connection to host
        {           
            // Load Connecting Screen
            output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);    // title/heading text
            errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
            errorOut.SetPosition(consoleWidth / 2, 7, 50, 0, ALIGN_CENTER);
            
            mouseButton cancelButton(CONSOLE_WIDTH / 2 -7, 17, 13, L" (C) Cancel  ");
            screen.AddButton(&cancelButton);

            quitButton.SetPosition(10, 17);
            screen.AddButton(&quitButton);
            screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors)); 
            
            // set up connecting message
            int len = args.host.size() + 18;
            swprintf(msgPointer1, len, L" Connecting To: %s ", std::wstring(args.host.begin(), args.host.end()).c_str());
            output1.SetPosition(consoleWidth / 2, 9, len, 1, ALIGN_CENTER);
            output1.SetText(msgPointer1);

            // Draw Screen
            screen.ReDraw();
            setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
            printCurrentController(activeGamepad);
            errorOut.Draw();
            output1.Draw();

            // set up connecting animation
            output2.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
            output2.SetPosition(consoleWidth / 2, 10, 38, 0, ALIGN_CENTER);   

            // ### establish the connection in separate thread to animate connecting dialog
            allGood = WSAEWOULDBLOCK;
            std::thread connectThread(threadedEstablishConnection, std::ref(client), std::ref(allGood));
                // animation and input loop
            while (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
                output2.SetText(ConnectAnimation[g_frameNum]);
                output2.Draw();
                
                int mouseState = screenLoop(screen);

                // check for hot keys
                if (IsAppActiveWindow() && getKeyState(VK_SHIFT)) {
                    if (getKeyState('Q')) {
                        APP_KILLED = TRUE;
                    }
                    else if (getKeyState('C')) {
                        mouseState = MOUSE_UP;
                        cancelButton.SetStatus(MOUSE_UP);
                    }
                }

                // check Cancel button
                if (mouseState == MOUSE_UP) {
                   if (cancelButton.Status() & MOUSE_UP)
                    {
                        cancelButton.SetStatus(MOUSE_OUT);
                        
                        // stop connection thread
                        allGood = CANCELLED_FLAG;
                        break;
                    }
                }

                countUpDown(g_frameNum, CX_ANI_FRAME_COUNT);  // bounce
                //loopCount(g_frameNum, CX_ANI_FRAME_COUNT);    // loop

                if (!APP_KILLED && allGood == WSAEWOULDBLOCK)
                    Sleep(100);
            }
                // clean up
            connectThread.detach();
            screen.ClearButtons();
            // ###
        }

            // failed to make connection
        if (!APP_KILLED && allGood < 0) {
            int len = args.host.size() + 33;;
            if (allGood == CANCELLED_FLAG) {
                swprintf(errorPointer, len, L" << Connection To %s Canceled >> ", std::wstring(args.host.begin(), args.host.end()).c_str());
                args.host = "";
            }
            else {
                swprintf(errorPointer, len, L" << Connection To %s Failed: %d >> ", std::wstring(args.host.begin(), args.host.end()).c_str(), failed_connections + 1);
            }
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
        }

        //
        // Set up UI and do timing/mode handshake with host
        else if(!APP_KILLED){
            // Set up screen for main connection loop
            {
                int QUITLINE;
                if (args.mode == 2) {
                    screen.SetBackdrop(DS4_Backdrop);
                    QUITLINE = DS4_QUIT_LINE;                    
                    BuildDS4Face();
                }
                else {
                    screen.SetBackdrop(XBOX_Backdrop);
                    QUITLINE = XBOX_QUIT_LINE;
                    BuildXboxFace();

                    screen.AddButton(&mappingButton);
                }

                SetControllerButtonPositions(args.mode);
               
                quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE);
                newColorsButton.SetPosition(consoleWidth / 2 - 8, QUITLINE - 3);

                screen.AddButton(&newColorsButton);
                screen.AddButton(&quitButton);

                restartButton[0].SetPosition(CONSOLE_WIDTH / 2 - 12, QUITLINE - 1);
                restartButton[1].SetPosition(CONSOLE_WIDTH / 2 - 8, QUITLINE - 1);
                restartButton[2].SetPosition(CONSOLE_WIDTH / 2 - 6, QUITLINE - 1);
                restartButton[3].SetPosition(CONSOLE_WIDTH / 2 + 6, QUITLINE - 1);

                screen.AddButton(&restartButton[1]);  // mode 1
                screen.AddButton(&restartButton[2]);  // mode 2
                screen.AddButton(&restartButton[0]);  // main restart
                //screen.AddButton(&restartButton[3]); // not needed in screen

                // Set button and controller colors
                    // Sets controller to color scheme colors with some contrast correction for bg color then draws the controller face
                DrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, args.mode);

                tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);

                    // color non controller buttons and draw them
                restartButton[3].SetColors(buttonColors);  // mode section of restart button
                screen.SetButtonsColors(buttonColors);
                screen.DrawButtons();

                restartButton[0].setCallback(restartStatusCallback);
                restartButton[1].setCallback(restartModeCallback);
                restartButton[2].setCallback(restartModeCallback);

                mappingButton.setCallback(mappingButtonCallback);

                wcsncpy_s(msgPointer1, 40, g_converter.from_bytes(" << Connected to: " + args.host + "  >> ").c_str(), _TRUNCATE);
                output1.SetText(msgPointer1);
                output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
                output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);

                wcsncpy_s(hostPointer, 18, g_converter.from_bytes(" "+args.host+" ").c_str(), _TRUNCATE);
                hostMsg.SetText(hostPointer);
                hostMsg.SetPosition(20, 1, 38, 1, ALIGN_LEFT);
                hostMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);

                fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);

                restartButton[3].Draw(-2); // in x64 release build the default parameter of -1 does not work??? debug show it set as 0. manually specifying value less then -1 here works
                output1.Draw();
                hostMsg.Draw();
            }

            //
            // Attempt timing and mode setting handshake * sets inConnection true
            {
                std::string txSettings = std::to_string(args.fps) + ":" + std::to_string(args.mode);
                // tx
                allGood = client.send_data(txSettings.c_str(), static_cast<int>(txSettings.length()));
                if (allGood < 1) {
                    swprintf(errorPointer, 48, L" << Connection To %S Failed >> ", args.host.c_str());
                    errorOut.SetText(errorPointer);
                    //setErrorMsg(g_converter.from_bytes(" << Connection To: " + args.host + " Failed >> ").c_str(), 48);
                    client.~TCPConnection();
                    break;
                }
                // rx
                bytesReceived = client.receive_data(buffer, buffer_size);
                if (bytesReceived > 0) {
                    inConnection = true;
                }
                else {
                    swprintf(errorPointer, 46, L" << Connection To: %S Lost >> ", args.host.c_str());
                    errorOut.SetText(errorPointer);
                    //setErrorMsg(g_converter.from_bytes(" << Connection To: " + args.host + " Lost >> ").c_str(), 46);
                    client.~TCPConnection();
                    break;
                }
            }
        }


        // //############################################################  //
        // Connection Loop  //##########################################  //
        fps_counter.reset();
        while (inConnection){

            screenLoop(screen);

            //
            // Catch hot key button presses
            if (IsAppActiveWindow() && !MAPPING_FLAG && getKeyState(VK_SHIFT)) {
                if (checkKey('C', IS_PRESSED)) {
                    // change colors
                    button_Guide_highlight.SetStatus(MOUSE_UP);
                    newControllerColorsCallback(button_Guide_highlight);
                }
                if (getKeyState('M') && args.mode == 1) {
                    MAPPING_FLAG = 1;                    
                }
                if (getKeyState('R') || getKeyState('Q')) {
                    if (getKeyState('R'))
                        RESTART_FLAG = 1;

                    if (getKeyState('Q'))
                        APP_KILLED = true;
                }
            }
            if (RESTART_FLAG || APP_KILLED) {
                client.~TCPConnection();
                inConnection = false;
                break; // Break out of the loop
            }

            // Do remapping if triggered (freezes host)
            if (MAPPING_FLAG) {
                // REMAP STUFF ** pauses communication (freezing host) till finished
                tUIRemapInputsScreen(activeGamepad, screen);
                if (APP_KILLED) {
                    client.~TCPConnection();
                    inConnection = false;
                    break; // Break out of the loop
                }

                // sets new input scheme
                activeInputs = activeGamepad.mapping.getSetButtonNames(); 

                // reset output1
                wcsncpy_s(msgPointer1, 40, g_converter.from_bytes(" << Connected to: " + args.host + "  >> ").c_str(), _TRUNCATE);
                output1.SetText(msgPointer1);
                output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
                output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);

                // re add screen buttons
                screen.AddButton(&mappingButton);
                screen.AddButton(&restartButton[1]);  // mode 1
                screen.AddButton(&restartButton[2]);  // mode 2
                screen.AddButton(&restartButton[0]);  // main restart
                screen.AddButton(&quitButton);

                // reset quit button
                quitButton.setCallback(&exitAppCallback);
                quitButton.SetPosition(consoleWidth / 2 - 5, XBOX_QUIT_LINE);

                // redraw everything
                DrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, args.mode);
                g_screen.DrawButtons();
                restartButton[3].Draw();
                output1.Draw();
                hostMsg.Draw();
                
                MAPPING_FLAG = 0;
            }


            //
            // Read from Input
            if (args.mode == 2) {
                //# Read the next HID report from DS4 controller
                allGood = GetDS4Report(); // *hid_report will be set

                if (!allGood) {
                    swprintf(errorPointer, 28, L" << Device Disconnected >> ");
                    errorOut.SetText(errorPointer);
                    //setErrorMsg(g_converter.from_bytes(" << Device Disconnected >> ").c_str(), 28);
                    client.~TCPConnection();
                    inConnection = false;
                    allGood = DISCONNECT_ERROR;
                    break;
                }

                // activate screen buttons from *hid_report
                buttonStatesFromDS4Report();
            }
            else {
                //# set the XBOX REPORT from SDL inputs
                get_xbox_report_from_SDL_events(activeGamepad, xbox_report);

                // activate screen buttons from xbox report
                buttonStatesFromXboxReport(xbox_report);
            }

                
            // 
            //  Send joystick input to server
            if (args.mode == 2) {
                //# Shift bytearray to index of first stick value
                allGood = client.send_data(reinterpret_cast<const char*>(hid_report+ds4DataOffset), reportSize);     
            }
            else {
                allGood = client.send_data(reinterpret_cast<const char*>(&xbox_report), reportSize); 
            }
                //  Error 
            if (allGood < 1) {
                swprintf(errorPointer, 50, L" << Connection To:  %s Failed >> ", std::wstring(args.host.begin(), args.host.end()).c_str());
                errorOut.SetText(errorPointer);
                //setErrorMsg(g_converter.from_bytes(" << Connection To: " + args.host + " Failed >> ").c_str(), 48);
                client.~TCPConnection();
                inConnection = false;
                break;
            }


            //
            // Wait for server response
            allGood = client.receive_data(buffer, buffer_size);                
            if (allGood < 1) {
                swprintf(errorPointer, 50, L" << Connection To:  %S Failed >> ", args.host.c_str());
                errorOut.SetText(errorPointer);
                //setErrorMsg(g_converter.from_bytes(" << Connection To: " + args.host + " Lost >> ").c_str(), 46);
                client.~TCPConnection();
                inConnection = false;
                break;
            }



            // **  Process Rumble Feedback data
            if (updateRumble('L', byte(buffer[0])) || updateRumble('R', byte(buffer[1]))) {
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

            // calculate timing
            fpsOutput = do_fps_counting();
            if (!fpsOutput.empty()) {
                //updateFPS(g_converter.from_bytes(fpsOutput + "   ").c_str(), 8);
                swprintf(fpsPointer, 8, L"%S   ",fpsOutput.c_str());
                fpsMsg.SetText(fpsPointer);
                fpsMsg.Draw();
            }


            //
            // Sleep to yield thread
            Sleep(max(fpsAdjustment, 0)); 
            if (args.mode == 2) {
                // make sure we get a recent report
                DS4manager.Flush();
            }
        }
        // ###########################################################  \\
        // Connection Ended    ########################################  \\

        if (allGood == DISCONNECT_ERROR) {
            screen.ClearButtons();
            break;
        }

        //
        // Catch mouse/key presses that could have terminated connection
        if (APP_KILLED) {
            return 0;
        }
            // Shift + R  Resets program, holding 1 or 2 will change mode
        if (RESTART_FLAG)  {

            while (RESTART_FLAG < 2 && getKeyState('R')) {
                if (getKeyState('1'))
                    RESTART_FLAG = 2;
                if (getKeyState('2'))
                    RESTART_FLAG = 3;
            }

            screen.ClearButtons();
            screen.SetBackdrop(JoySendMain_Backdrop);

            return RESTART_FLAG;
        }

        
        //
        // Connection has failed or been aborted
        ++failed_connections;
        if (failed_connections > 2){
            args.host = "";
            failed_connections = 0;
        }
        screen.ClearButtons();
        screen.SetBackdrop(JoySendMain_Backdrop);
    }

    return !APP_KILLED;
}

// Set console size
console con(consoleWidth+2, consoleHeight+2);

int main(int argc, char **argv)
{
    Arguments args = parse_arguments(argc, argv);
    g_mode = args.mode;
    
    // Register the signal handler function
    std::signal(SIGINT, signalHandler);

    int err = InitJoystickInput();
    if (err) return -1;

    // Set Version into backdrop
    {
        int versionStartPoint = 73 * 3 + 29;
        const size_t verLength = wcslen(APP_VERSION_NUM);

        for (int i = 0; i < verLength; i++) {
            JoySendMain_BackdropSource[versionStartPoint + i] = APP_VERSION_NUM[i];
        }
    }

    loadIPDataFromFile();

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
    setCursorPosition(0, consoleHeight);
    setTextColor(DEFAULT_TEXT);
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