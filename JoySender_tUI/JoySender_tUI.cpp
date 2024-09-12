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
#define JOYSENDER_TUI
#include "JoySender_tUI.h"

int joySendertUI(Arguments& args) {
    // general error values
    int allGood;        
    // for network data/communication
    char buffer[24];
    int buffer_size = sizeof(buffer);
    int bytesReceived;
    bool inConnection = false;
    int failed_connections = 0;
    // for joystick mapping and sharing joystick data between functions
    SDLJoystickData activeGamepad;
    XUSB_REPORT xbox_report = {0}; 
    BYTE* ds4_report = ds4_InReportBuf;
    // Lambda Functions and variables for FPS and FPS Limiting calculations
    FPSCounter fps_counter;
    double loop_delay = 0.0;
    std::string fpsOutput;
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
            return formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
    };
    
    g_mode = args.mode;
    JOYSENDER_tUI_INIT_UI();
    g_screen.AddButton(&quitButton);
    
    // User or Auto Select Gamepad
    if (!JOYSENDER_tUI_SELECT_JOYSTICK(activeGamepad, args, allGood)) {
        return -1;
    }
    if (APP_KILLED) { // if user has quit stop execution
        return 0;
    }

    // tidy the screen butttons
    g_screen.ClearButtonsExcept({42,45,46,47});

    // Initial Settings for Operating Mode:  DS4 / XBOX
    JOYSENDER_tUI_OPMODE_INIT(activeGamepad, args, allGood);
    
    // UI resets
    g_screen.SetBackdrop(JoySendMain_Backdrop); // i think this is only need if a mapping occurred
    setErrorMsg(L"\0", 1); // clear error output in memory

    //# Loop keeps client running
    //# asks for new host if, inner Connection Loop, fails 3 times
    while (!APP_KILLED) {
        // Acquire host address for connection attempt
        if (args.host.empty()) {       
            args.host = tUIGetHostAddress(activeGamepad);
            // tidy the screen butttons
            g_screen.ClearButtonsExcept({ 42,45,46,47 });

            if (APP_KILLED) {
                return -1;
            }
            setErrorMsg(L"\0", 1); // clear errors in memory
        }

        TCPConnection client(args.host, args.port);
        client.set_silence(true);

        // Establish connection to host
        JOYSENDER_tUI_ANIMATED_CX(activeGamepad, client, args, allGood);

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

        // Setup UI and do timing/mode handshake with host
        else if(!APP_KILLED){
            // Set up g_screen for main connection loop
            JOYSENDER_tUI_BUILD_MAIN_LOOP(args);

            // Attempt timing and mode setting handshake * sets inConnection true
            JOYSENDER_tUI_CX_HANDSHAKE();
        }

        if (inConnection && OLDMAP_FLAG == true) {
            setErrorMsg(g_converter.from_bytes(OLDMAP_WARNING_MSG).c_str(), 65);
            errorOut.SetPosition(consoleWidth/2 - 31, consoleHeight - 1, 65, 1, ALIGN_LEFT);
            errorOut.Draw();
        }

        // //########################################################  //
        // Connection Loop  //######################################  //
        fps_counter.reset();
        while (inConnection){
            screenLoop(g_screen);
            EGG_LOOP();

            // Catch hot key button presses
            if (!MAPPING_FLAG && getKeyState(VK_SHIFT) && IsAppActiveWindow()) {
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
                inConnection = false;
                break;
            }

            // Do remapping if triggered (freezes host)
            if (MAPPING_FLAG) {
                // REMAP STUFF ** pauses communication (freezing host) till finished
                 tUIRemapInputsScreen(activeGamepad);
                if (APP_KILLED) {
                    inConnection = false;
                    break; // Break out of the loop
                }
                JOYSENDER_tUI_RESET_AFTER_MAP(args);
                memset((void*)&xbox_report, 0, sizeof(xbox_report));
            }

            // Read from Input
            if (args.mode == 2) {
                //# Read the next HID report from DS4 controller
                allGood = GetDS4Report(); // *ds4_report will be set
                buttonStatesFromDS4Report(&ds4_report[ds4DataOffset]);
            }
            else {
                //# set the XBOX REPORT from SDL inputs
                allGood = get_xbox_report_from_SDL_events(activeGamepad, xbox_report);
                buttonStatesFromXboxReport(xbox_report);
            }
            if (!allGood) {
                swprintf(errorPointer, 28, L" << Device Disconnected >> ");
                errorOut.SetText(errorPointer);
                inConnection = false;
                allGood = DISCONNECT_ERROR;
                break;
            }

            //  Send joystick input to server
            if (args.mode == 2) {
                //# Shift bytearray to index of first stick value
                allGood = client.send_data(reinterpret_cast<const char*>(ds4_report + ds4DataOffset), DS4_REPORT_NETWORK_DATA_SIZE);
            }
            else {
                allGood = client.send_data(reinterpret_cast<const char*>(&xbox_report), sizeof(xbox_report));
            }
            if (allGood < 1) {
                swprintf(errorPointer, 50, L" << Connection To:  %s Failed >> ", std::wstring(args.host.begin(), args.host.end()).c_str());
                errorOut.SetText(errorPointer);
                inConnection = false;
                break;
            }

            // Wait for server Feedback (rumble, lightbar)
            allGood = client.receive_data(buffer, buffer_size);                
            if (allGood < 1) {
                swprintf(errorPointer, 50, L" << Connection To:  %S Failed >> ", args.host.c_str());
                errorOut.SetText(errorPointer);
                inConnection = false;
                break;
            }

            // Process Feedback data
            processFeedbackBuffer((byte*)&buffer, activeGamepad, args.mode);

            // calculate timing
            fpsOutput = do_fps_counting();
            if (!fpsOutput.empty()) {
                //updateFPS(g_converter.from_bytes(fpsOutput + "   ").c_str(), 8);
                swprintf(fpsPointer, 8, L" %S   ",fpsOutput.c_str());
                fpsMsg.SetText(fpsPointer);
                fpsMsg.Draw();
            }

            // Sleep to yield thread
            Sleep(loop_delay > 0 ? loop_delay : 0);
            if (args.mode == 2) {
                // make sure we get a recent report
                DS4manager.Flush();
            }
        }
        // #########################################################  \\
        // Connection Ended    ######################################  \\

        if (allGood == DISCONNECT_ERROR) {
            g_screen.ClearButtons();
            break;
        }
        if (APP_KILLED) {
            return 0;
        }

        // Catch mouse/key presses that could have terminated connection
        if (RESTART_FLAG)  {
            // Shift + R  Resets program, holding 1 or 2 will change mode
            while (RESTART_FLAG < 2 && getKeyState('R')) {
                if (getKeyState('1'))
                    RESTART_FLAG = 2;
                if (getKeyState('2'))
                    RESTART_FLAG = 3;
            }
            g_screen.ClearButtons();
            g_screen.SetBackdrop(JoySendMain_Backdrop);
            return RESTART_FLAG;
        }

        // Connection has failed or been aborted
        ++failed_connections;
        if (failed_connections > 2){
            args.host = "";
            failed_connections = 0;
        }
        g_screen.ClearButtonsExcept({ 42,45,46,47 });
        g_screen.SetBackdrop(JoySendMain_Backdrop);
    }
    return APP_KILLED ? 0 : 1;
}

