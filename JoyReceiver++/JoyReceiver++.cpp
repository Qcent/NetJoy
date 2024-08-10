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

#include "TCPConnection.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"
#include "JoyReceiver++.h"

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
        r.Right = width - 1;
        r.Bottom = height - 1;
        SetConsoleWindowInfo(hConOut, TRUE, &r);

        c.X = width;
        c.Y = height;
        SetConsoleScreenBufferSize(hConOut, c);
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

console con(consoleWidth + 2, consoleHeight + 2);

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
    _setmode(_fileno(stdout), _O_U8TEXT);

    // Set Version into window title
    wchar_t winTitle[30];
    wcscpy(winTitle, L"JoyReceiver++ tUI ");
    wcscat(winTitle, APP_VERSION_NUM);
    SetConsoleTitleW(winTitle);
    
    hideConsoleCursor();

    // Set Version into backdrop
    {
        int versionStartPoint = 73 * 3 + 31;
        const int verLength = wcslen(APP_VERSION_NUM);

        for (int i = 0; i < verLength; i++) {
            JoyRecvMain_Backdrop[versionStartPoint + i] = APP_VERSION_NUM[i];
        }
    }

    //
    // Set up for tUI screen

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
    setErrorMsg(L"\0", 1); // initialize error message as empty string
    fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT);                // fps output
    quitButton.setCallback(&exitAppCallback);
    newColorsButton.setCallback(&newControllerColorsCallback);
    
    // colors   
    errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2); 
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
    
    ///********************************

    // Make Connection Loop
    while (!APP_KILLED) {

        // Set and Draw connection screen
        {
            WORD headingColor = makeSafeColors(fullColorSchemes[g_currentColorScheme].controllerColors.col4);
            output2.SetColor(headingColor); // for connection animation
            screen.SetBackdrop(JoyRecvMain_Backdrop);
            screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
            quitButton.SetPosition(11, 17);
            screen.AddButton(&quitButton);
            screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
            screen.ReDraw();

            errorOut.SetPosition(consoleWidth / 2, 5, 50, 0, ALIGN_CENTER);
            errorOut.Draw();

            output1.SetPosition(consoleWidth / 2 + 2, 7, 50, 1, ALIGN_CENTER);
            output1.SetText(L" Waiting For Connection ");
            output1.SetColor(headingColor);
            output1.Draw();

            // Show PORT and IPs
            setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
            setCursorPosition(26, 9);
            std::wcout << L" ";
            std::wcout << args.port;
            std::wcout << L" ";

            setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
            setCursorPosition(28, 11);
            std::wcout << L" " + localIP + L" ";

            setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
            setCursorPosition(28, 13);
            std::wcout << L" " + externalIP + L" ";
        }


        // Wait to establish a connection in separate thread while animating the screen
        {
            // set up animation variables
            int frameDelay = 0;
            int footFrameNum = 0;
            mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t");
            mouseButton rightBorderPiece(68, 18, 5, L"\t/   \t\t:}-     ");
            leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);

            mouseButton ani2(3, 18, 10, FooterAnimation[footFrameNum]);
            ani2.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
            int lastAni1Frame = g_frameNum-1;
            int lastAni2Frame = footFrameNum-1;
            
            // set up thread
            //int runFrame = 1;
            allGood = WSAEWOULDBLOCK;
            std::thread connectThread(threadedAwaitConnection, std::ref(server), std::ref(allGood), std::ref(connectionIP));
            //std::thread frameThread(threadedFrameAdvance, std::ref(runFrame), 500, std::ref(g_frameNum), CX_ANI_FRAME_COUNT);
            
            while (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
                // animation 1
                    // draw
                if (lastAni2Frame != g_frameNum) {
                    lastAni2Frame = g_frameNum;
                    output2.SetText(ConnectAnimationLeft[g_frameNum]);
                    output2.SetPosition((consoleWidth / 2) - 16, 7); // left side
                    output2.Draw();

                    output2.SetText(ConnectAnimationRight[g_frameNum]);
                    output2.SetPosition((consoleWidth / 2) + 15, 7); // right side
                    output2.Draw();
                }
                    // advance frame
                if (frameDelay % 13 == 0) {
                    loopCount(g_frameNum, CX_ANI_FRAME_COUNT);
                }

                // animation 2
                    // draw
                if (lastAni2Frame != footFrameNum) {
                    lastAni2Frame = footFrameNum;
                    ani2.SetText(FooterAnimation[footFrameNum]);

                    for (int i = 0; i < 7; ++i) {
                        ani2.SetPosition(3+(i*10), 18);
                        ani2.Draw();
                    }

                    // redraw border pieces
                    leftBorderPiece.Draw();
                    rightBorderPiece.Draw();
                }
                    // advance frame
                if (frameDelay % 4 == 0) {
                    if ((frameDelay % 600) <= 300) {
                        revLoopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT);  // reverse loop
                    }
                    else {

                        loopCount(footFrameNum, FOOTER_ANI_FRAME_COUNT);  // loop
                        //countUpDown(g_frameNum, CX_ANI_FRAME_COUNT);    // bounce
                    }
                }

                // check input
                checkForQuit();
                screenLoop(screen);
                
                if (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
                    Sleep(20);
                    ++frameDelay;
                }
            }
            //runFrame = 0;
            connectThread.detach();
            //frameThread.detach();
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
        if (!bytesReceived) {

            int len = clientIP.size() + 33;
            swprintf(errorPointer, len, L" << Connection From: %s Failed >> ", clientIP);
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
            //return -1;
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
                //APP_KILLED = 1;
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
                //APP_KILLED = 1;
            }
        }


        //
        // Send response back to client
        feedbackData = "Go for Joy!";
        allGood = server.send_data(feedbackData.c_str(), static_cast<int>(feedbackData.length()));
        if (!allGood) {
            int len = clientIP.size() + 30;
            swprintf(errorPointer, len, L" << Connection To: %s Failed >> ", clientIP);
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
            if (!bytesReceived) {
                int len = clientIP.size() + 31;
                swprintf(errorPointer, len, L" << Connection From: %s Lost >> ", clientIP);
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
            if (!allGood) {
                int len = clientIP.size() + 29;
                swprintf(errorPointer, len, L" << Connection To: %s Lost >> ", clientIP);
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