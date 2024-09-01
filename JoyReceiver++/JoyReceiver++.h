#pragma once
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>
#include <thread>
#include <mutex>
#include "utilities.hpp"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

std::mutex mtx;
char feedbackData[5] = { 0 }; // For sending rumble data back to joySender
volatile sig_atomic_t APP_KILLED = 0;
void signalHandler(int signal);


// Define the callback functions for Rumble Support 
VOID CALLBACK xbox_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target, 
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

void displayBytes(BYTE* buffer, DWORD bufferSize) {
    for (DWORD i = 0; i < bufferSize; i++) {
        if (i % 18 == 0) printf("\r\n");
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

std::thread ds4Rumbler;
bool ds4ThreadStop = true;
void ds4RumbleThread(PVIGEM_CLIENT vigemClient, PVIGEM_TARGET gamepad) {
    DS4_OUTPUT_BUFFER buffer;
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    while (!APP_KILLED && !ds4ThreadStop) {
        
        auto vigemErr = vigem_target_ds4_await_output_report_timeout(vigemClient, gamepad, 350, &buffer);
        if (!VIGEM_SUCCESS(vigemErr) && vigemErr != VIGEM_ERROR_TIMED_OUT) {
            std::cerr << "DS4 Rumble callback failed with error code: 0x" << std::hex << vigemErr << std::endl;
            lock.lock();
            std::memset(feedbackData, '\0', sizeof(feedbackData));
            lock.unlock();
        }
        else if(vigemErr != VIGEM_ERROR_TIMED_OUT){
#if 0 
            std::cout << "Buffer Data: ";
            displayBytes(buffer.Buffer, 64);
            repositionConsoleCursor(-5, 0);
#endif
            lock.lock();
            std::memcpy(feedbackData, &buffer.Buffer[4], 5 * sizeof(char));
            lock.unlock();
        }
        else {
            lock.lock();
            std::memset(feedbackData, '\0', sizeof(feedbackData));
            lock.unlock();
        }
    }
}
