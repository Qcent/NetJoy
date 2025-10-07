/*

Copyright (c) 2025 Dave Quinn <qcent@yahoo.com>

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

#define APP_VERSION_NUM     L"2.0.2.0"

#include <iostream>
#include <conio.h>
#include <csignal>
#include "NetworkCommunication.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"

#pragma comment(lib, "SDL3.lib")

// GLOBAL VARIABLES
volatile sig_atomic_t APP_KILLED = 0;
constexpr auto APP_NAME = "NetJoy";
constexpr auto OLDMAP_WARNING_MSG = "! WARNING OLD MAP FILE DETECTED, RE-MAPPING INPUTS RECOMMENDED !";
bool OLDMAP_FLAG = 0;
unsigned char RESTART_FLAG = 0;
unsigned char MAPPING_FLAG = 0;
std::string g_outputText;
void displayOutputText();

#include "utilities.hpp"
#include "GamepadMapping.hpp"
#include "DS4Manager.hpp"
#include "NxProManager.hpp"

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
        std::cout << "Please enter the host IP address (" << (UDP_COMMUNICATION ? "UDP" : "TCP") << ") :"; // (blank = localhost) : ";
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

// returns a list of DS4 controllers by HidDeviceInfo info
std::vector<HidDeviceInfo> getDS4ControllersList() {
    std::vector<HidDeviceInfo> devList, devListTwo;

    // Search for all compatable devices 1 at a time
      // DS4 controllers
    devList = DS4manager.scanDevices(
        ANY,                // vendor id  // Sony:: 1356
        ANY,                // product id // DS4 v1:: 2508
        ANY,                // serial
        ANY,                // manufacturer ## L"Nintendo"
        L"Wireless Controller",                // product string // L"Wireless Controller" ## L"Wireless Gamepad"
        ANY,                // release
        ANY,                // usage page
        ANY                 // usage
    );
    // Nx Pro controllers
    devListTwo = DS4manager.scanDevices(
        ANY,                // vendor id  // Sony:: 1356
        ANY,                // product id // DS4 v1:: 2508
        ANY,                // serial
        ANY,                // manufacturer ## L"Nintendo"
        L"Wireless Gamepad",                // product string // L"Wireless Controller" ## L"Wireless Gamepad"
        ANY,                // release
        ANY,                // usage page
        ANY                 // usage
    );

    // Consolidate
    if (!devListTwo.empty()) devList.insert(devList.end(), devListTwo.begin(), devListTwo.end());

#ifdef NetJoyTUI
    extern void setErrorMsg(const wchar_t* text, size_t length);
    if (!devList.size()) {
        setErrorMsg(L" No DS4 Devices Connected! ", 28);
    }
#endif
    
    return devList;
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
        switch (HID_CONTROLLER_TYPE) {
        case(DS4Controller_TYPE):
            if (update) {
                SetDS4RumbleValue(byte(buffer[0]), byte(buffer[1]));
            }
            update += updateDS4Lightbar(&buffer[2]);
            if (update) {
                SendDS4Update();
            }
            break;

        case(NxProController_TYPE):
            if (update) {
                NxRumble::instance().setFrame({ 100.0f, (static_cast<float>(buffer[0]) / 255.0f), 
                                                     75.0f, (static_cast<float>(buffer[1]) / 255.0f), 55 });
            }
            break;
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
        if (HID_CONTROLLER_TYPE == NxProController_TYPE) {

            NxProController NxPro(&DS4manager);
            // auto reportOpt = NxPro.GetReport(); // for checking report values before subcommands

            NxPro.sendSubcommand(0x1, rumbleCommand, enable);
            NxPro.sendSubcommand(0x1, imuDataCommand, enable);
            NxPro.sendSubcommand(0x1, ledCommand, led);

            NxPro.loadCalibration();

            //ImuCal = NxPro.info.calibCoeff;

            auto NxReport = NxPro.GetReport();

            bool extReport = NxReport.value()[0] != 63; // report id when BT and no IMU: 63 (flawed, but successful)
            NxPro.parseConnection(NxReport); // cannot parse correctly until extended reports are enabled ie. IMU data

         
#if 0
            for (int i = 0; i < 6; i++) {
                NxPro.sendSimpleRumble(60, 00);
                Sleep(18);
                NxPro.sendSimpleRumble(00, 30);
                Sleep(20);
                NxPro.sendSimpleRumble(70, 00);
                Sleep(24);
                NxPro.sendSimpleRumble(0, 60);
                Sleep(26);
            }
            NxPro.sendSimpleRumble(0, 0);
#endif

            auto& rumbler = NxRumble::instance(&DS4manager, !NxPro.info.usbPowered, true);
            rumbler.start();
            
            rumbler.setFrame({ 250.0f, 1.0f, 0.0f, 0.7f, 70 });
            Sleep(190);
            rumbler.setFrame({ 0.0f, 1.0f, 240.0f, 0.8f, 50 });
            Sleep(80);
            
#ifndef NetJoyTUI
            g_outputText = "Nintendo Pro -> DS4 ";
            if (extReport) g_outputText += "Full Motion ";
            g_outputText += "Mode Activated : ";
            g_outputText += (NxPro.info.usbPowered ? "| USB |" : "| Wireless |");
            if (extReport) g_outputText += " Rumble On | "; // no good check for rumble implemented, but never fails
            g_outputText += "\r\n";
#endif


        }
        else {

            ds4DataOffset = DS4manager.devInfo.serial.empty() ?
                DS4_VIA_USB : 
                DS4_VIA_BT ;

            int attempts = 0;   // DS4 can fail to properly initialize when connecting to pc (after power up) via BT so lets hack in multiple attempts
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
            // Problem is fixed but leaving this in as a fallback
                Sleep(10);
                if (ds4DataOffset == DS4_VIA_USB)
                    ds4DataOffset = DS4_VIA_BT;
                else
                    ds4DataOffset = DS4_VIA_USB;
            }
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

void JOYSENDER_SET_DS4_CONTROLLER_FLAG(HidDeviceInfo& dev) {
// Check and set flag if contoller is nintendo gamepad ** or others
    if (dev.manufacturer == L"Nintendo") {
        HID_CONTROLLER_TYPE = NxProController_TYPE;
    }
    else { HID_CONTROLLER_TYPE = DS4Controller_TYPE;; }
}

int JOYSENDER_CONSOLE_SELECT_DS4_DIALOG() {
    std::vector<HidDeviceInfo> devList = getDS4ControllersList();
    HidDeviceInfo* selectedDev = nullptr;

    if (devList.size() == 0) {
        std::cout << "\t No Connected DS4 Controllers \n Connect a controller and press a key..." << std::endl;
        _getch();
        return JOYSENDER_CONSOLE_SELECT_DS4_DIALOG();
    }
    if (devList.size() > 0) {
        // User selects from connected DS4 devices
        int idx = ConsoleSelectDS4Dialog(devList);
        if (idx == -1) {
            clearConsoleScreen();
            std::cout << "Invalid index." << std::endl;
            return JOYSENDER_CONSOLE_SELECT_DS4_DIALOG();
        }
        selectedDev = &devList[idx];
    }
   // else {// no more auto select (stops death loops on failed connections)
   //     selectedDev = &devList[0];
   // }

    if (selectedDev != nullptr) {
        if (DS4manager.OpenHidDevice(selectedDev)) {
            // Set controller type flag
            JOYSENDER_SET_DS4_CONTROLLER_FLAG(*selectedDev);

            std::wcout << "Connected to : " << selectedDev->manufacturer << " " << selectedDev->product << std::endl;
            return true;
        }
    }

    return false;
}



#define JOYSENDER_PROCESS_SIGNAL_PACKET() \
{ \
    UDPConnection::SIGPacket* pkt = (UDPConnection::SIGPacket*)buffer; \
    if(pkt->type == UDPConnection::PACKET_HANGUP){ \
        g_outputText += "<< Host Disconnected >> \r\n"; \
        displayOutputText(); \
        inConnection = false; \
    } \
    /* Do not process as feedback data */ \
    continue; \
}


void JOYSENDER_FEEDBACK_THREAD(NetworkConnection& client, char* buffer, size_t buffer_size, SDLJoystickData& activeGamepad, Arguments& args, bool& inConnection) {
    int timeouts = 0;
    while (!APP_KILLED && inConnection) {
           
        int allGood = client.receive_data(buffer, buffer_size);

        if (allGood == sizeof(UDPConnection::SIGPacket)) JOYSENDER_PROCESS_SIGNAL_PACKET()

        if (allGood < 1) {           
            if (!inConnection) break;
 //           if (MAPPING_FLAG && UDP_COMMUNICATION) { // no mapping wait in joysender++
 //               Sleep(150);
 //               client.keep_alive();
 //           }
            int er = WSAGetLastError();
            if (er == 10060 && !MAPPING_FLAG) {
                                           
                    if (++timeouts > 3) inConnection = false;
            }
            else /*if (er == 10054)*/ {
                                           
                    inConnection = false;
            }
            if (!inConnection) {
                // Connection was lost or ended
                if (!APP_KILLED) {
                    g_outputText += "<< Connection Lost >> \r\n";
                    displayOutputText();
                }
            }
        }
        else processFeedbackBuffer((byte*)&buffer, activeGamepad, args.mode);
    } 
}