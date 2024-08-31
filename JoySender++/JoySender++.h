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

#pragma once
#define NOMINMAX

#include <iostream>
#include <conio.h>
#include <csignal>
#include "TCPConnection.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"
#include "utilities.hpp"

#pragma comment(lib, "SDL3.lib")

// GLOBAL VARIABLES
constexpr auto APP_NAME = "NetJoy";
constexpr auto OLDMAP_WARNING_MSG = "! WARNING OLD MAP FILE DETECTED, RE-MAPPING INPUTS RECOMMENDED !";

std::string g_outputText;

volatile sig_atomic_t APP_KILLED = 0;

// Function safely returns environment variables
std::string g_getenv(const char* variableName);
// Some Console Output, ANSI helper functions
void enableANSI();
void hideConsoleCursor();
void showConsoleCursor();
void clearConsoleScreen();
void repositionConsoleCursor(int lines = 0, int offset = 0);
void clearConsoleLine();
// For FPS and Latency Output
void overwriteFPS(const std::string& text);
void overwriteLatency(const std::string& text);
// Function to display g_outputText to console
void displayOutputText();
// Function to detect if keyboard key is pressed
bool getKeyState(int KEYCODE);
// Function to convert strings to wide strings
std::wstring g_toWide(std::string& str);
// Function that will prompt for and verify an ip address 
std::string getHostAddress();
// Function that takes string representing a float and fixes it's decimal places to numDigits
std::string formatDecimalString(const std::string& str, UINT8 numDigits);
// Function captures all input from the keyboard to clear buffer
void swallowInput();
// Function waits for no keyboard presses to be detected before returning
void wait_for_no_keyboard_input();
// Function determines if app if the active window
bool IsAppActiveWindow();
// Function determines if rumble values need to be updated
bool updateRumble(const char motor, BYTE val);
// Main Loop Function 
int joySender(Arguments& args);


// Function to display g_outputText to console
void displayOutputText() {
    clearConsoleScreen();
    std::cout << g_outputText;
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