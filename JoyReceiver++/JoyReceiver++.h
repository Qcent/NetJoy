#pragma once
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

volatile sig_atomic_t APP_KILLED = 0;
std::mutex mtx;
char feedbackData[5] = { 0 }; // For sending rumble data back to joySender
void signalHandler(int signal);

#include "utilities.hpp"

std::thread ds4Rumbler;
bool ds4ThreadStop = true;
void ds4RumbleThread(PVIGEM_CLIENT vigemClient, PVIGEM_TARGET gamepad) {
    DS4_OUTPUT_BUFFER buffer;
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    while (!APP_KILLED && !ds4ThreadStop) {

        auto vigemErr = vigem_target_ds4_await_output_report_timeout(vigemClient, gamepad, 3000, &buffer);
        if (!VIGEM_SUCCESS(vigemErr) && vigemErr != VIGEM_ERROR_TIMED_OUT) {
            std::cerr << "DS4 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
            lock.lock();
            std::memset(feedbackData, '\0', sizeof(feedbackData));
            lock.unlock();
        }
        else if (vigemErr != VIGEM_ERROR_TIMED_OUT) {
#if 0 
            auto displayBytes = [](BYTE* buffer, DWORD bufferSize) {
                for (DWORD i = 0; i < bufferSize; i++) {
                    if (i % 18 == 0) printf("\r\n");
                    printf("%02X ", buffer[i]);
                }
                printf("\r\n");
                };

            std::cout << "Buffer Data: ";
            displayBytes(buffer.Buffer, 64);
            repositionConsoleCursor(-5, 0);
#endif
            lock.lock();
            std::memcpy(feedbackData, &buffer.Buffer[4], 5 * sizeof(char));
            lock.unlock();
        }
    }
}

// Define the callback function for x3360 rumble support 
VOID CALLBACK _Function_class_(EVT_VIGEM_X360_NOTIFICATION)
xbox_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target,
    UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData){
#if 0
    static int count = 1;
    repositionConsoleCursor(-2, 6);
    std::cout << "Rumble Calls:" << count++ << "\t ";
    repositionConsoleCursor(0, 22);
    std::cout << "LargeMotor:" << (int)LargeMotor << "   ";
    repositionConsoleCursor(0, 37);
    std::cout << "SmallMotor:" << (int)SmallMotor << "   " << "   ";
    repositionConsoleCursor(2, 0);
#endif
    // Set the UserData with the desired value
    feedbackData[0] = static_cast<char>(LargeMotor);
    feedbackData[1] = static_cast<char>(SmallMotor);
}

// xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
// Defined Common Functionality

#define JOYRECEIVER_INIT_VARIABLES() \
Arguments args = parse_arguments(argc, argv); \
FPSCounter fps_counter; \
FPSCounter latencyTimer; \
TCPConnection server(args.port); \
PVIGEM_TARGET gamepad; \
XUSB_REPORT xbox_report = {0}; \
DS4_REPORT_EX ds4_report_ex = {0}; \

std::unique_lock<std::mutex> lock(mtx, std::defer_lock); \
int allGood; \
UINT8 connection_error_count = 0; \
char buffer[64]; \
int buffer_size = sizeof(buffer); \
int bytesReceived = 0; \
int op_mode = 0; \
int client_timing = 0; \
double expectedFrameDelay = 0; \
std::string externalIP; \
std::string localIP; \
std::string fpsOutput; 
 

#define JOYRECEIVER_INIT_VIGEM_BUS() \
const auto vigemClient = vigem_alloc(); \
if (vigemClient == nullptr) { \
    std::cerr << "Uh, not enough memory to initialize ViGEM gamepad?!" << std::endl; \
    APP_KILLED = 1; \
} \
auto vigemErr = vigem_connect(vigemClient); \
if (!VIGEM_SUCCESS(vigemErr)){ \
    std::cerr << "ViGEm Bus connection failed with error code: 0x" << std::hex << vigemErr << std::endl; \
    APP_KILLED = 1; \
}

