#pragma once
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>
#include <thread>
#include <mutex>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

constexpr auto APP_NAME = "NetJoy";
#define APP_VERSION_NUM     L"3.0.4.0"
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
UDP_COMMUNICATION = args.udp; \
FPSCounter fps_counter; \
FPSCounter latencyTimer; \
NetworkConnection server(args.udp, args.port); \
PVIGEM_TARGET gamepad; \
XUSB_REPORT xbox_report = {0}; \
DS4_REPORT_EX ds4_report_ex = {0}; \

std::unique_lock<std::mutex> lock(mtx, std::defer_lock); \
int allGood; \
UINT8 connection_error_count = 0; \
char feedBackComp[5] = { 0 }; \
char buffer[64] = { 0 }; \
int buffer_size = sizeof(buffer); \
int bytesReceived = 0; \
int op_mode = 0; \
int client_timing = 0; \
double expectedFrameDelay = 0; \
std::string externalIP; \
std::string localIP; \
std::string fpsOutput; \
char connectionIP[INET_ADDRSTRLEN];
 

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
server.start_as_server(args.port); 

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
            sockaddr_in clientAddress = connectionResult.second; \
            inet_ntop(AF_INET, &(clientAddress.sin_addr), connectionIP, INET_ADDRSTRLEN); \
            break; \
        } \
    } \
    if (!APP_KILLED) { \
        server.set_client_blocking(true); \
        server.set_server_blocking(true); \
        server.set_client_timeout(NETWORK_TIMEOUT_MILLISECONDS); \
    } \
}

void JOYRECEIVER_GET_MODE_AND_TIMING_FROM_BUFFER(const char* buffer, const int bytesReceived, int& client_timing, int& op_mode, double& expectedFrameDelay){
    try {
        std::vector<std::string> split_settings = split(std::string(buffer, bytesReceived), ':');
        client_timing = std::stoi(split_settings[0]);
        op_mode = (split_settings.size() > 1) ? std::stoi(split_settings[1]) : 0;
        expectedFrameDelay = 1000.0 / client_timing;
    }
    catch (...) {
        std::cerr << "\r\n<< Illegal connection attempted: program exiting >>" << std::endl;
        std::cerr << " Received from " << connectionIP << ": (" << bytesReceived << " bytes)" << std::endl;
        displayBytes(reinterpret_cast<const byte*>(buffer), bytesReceived);
        std::cerr << std::endl << std::string(buffer, bytesReceived) << std::endl;
        std::cerr << "\n\t<< PRESS ESCAPE TO QUIT >>" << std::endl;
        while (!APP_KILLED) {
            Sleep(50);
            if (checkKey(VK_ESCAPE, IS_RELEASED) && IsAppActiveWindow()) {
                APP_KILLED = true;
            }
            op_mode = -1;
        }
    }
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


#ifdef NetJoy_tUI
#define JR_PSP \
    swprintf(errorPointer, 50, L" << Special Signal: %d >> ", pkt->type); \
    errorOut.SetWidth(50); \
    errorOut.SetText(errorPointer); \
    errorOut.Draw();

#define ALIVE_PACKET_SIGNALS_MAPPING() \
    if(pkt->type == UDPConnection::PACKET_ALIVE){ \
        bytesReceived = -WSAETIMEDOUT; \
        reset = false; \
    } 

#else
#define JR_PSP
#define ALIVE_PACKET_SIGNALS_MAPPING()
#endif

#define JOYRECEIVER_PROCESS_SIGNAL_PACKET() \
{ \
    bool reset = true; \
    UDPConnection::SIGPacket* pkt = (UDPConnection::SIGPacket*)buffer; \
    JR_PSP \
    if(pkt->type == UDPConnection::PACKET_HANGUP){ \
        break; \
    } \
    ALIVE_PACKET_SIGNALS_MAPPING(); \
    /* Do not process as input report, jump to receive: and get more data*/ \
    if(reset) goto receive; \
}

// with Nagle off we sometimes receive partial packets
#define JOYRECEIVER_GET_COMPLETE_PACKET() \
{ \
    int filled = bytesReceived; \
    while (!APP_KILLED && filled) { \
        bytesReceived = server.receive_data(buffer + filled, buffer_size - filled); \
        if (bytesReceived < 1) break; \
        filled += bytesReceived; \
        if (filled == buffer_size) { \
            filled = 0; \
        } \
    } \
}

#if DEVTEST
// for visual on 6 axis data
void output_extra_ds4_data(DS4_REPORT_EX& report) {
    static uint8_t count = 0;

    static int maxX = 0, maxY = 0, maxZ = 0;
    static int minX = 0, minY = 0, minZ = 0;
    static int maxXg = 0, maxYg = 0, maxZg = 0;
    static int minXg = 0, minYg = 0, minZg = 0;

    // --- Accelerometer ---
    if (report.Report.wAccelX > maxX) maxX = report.Report.wAccelX;
    else if (report.Report.wAccelX < minX) minX = report.Report.wAccelX;

    if (report.Report.wAccelY > maxY) maxY = report.Report.wAccelY;
    else if (report.Report.wAccelY < minY) minY = report.Report.wAccelY;

    if (report.Report.wAccelZ > maxZ) maxZ = report.Report.wAccelZ;
    else if (report.Report.wAccelZ < minZ) minZ = report.Report.wAccelZ;

    // --- Gyroscope ---
    if (report.Report.wGyroX > maxXg) maxXg = report.Report.wGyroX;
    else if (report.Report.wGyroX < minXg) minXg = report.Report.wGyroX;

    if (report.Report.wGyroY > maxYg) maxYg = report.Report.wGyroY;
    else if (report.Report.wGyroY < minYg) minYg = report.Report.wGyroY;

    if (report.Report.wGyroZ > maxZg) maxZg = report.Report.wGyroZ;
    else if (report.Report.wGyroZ < minZg) minZg = report.Report.wGyroZ;

    if (checkKey('G', IS_PRESSED)) {
        maxX = 0, maxY = 0, maxZ = 0;
        minX = 0, minY = 0, minZ = 0;
        maxXg = 0, maxYg = 0, maxZg = 0;
        minXg = 0, minYg = 0, minZg = 0;
    }

    if ((++count % 10) > 0) return;

#if defined(NetJoyTUI) 
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
    std::wcout << L"X: " << report.Report.wAccelX << L" \tXg: " << report.Report.wGyroX << L"\tmax: " << maxX << L"  min: " << minX << L"  maxG: " << maxXg << L"  minG: " << minXg << L"       \r\n"; // pitch 
    std::wcout << L"Y: " << report.Report.wAccelY << L" \tYg: " << report.Report.wGyroY << L"\tmax: " << maxY << L"  min: " << minY << L"  maxG: " << maxYg << L"  minG: " << minYg << L"       \r\n"; // yaw
    std::wcout << L"Z: " << report.Report.wAccelZ << L" \tZg: " << report.Report.wGyroZ << L"\tmax: " << maxZ << L"  min: " << minZ << L"  maxG: " << maxZg << L"  minG: " << minZg << L"       \r\n"; // roll
#else
    repositionConsoleCursor(2, 0);
    std::cout << "X: " << report.Report.wAccelX << "     \tXg: " << report.Report.wGyroX << "   pitch    \n";
    std::cout << "Y: " << report.Report.wAccelY << "     \tYg: " << report.Report.wGyroY << "   yaw      \n";
    std::cout << "Z: " << report.Report.wAccelZ << "     \tZg: " << report.Report.wGyroZ << "   roll     \n";
    repositionConsoleCursor(-5, 0);
#endif
}
#endif
