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
bool OLDMAP_FLAG = 0;
std::string g_outputText;
void displayOutputText();

#include "utilities.hpp"
#include "GamepadMapping.hpp"
#include "DS4Manager.hpp"

#ifdef NetJoyTUI
extern void uiOpenOrCreateMapping(SDLJoystickData&);
#endif

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

// universal (++/tUI) opmode(xbox/ds4) init function
void JOYSENDER_OPMODE_INIT(SDLJoystickData& activeGamepad, Arguments& args, int& allGood) {
    
    if (args.mode == 2) {

        // Get a report from the device to determine what type of connection it has
        Sleep(5); // a fresh report should be generated every 4ms from the ds4 device
        allGood = GetDS4Report();
        if (allGood < 1) {
            Sleep(5);
            allGood = GetDS4Report();
        }
        if (allGood < 1) {
            std::cout << "No good reports \r\n";
        }
        /* ^^^^^^^^^^^^^^^^^^^^^*/
        /* GetDS4Report can return 0 if the report does not contain controller data.
        this is not checked for here and is most likely the cause of the issue below.
        TODO: investigate and fix in next release                                 */
        /*^^^^^^^^^^^^^^^^^^^^^^*/

        // first byte is used to determine where stick input starts
        ds4DataOffset = ds4_InReportBuf[0] == 0x11 ? DS4_VIA_BT : DS4_VIA_USB;

        int attempts = 0;   // DS4 fails to properly initialize when connecting to pc (after power up) via BT so lets hack in multiple attempts
        while (attempts < 2) {
            attempts++;

            Sleep(5); // lets slow things down
            bool extReport = ActivateDS4ExtendedReports();

            // Set up feedback buffer with correct headers for connection mode
            InitDS4FeedbackBuffer();

            // Set new LightBar color with update to confirm rumble/lightbar support
            switch (ds4DataOffset) {
            case(DS4_VIA_BT):
                SetDS4LightBar(105, 4, 32); // hot pink
                break;
            case(DS4_VIA_USB):
                SetDS4LightBar(180, 188, 5); // citrus yellow-green
            }

            Sleep(5); // update fails if controller is bombarded with read/writes, so take a rest bud
            allGood = SendDS4Update();

#ifndef NetJoyTUI
            g_outputText = "DS4 ";
            if (extReport) g_outputText += "Full Motion ";
            g_outputText += "Mode Activated : ";
            if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
            else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
            if (allGood) g_outputText += " Rumble On | ";
            g_outputText += "\r\n";
#endif

            // Rumble the Controller
            if (allGood) {
                // jiggle it
                SetDS4RumbleValue(12, 200);
                SendDS4Update();
                Sleep(110);
                SetDS4RumbleValue(165, 12);
                SendDS4Update();
                // stop the rumble
                SetDS4RumbleValue(0, 0);
                Sleep(130);
                SendDS4Update();
                break; // break out of attempt loop
            }
            // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
        // Problem is probably related to not getting the correct report and assigning ds4DataOffset = DS4_VIA_USB
        // taking the lazy route and just set ds4DataOffset = DS4_VIA_BT this works for me 100% of the time and hasn't led to problems yet ...
            Sleep(10);
            if (ds4DataOffset == DS4_VIA_USB)
                ds4DataOffset = DS4_VIA_BT;
            else
                ds4DataOffset = DS4_VIA_USB;
        }
    }
    else {

        BuildJoystickInputData(activeGamepad);
        // Look for an existing map for selected device
#ifdef NetJoyTUI
        uiOpenOrCreateMapping(activeGamepad);
#else
        g_outputText = "XBOX Mode Activated\r\n";
        OpenOrCreateMapping(activeGamepad);
#endif

        if (g_outputText.find(OLDMAP_WARNING_MSG) != std::string::npos) {
            OLDMAP_FLAG = true;
        }
    }

}