#define JOYRECEIVER_DETERMINE_IPS_START_SERVER() \
externalIP = server.get_external_ip(); \
localIP = server.get_local_ip(); \
server.set_silence(true); \
server.start_server(); 

#define JOYRECEIVER_CONSOLE_AWAIT_CONNECTION() \
{ \
    server.set_server_blocking(false); \
    std::pair<SOCKET, sockaddr_in> connectionResult; \
    SOCKET clientSocket; \
    while (!APP_KILLED) { \
        connectionResult = server.await_connection(); \
        clientSocket = connectionResult.first; \
        if (clientSocket == INVALID_SOCKET) { \
            allGood = WSAGetLastError(); \
            if (allGood == WSAEWOULDBLOCK) { \
                Sleep(10); \
            } else if (allGood == WSAEINVAL) { \
                std::cout << " << Unable to use port : " << args.port << " >>\r\n"; \
                std::cout << "<< Exiting >>" << std::endl; \
                APP_KILLED = true; \
                break; \
            } else { \
                std::cerr << "Failed to accept client connection: " << allGood << std::endl; \
                ++connection_error_count; \
                if (connection_error_count > 10) { \
                    std::cout << "<< Multiple Network Errors : Exiting >>" << std::endl; \
                    APP_KILLED = true; \
                    break; \
                } \
            } \
        } else { \
            connection_error_count = 0; \
            break; \
        } \
    } \
    if (!APP_KILLED) { \
        server.set_client_blocking(true); \
        server.set_server_blocking(true); \
    } \
}

#define JOYRECEIVER_GET_MODE_AND_TIMING_FROM_BUFFER() \
{\
    std::vector<std::string> split_settings = split(std::string(buffer, bytesReceived), ':'); \
    client_timing = std::stoi(split_settings[0]); \
    op_mode = (split_settings.size() > 1) ? std::stoi(split_settings[1]) : 0; \
    expectedFrameDelay = 1000.0 / client_timing; \
}

#define JOYRECEIVER_PLUGIN_VIGEM_CONTROLLER() \
{ \
    /* Check operating mode and allocate the appropriate gamepad */ \
    if (op_mode == 2) { /* Emulating a DS4 controller */ \
        gamepad = vigem_target_ds4_alloc(); \
    } else { /* Emulating an XBOX360 controller */ \
        gamepad = vigem_target_x360_alloc(); \
    } \
\
    /* Add gamepad to the ViGEm client bus, this equals a plug-in event */ \
    vigemErr = vigem_target_add(vigemClient, gamepad); \
    if (!VIGEM_SUCCESS(vigemErr)) { \
        std::cerr << "Virtual Gamepad plugin failed with error code: 0x" << std::hex << vigemErr << std::endl; \
        APP_KILLED = 1; \
    } \
\
    /* Register 360 rumble callback or spin up DS4 feedback thread */ \
    if (op_mode == 2) { \
        ds4ThreadStop = false; \
        ds4Rumbler = std::thread(ds4RumbleThread, std::ref(vigemClient), std::ref(gamepad)); \
        ds4Rumbler.detach(); \
    } else { \
        vigemErr = vigem_target_x360_register_notification(vigemClient, gamepad, &xbox_rumble, &feedbackData); \
        if (!VIGEM_SUCCESS(vigemErr)) { \
            std::cerr << "Registering 360 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl; \
        } \
    } \
}


#define JOYRECEIVER_UNPLUG_VIGEM_CONTROLLER() \
{ \
    if (op_mode == 2) ds4ThreadStop = true; \
    else vigem_target_x360_unregister_notification(gamepad); \
    /* Free resources(this disconnects the virtual device) */ \
    vigem_target_remove(vigemClient, gamepad); \
    vigem_target_free(gamepad); \
}

#define JOYRECEIVER_SHUTDOWN_VIGEM_BUS() \
{ \
    vigem_disconnect(vigemClient); \
    vigem_free(vigemClient); \
}