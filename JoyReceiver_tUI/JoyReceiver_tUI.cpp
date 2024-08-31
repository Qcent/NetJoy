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
    Arguments args = parse_arguments(argc, argv);
    FPSCounter fps_counter;
    TCPConnection server(args.port);
    PVIGEM_TARGET gamepad;
    XUSB_REPORT xbox_report;
    DS4_REPORT_EX ds4_report_ex;

    int allGood;
    char buffer[128];
    int buffer_size = sizeof(buffer);
    int bytesReceived;
    std::wstring externalIP;
    std::wstring localIP;
    char connectionIP[INET_ADDRSTRLEN];
    std::wstring clientIP;
    std::string fpsOutput;
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
    const auto vigemClient = vigem_alloc();
    if (vigemClient == nullptr)
    {
        //setErrorMsg(L" << Not Enough Memory To Initialize ViGEm >> ", 46);
        std::cerr << "Uh, not enough memory to initialize ViGEM gamepad?!" << std::endl;
        //return -1;
        APP_KILLED = 1;
    }
    // Connect to ViGEM Bus
    auto vigemErr = vigem_connect(vigemClient);
    if (!VIGEM_SUCCESS(vigemErr))
    {
        //setErrorMsg(L" << ViGEm Bus Connection Failed >> ", 36);
        std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << vigemErr << std::endl;
        //return -1;
        APP_KILLED = 1;
    }

    // Quit if error occurred
    if (APP_KILLED) {
        return -1;
    }

    // Get IPs and start server
    externalIP = g_converter.from_bytes(server.get_external_ip());
    localIP = g_converter.from_bytes(server.get_local_ip());
    server.set_silence(true);
    server.start_server();
    //server.set_silence(false); // we don't want the server talking on our tUI screens


    //
    // Set Up Console Window

    // Get the console input handle to enable mouse input
    g_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(g_hConsoleInput, &mode);                     // Disable Quick Edit Mode // working??
    SetConsoleMode(g_hConsoleInput, ENABLE_MOUSE_INPUT | mode & ~ENABLE_QUICK_EDIT_MODE);

    // Set the console to UTF-8 mode
    auto _ =_setmode(_fileno(stdout), _O_U8TEXT);

    // Set Version into window title
    wchar_t winTitle[30];
    wcscpy(winTitle, L"JoyReceiver++ tUI ");
    wcscat(winTitle, APP_VERSION_NUM);
    SetConsoleTitleW(winTitle);
    
    hideConsoleCursor();

    // Set Version into backdrop
    {
        constexpr int versionStartPoint = 73 * 3 + 31;
        const int verLength = wcslen(APP_VERSION_NUM);

        for (int i = 0; i < verLength; i++) {
            JoyRecvMain_Backdrop[versionStartPoint + i] = APP_VERSION_NUM[i];
        }
    }

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

        // Wait to establish a connection in separate thread while animating the screen
        ESTABLISH_ANIMATED_CX_tUI();

        if (allGood == WSAEINVAL) {
            swprintf(errorPointer, 52, L" Unable to use port : %d \r\n Program will exit", args.port);
            MessageBox(NULL, errorPointer, L"JoyReceiver Error", MB_ICONERROR | MB_OK);
        }

        if (APP_KILLED) break;
        if (allGood < 0) {
            setErrorMsg(L" << Error While Establishing Connection >> ", 44);
            break;
        }

        // clientSocket will have inherited non blocking mode
        // Return both sockets to blocking mode
        server.set_client_blocking(true);
        server.set_server_blocking(true);

        // set wstring clientIP
        clientIP = g_converter.from_bytes(connectionIP);

        //
        // Receive Operating Mode and Client Timing
        bytesReceived = server.receive_data(buffer, buffer_size);
        if (bytesReceived < 1) {

            int len = clientIP.size() + 33;
            swprintf(errorPointer, len, L" << Connection From: %s Failed >> ", clientIP.c_str());
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
            break;
        }
        std::vector<std::string> split_settings = split(std::string(buffer, bytesReceived), ':');
        int client_timing = std::stoi(split_settings[0]);
        int op_mode = (split_settings.size() > 1) ? std::stoi(split_settings[1]) : 0;
        g_mode = op_mode;

        if (op_mode == 2) { // Emulating a DS4 controller
            gamepad = vigem_target_ds4_alloc();
        }
        else {              // Emulating an XBOX360 controller
            gamepad = vigem_target_x360_alloc();
        }

        // Add gamepad to the vigemClient bus, this equals a plug-in event
        vigemErr = vigem_target_add(vigemClient, gamepad);
        if (!VIGEM_SUCCESS(vigemErr))
        {
            setErrorMsg(L" << Virtual Gamepad Plugin Failed >> ", 38);
            errorOut.Draw();
            //std::cerr << "Virtual Gamepad plugin failed with error code: 0x" << std::hex << vigemErr << std::endl;
            APP_KILLED = 1;
        }

        // Register Rumble callback notifications
        if (op_mode == 2) {

#if 0
            std::thread ds4RumbleThread([&vigemClient, &gamepad]() {
                PDS4_OUTPUT_BUFFER buffer = {};
                while (!APP_KILLED) {
                    // Perform computations and update shared variables
                    auto vigemErr = vigem_target_ds4_await_output_report(vigemClient, gamepad, buffer);
                    if (!VIGEM_SUCCESS(vigemErr)) {
                        std::cerr << "Registering DS4 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
                        // APP_KILLED = 1;
                    }
                    else {
                        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex

                        buffer;
                        // Update the shared variable from buffer
                        // feedbackData = static_cast<char>(LargeMotor);
                        // feedbackData += static_cast<char>(SmallMotor);
                    }
                }
                });
#else
            
            #pragma warning(disable : 4996)
            vigemErr = vigem_target_ds4_register_notification(vigemClient, gamepad, &ds4_rumble, &feedbackData);
            if (!VIGEM_SUCCESS(vigemErr))
            {
                setErrorMsg(L" << Registering DS4 Rumble Callback Failed >> ", 47);
                errorOut.Draw();
                //std::cerr << "Registering DS4 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
            }
#endif
        }
        else {
            vigemErr = vigem_target_x360_register_notification(vigemClient, gamepad, &xbox_rumble, &feedbackData);
            if (!VIGEM_SUCCESS(vigemErr))
            {
                setErrorMsg(L" << Registering 360 Rumble Callback Failed >> ", 47);
                errorOut.Draw();
                //std::cerr << "Registering 360 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
            }
        }


        //
        // Send response back to client
        feedbackData = "Go for Joy!";
        allGood = server.send_data(feedbackData.c_str(), static_cast<int>(feedbackData.length()));
        if (allGood < 1) {
            int len = clientIP.size() + 30;
            swprintf(errorPointer, len, L" << Connection To: %s Failed >> ", clientIP.c_str());
            errorOut.SetWidth(len);
            errorOut.SetText(errorPointer);
            break;
        }
        
        // prep for loop
        feedbackData = "Rumble Data";
            // set UI
        {
            screen.ClearButtons();

            // backdrop
            int QUITLINE;
            if (op_mode == 2) {
                screen.SetBackdrop(DS4_Backdrop);
                QUITLINE = DS4_QUIT_LINE;
                BuildDS4Face();
            }
            else {
                screen.SetBackdrop(XBOX_Backdrop);
                QUITLINE = XBOX_QUIT_LINE;
                BuildXboxFace();
            }

            // messages
            swprintf(msgPointer1, 43, L" << Connection From: %s  >> ", clientIP.c_str());
            output1.SetText(msgPointer1);
            output1.SetPosition(3, 1, 43, 1, ALIGN_LEFT);
            output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);

            swprintf(clientPointer, 16, L" %s ", clientIP.c_str());
            clientMsg.SetText(clientPointer);
            clientMsg.SetPosition(23, 1, 38, 1, ALIGN_LEFT);
            clientMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);

            // buttons
            SetControllerButtonPositions(op_mode);
            quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE);
            newColorsButton.SetPosition(consoleWidth / 2 - 8, QUITLINE - 2);
            screen.AddButton(&quitButton);
            screen.AddButton(&newColorsButton);
            

            // colors / drawing
                // set controller to color scheme colors with some contrast correction for bg color then draws the controller face
            DrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, op_mode);

            tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);

                // color non controller buttons and draw them
            screen.SetButtonsColors(buttonColors);
            screen.DrawButtons();

            output1.Draw();
            clientMsg.Draw();
        }

        fps_counter.reset();

        //
        // Start Receive Joystick Data Loop
        while (!APP_KILLED) {

            screenLoop(screen);

            //
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
                int len = clientIP.size() + 31;
                swprintf(errorPointer, len, L" << Connection From: %s Lost >> ", clientIP.c_str());
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
            allGood = server.send_data(feedbackData.c_str(), static_cast<int>(feedbackData.length()));
            if (allGood < 1) {
                int len = clientIP.size() + 29;
                swprintf(errorPointer, len, L" << Connection To: %s Lost >> ", clientIP.c_str());
                errorOut.SetWidth(len);
                errorOut.SetText(errorPointer);
                break;
            }

            // FPS output
            fpsOutput = do_fps_counting();
            if (!fpsOutput.empty()) {
                updateFPS(g_converter.from_bytes(fpsOutput + "   ").c_str(), 8);
            }
            
        }
        
        if (!APP_KILLED) {
            screen.ClearButtons();
        }

        // Unregister rumble callbacks // deprecated?
        if (op_mode == 2) vigem_target_ds4_unregister_notification(gamepad);
        else vigem_target_x360_unregister_notification(gamepad);
        // Free resources (this disconnects the virtual device)
        vigem_target_remove(vigemClient, gamepad);
        vigem_target_free(gamepad);
    }


    // Release connection to the ViGEM Bus
    vigem_disconnect(vigemClient);
    vigem_free(vigemClient);

    // Close TCPConnection
    server.~TCPConnection();

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