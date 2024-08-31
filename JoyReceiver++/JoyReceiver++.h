#pragma once
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>
#include "utilities.hpp"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

std::string feedbackData; // For sending rumble data back to joySender

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
    repositionConsoleCursor(2);
#endif
    std::string& feedbackData = *reinterpret_cast<std::string*>(UserData);
    // Set the UserData with the desired value
    feedbackData = static_cast<char>(LargeMotor);
    feedbackData += static_cast<char>(SmallMotor);
}

VOID CALLBACK ds4_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target, UCHAR LargeMotor,
    UCHAR SmallMotor, DS4_LIGHTBAR_COLOR LightbarColor, LPVOID UserData){
#if 0
    repositionConsoleCursor(-2, 3);
    std::cout << "Rumble Data: ";
    std::cout << "L:" << (int)LargeMotor << "  ";
    repositionConsoleCursor(0, 22);
    std::cout << "R:" << (int)SmallMotor << "  ";
    repositionConsoleCursor(0, 28);
    std::cout << "Color: " << (int)LightbarColor.Red << "  ";
    repositionConsoleCursor(0, 38);
    std::cout << " | " << (int)LightbarColor.Green << "  ";
    repositionConsoleCursor(0, 44);
    std::cout << " | " << (int)LightbarColor.Blue << "        ";
    repositionConsoleCursor(2);
#endif
    std::string& feedbackData = *reinterpret_cast<std::string*>(UserData);
    // Set the UserData with the desired value
    feedbackData = static_cast<char>(LargeMotor);
    feedbackData += static_cast<char>(SmallMotor);

    feedbackData += static_cast<char>(LightbarColor.Red);
    feedbackData += static_cast<char>(LightbarColor.Green);
    feedbackData += static_cast<char>(LightbarColor.Blue);
};
