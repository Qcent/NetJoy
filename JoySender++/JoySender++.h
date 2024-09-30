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

#pragma comment(lib, "SDL3.lib")

// GLOBAL VARIABLES
volatile sig_atomic_t APP_KILLED = 0;
constexpr auto APP_NAME = "NetJoy";
constexpr auto OLDMAP_WARNING_MSG = "! WARNING OLD MAP FILE DETECTED, RE-MAPPING INPUTS RECOMMENDED !";
std::string g_outputText;
void displayOutputText();

#include "utilities.hpp"
#include "GamepadMapping.hpp"
#include "DS4Manager.hpp"

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

// Function determines if rumble values need to be updated
bool updateRumble(const char motor, BYTE val) {
    static BYTE L, R = 0;
    switch (motor) {
    case 'L':
    {
        if (L == val) {
            return false;
        }
        L = val;
        return true;
    }

    case 'R':
    {
        if (R == val) {
            return false;
        }
        R = val;
        return true;
    }
    }
    return false;
}

// Takes in a 3 byte buffer containing lightbar values, will set SetDS4LightBar if values are different from current
bool updateDS4Lightbar(const byte* buffer) {
    static UINT8 lastValue[3] = { 0 };
    if (memcmp(buffer, lastValue, sizeof(lastValue)) == 0) {
        return false;
    }
    memcpy(lastValue, buffer, sizeof(lastValue));
    SetDS4LightBar(buffer[0], buffer[1], buffer[2]);
    return true;
}

// Reckons what to do with feedback from JoyReceiver 
void processFeedbackBuffer(const byte* buffer, SDLJoystickData& activeGamepad, int mode) {
    int update = 0;

    if (updateRumble('L', buffer[0]) || updateRumble('R', buffer[1])) {
        ++update;
    }
    if (mode == 2) {
        if (update) {
            SetDS4RumbleValue(byte(buffer[0]), byte(buffer[1]));
        }
        update += updateDS4Lightbar(&buffer[2]);
        if (update) {
            SendDS4Update();
        }
        return;
    }
    if (update) {
        SDLRumble(activeGamepad, buffer[0], buffer[1]);
    }

}