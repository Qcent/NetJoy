// JoyReceiver++.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

#include "TCPConnection.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"
#include "JoyReceiver++.h"

// for testing
void displayBytes(BYTE* buffer, DWORD bufferSize) {
    for (DWORD i = 0; i < bufferSize; i++) {
        // Check if the current byte is "CC"
        if (buffer[i] == 0xCC) {
            // Check if the next three bytes are also "CC"
            if (i + 3 < bufferSize && buffer[i + 1] == 0xCC && buffer[i + 2] == 0xCC && buffer[i + 3] == 0xCC) {
                // std::cout << "\nEncountered CC sequence, exiting..." << std::endl;
                printf("\r\n");
                return;  // Terminate the function
            }
        }
        printf("%02X ", buffer[i]);
    }
    printf("\r\n");
}

int main(int argc, char* argv[]) {
    Arguments args = parse_arguments(argc, argv);
    FPSCounter fps_counter;
    FPSCounter latencyTimer;
    TCPConnection server(args.port);
    PVIGEM_TARGET gamepad;
    XUSB_REPORT xbox_report;
    //DS4_REPORT ds4_report;
    DS4_REPORT_EX ds4_report_ex;

    int allGood;
    UINT8 ErrCount = 0;
    char buffer[128];
    int buffer_size = sizeof(buffer);
    int bytesReceived;
    double expectedFrameDelay = 0;
    std::string externalIP;
    std::string localIP;
    std::string fpsOutput;
    bool LIKELY_NETWORK_DISSCONNECT = false; // a hack to fix unacknowledged disconnection bug, detected via fps spike
    auto do_fps_counting = [&fps_counter, &LIKELY_NETWORK_DISSCONNECT](int report_frequency = 30) {
        if (fps_counter.increment_frame_count() >= report_frequency) {
            double fps = fps_counter.get_fps();
            // sometimes a network disconnect is not detected via bytesReceived but fps will skyrocket
            if (fps > 1000) LIKELY_NETWORK_DISSCONNECT = true;
            fps_counter.reset();
            return formatDecimalString(std::to_string(fps), 2);
        }
        return std::string();
        };
    double latencyOutput;
    const int latency_report_freq = 25;
    auto do_latency_timing = [&latencyTimer](int report_frequency = 15) {
        if (latencyTimer.increment_frame_count() >= report_frequency) {
            double elapsedTime = latencyTimer.get_elapsed_time();
            latencyTimer.reset();
            return elapsedTime / report_frequency;
        }
        return 0.0;
    };

    // Register the signal handler function
    std::signal(SIGINT, signalHandler);

    hideConsoleCursor();

    // Initialize ViGEM Bus connection
    const auto vigemClient = vigem_alloc();
    if (vigemClient == nullptr)
    {
        std::cerr << "Uh, not enough memory to initialize ViGEM gamepad?!" << std::endl;
        //return -1;
        APP_KILLED = 1;
    }
    // Connect to ViGEM Bus
    auto vigemErr = vigem_connect(vigemClient);
    if (!VIGEM_SUCCESS(vigemErr))
    {
        std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << vigemErr << std::endl;
        //return -1;
        APP_KILLED = 1;
    }


    externalIP = server.get_external_ip();
    localIP = server.get_local_ip();
    server.set_silence(true);
    server.start_server();
    server.set_silence(false);

    std::system("cls");
    ///********************************
    // Make Connection Loop
    while (!APP_KILLED) {
        ErrCount = 0;
        std::cout << "Waiting for Connection on port : " << args.port
            << "\n\t\t LAN : " << localIP << "\n\t\t WAN : " << externalIP << std::endl;

        //
        // Await Connection in Non Blocking Mode
        //
        server.set_server_blocking(false);
        std::pair<SOCKET, sockaddr_in> connectionResult;
        SOCKET clientSocket;
        while (!APP_KILLED) {
            // Attempt to accept a client connection in non Blocking mode
            connectionResult = server.await_connection();
            clientSocket = connectionResult.first;
            if (clientSocket == INVALID_SOCKET) {
                allGood = WSAGetLastError();
                if (allGood == WSAEWOULDBLOCK) {
                    // No client connection is immediately available
                    Sleep(10); // Sleep for 10 milliseconds
                }
                else {
                    // An error occurred during accept()
                    std::cerr << "Failed to accept client connection: " << allGood << std::endl;
                    ++ErrCount;
                    if (ErrCount > 10) { 
                        std::cout << "<< Exiting >>" << std::endl;
                        APP_KILLED = true; 
                    }
                    break; // Exit the loop or handle the error condition
                }
            }
            else {
                // A client connection is established
                // Process the connection
                // Create a new thread or perform other operations on the clientSocket?
                
                // Break the loop, or this will continue attempting to accept more connections
                break;
            }
        }
        if (APP_KILLED) break;


        // clientSocket will have inherited non blocking mode
        // Return both sockets to blocking mode
        server.set_client_blocking(true);
        server.set_server_blocking(true);


        //
        // Receive Operating Mode and Client Timing
        bytesReceived = server.receive_data(buffer, buffer_size);
        if (!bytesReceived) {
            std::cout << "<< Connection Failed >>" << std::endl;
            break;
        }
        std::vector<std::string> split_settings = split(std::string(buffer, bytesReceived), ':');
        int client_timing = std::stoi(split_settings[0]);
        int op_mode = (split_settings.size() > 1) ? std::stoi(split_settings[1]) : 0;
        expectedFrameDelay = 1000 / client_timing;

        std::cout << "Client Timing: " << client_timing << "fps" << "\t\t" << "Running in Mode: " << op_mode <<   std::endl;
        

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
            std::cerr << "Virtual Gamepad plugin failed with error code: 0x" << std::hex << vigemErr << std::endl;
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
                std::cerr << "Registering DS4 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
                //APP_KILLED = 1;
            }
#endif
        }
        else {
            vigemErr = vigem_target_x360_register_notification(vigemClient, gamepad, &xbox_rumble, &feedbackData);
            if (!VIGEM_SUCCESS(vigemErr))
            {
                std::cerr << "Registering 360 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
                //APP_KILLED = 1;
            }
        }


        //
        // Send response back to client
        feedbackData = "Go for Joy!";
        allGood = server.send_data(feedbackData.c_str(), static_cast<int>(feedbackData.length()));
        if (!allGood) {
            std::cout << "<< Connection Failed >>" << std::endl;
            break;
        }
        
        // prep for loop
        feedbackData = "Rumble Data";
        std::cout << std::endl << std::endl;
        fps_counter.reset();

        //
        // Start Receive Joystick Data Loop
        while (!APP_KILLED) {
            //*****************************
            // Receive joystick input from client to the buffer
            bytesReceived = server.receive_data(buffer, buffer_size);
            if (!bytesReceived) break;

            // FPS output
            if (args.latency) {
                fpsOutput = do_fps_counting();  
                if (!fpsOutput.empty()) {
                    overwriteFPS("FPS: " + fpsOutput);
                }
                latencyOutput = do_latency_timing(latency_report_freq);
                if (latencyOutput) {
                    overwriteLatency("Latency: " + formatDecimalString(std::to_string((latencyOutput * 1000)- expectedFrameDelay), 5) + " ms    ");
                }
            }

            if (LIKELY_NETWORK_DISSCONNECT) {
                LIKELY_NETWORK_DISSCONNECT = false;
                break;
            }

            //******************************
            // Update virtual gamepad
            if (op_mode == 2) {
                // Cast the buffer to an DS4_REPORT_EX pointer
                ds4_report_ex = *reinterpret_cast<DS4_REPORT_EX*>(buffer);
                vigem_target_ds4_update_ex(vigemClient, gamepad, ds4_report_ex);
               /// ds4_report = *reinterpret_cast<DS4_REPORT*>(buffer);
               /// vigem_target_ds4_update(vigemClient, gamepad, ds4_report);
            }
            else {
                // Cast the buffer to an XUSB_REPORT pointer
                xbox_report = *reinterpret_cast<XUSB_REPORT*>(buffer);
                vigem_target_x360_update(vigemClient, gamepad, xbox_report);
            }


            //*******************************
            // Send response back to client :: Rumble data
            allGood = server.send_data(feedbackData.c_str(), static_cast<int>(feedbackData.length()));
            if (!allGood) break;
        }
        
        if (!APP_KILLED) {
            std::system("cls");
            std::cout << "<< Connection Lost >>" << std::endl;
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
    showConsoleCursor();
    Sleep(7);
    std::cout << "all cleaned up! (:" << std::endl;
    return 1;
}

void signalHandler(int signal) {
    if (signal == SIGINT) {
        APP_KILLED = 1;
        Sleep(5);
        std::cout << "Keyboard interrupt received. Exiting gracefully." << std::endl;
    }
}