console con(consoleWidth, consoleHeight);

int main(int argc, char** argv)
{
    Arguments args = parse_arguments(argc, argv);
    int RUN = 1;

    // Register the signal handler function
    std::signal(SIGINT, signalHandler);

    int err = InitJoystickInput();
    if (err) return -1;

    // Get the console input handle to enable mouse input
    g_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

    DWORD mode;
    GetConsoleMode(g_hConsoleInput, &mode);                     // Disable Quick Edit Mode
    SetConsoleMode(g_hConsoleInput, ENABLE_MOUSE_INPUT | mode & ~ENABLE_QUICK_EDIT_MODE);

    // Set the console to UTF-8 mode
    err = _setmode(_fileno(stdout), _O_U8TEXT);
    if (err == -1) RUN = -1;

    // Set Version into window title
    wchar_t winTitle[30] = {0};
    wcscpy_s(winTitle, L"JoySender++ tUI ");
    wcscat_s(winTitle, APP_VERSION_NUM);
    SetConsoleTitleW(winTitle);

    // Set Version into backdrop
    {
        constexpr int versionStartPoint = 73 * 3 + 29;
        const size_t verLength = wcslen(APP_VERSION_NUM);

        for (int i = 0; i < verLength; i++) {
            JoySendMain_Backdrop[versionStartPoint + i] = APP_VERSION_NUM[i];
        }
    }

    loadIPDataFromFile();

    hideConsoleCursor();

    while (RUN > 0) {
        RUN = joySendertUI(args);

        if (RUN > 1){
            args.mode = RUN - 1;
            args.select = false;
        }
        wait_for_no_keyboard_input();
    }

    // Cleanup and quit
    CLEAN_EGGS();
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