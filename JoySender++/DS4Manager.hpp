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

#include "HidManager.h"
#include "NxProManager.hpp"

WORD HID_CONTROLLER_TYPE = 0x00;
constexpr WORD DS4Controller_TYPE = 0x0000;
constexpr WORD NxProController_TYPE = 0x0001;

#define DS4_REPORT_NETWORK_DATA_SIZE 61
#define DS4_BT_OUTPUT_REPORT_SIZE 78
#define DS4_USB_OUTPUT_REPORT_SIZE 32
constexpr byte DS4_VIA_USB = 1;
constexpr byte DS4_VIA_BT = 3;


// for testing
#if 0
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
void PrintErrorMessage(DWORD errorCode)
{
    LPWSTR messageBuffer = nullptr;

    DWORD result = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPWSTR>(&messageBuffer),
        0,
        nullptr
    );

    if (result != 0 && messageBuffer != nullptr)
    {
        std::wcout << "Error: " << errorCode << " - " << messageBuffer << std::endl;

        // Free the message buffer
        LocalFree(messageBuffer);
    }
    else
    {
        std::cerr << "Failed to retrieve error message for code: " << errorCode << std::endl;
    }
}
void outputLastError() {
    DWORD errorNum = GetLastError();
    PrintErrorMessage(errorNum);
}
#endif


template<int N> struct BTAudio {
    uint16_t FrameNumber;
    uint8_t AudioTarget; // 0x02 speaker?, 0x24 headset?, 0x03 mic?
    uint8_t SBCData[N - 3];

};

struct USBSetStateData {
    uint8_t EnableRumbleUpdate : 1;
    uint8_t EnableLedUpdate : 1;
    uint8_t EnableLedBlink : 1;
    uint8_t EnableExtWrite : 1;
    uint8_t EnableVolumeLeftUpdate : 1;
    uint8_t EnableVolumeRightUpdate : 1;
    uint8_t EnableVolumeMicUpdate : 1;
    uint8_t EnableVolumeSpeakerUpdate : 1;
    uint8_t UNK_RESET1 : 1; // unknown reset, both set high by Remote Play
    uint8_t UNK_RESET2 : 1; // unknown reset, both set high by Remote Play
    uint8_t UNK1 : 1;
    uint8_t UNK2 : 1;
    uint8_t UNK3 : 1;
    uint8_t UNKPad : 3;
    uint8_t Empty1;
    uint8_t RumbleRight; // weak
    uint8_t RumbleLeft; // strong
    uint8_t LedRed;
    uint8_t LedGreen;
    uint8_t LedBlue;
    uint8_t LedFlashOnPeriod;
    uint8_t LedFlashOffPeriod;
    uint8_t ExtDataSend[8]; // sent to I2C EXT port, stored in 8x8 byte block
    uint8_t VolumeLeft; // 0x00 - 0x4F inclusive
    uint8_t VolumeRight; // 0x00 - 0x4F inclusive
    uint8_t VolumeMic; // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
    uint8_t VolumeSpeaker; // 0x00 - 0x4F
    uint8_t UNK_AUDIO1 : 7; // clamped to 1-64 inclusive, appears to be set to 5 for audio
    uint8_t UNK_AUDIO2 : 1; // unknown, appears to be set to 1 for audio
    uint8_t Pad[8];

    // Constructor to initialize the fields
    USBSetStateData()
    {
        // Initialize the fields with default values
        EnableRumbleUpdate = 1;
        EnableLedUpdate = 1;
        EnableLedBlink = 1;
        EnableExtWrite = 0;
        EnableVolumeLeftUpdate = 0;
        EnableVolumeRightUpdate = 0;
        EnableVolumeMicUpdate = 0;
        EnableVolumeSpeakerUpdate = 0;
        UNK_RESET1 = 0; // unknown reset, both set high by Remote Play
        UNK_RESET2 = 0; // unknown reset, both set high by Remote Play
        UNK1 = 1;
        UNK2 = 0;
        UNK3 = 0;
        UNKPad = 0;
        Empty1 = 0;
        RumbleRight = 0; // weak
        RumbleLeft = 0; // strong
        LedRed = 150;
        LedGreen = 0;
        LedBlue = 0;
        LedFlashOnPeriod = 0;
        LedFlashOffPeriod = 0;
        ExtDataSend[7] = {}; // sent to I2C EXT port, stored in 8x8 byte block
        VolumeLeft = 0; // 0x00 - 0x4F inclusive
        VolumeRight = 0; // 0x00 - 0x4F inclusive
        VolumeMic = 0; // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
        VolumeSpeaker = 0; // 0x00 - 0x4F
        UNK_AUDIO1 = 0; // clamped to 1-64 inclusive, appears to be set to 5 for audio
        UNK_AUDIO2 = 0; // unknown, appears to be set to 1 for audio
        Pad[7] = {};
    }
};

struct BTSetStateData {
    uint8_t EnableRumbleUpdate : 1;
    uint8_t EnableLedUpdate : 1;
    uint8_t EnableLedBlink : 1;
    uint8_t EnableExtWrite : 1;
    uint8_t EnableVolumeLeftUpdate : 1;
    uint8_t EnableVolumeRightUpdate : 1;
    uint8_t EnableVolumeMicUpdate : 1;
    uint8_t EnableVolumeSpeakerUpdate : 1;
    uint8_t UNK_RESET1 : 1; // unknown reset, both set high by Remote Play
    uint8_t UNK_RESET2 : 1; // unknown reset, both set high by Remote Play
    uint8_t UNK1 : 1;
    uint8_t UNK2 : 1;
    uint8_t UNK3 : 1;
    uint8_t UNKPad : 3;
    uint8_t Empty1;
    uint8_t RumbleRight; // weak
    uint8_t RumbleLeft; // strong
    uint8_t LedRed;
    uint8_t LedGreen;
    uint8_t LedBlue;
    uint8_t LedFlashOnPeriod;
    uint8_t LedFlashOffPeriod;
    uint8_t ExtDataSend[8]; // sent to I2C EXT port, stored in 8x8 byte block
    uint8_t VolumeLeft; // 0x00 - 0x4F inclusive
    uint8_t VolumeRight; // 0x00 - 0x4F inclusive
    uint8_t VolumeMic; // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
    uint8_t VolumeSpeaker; // 0x00 - 0x4F
    uint8_t UNK_AUDIO1 : 7; // clamped to 1-64 inclusive, appears to be set to 5 for audio
    uint8_t UNK_AUDIO2 : 1; // unknown, appears to be set to 1 for audio
    uint8_t Pad[52];

    // Constructor to initialize the fields
    BTSetStateData()
    {
        // Initialize the fields with default values
        EnableRumbleUpdate = 1;
        EnableLedUpdate = 1;
        EnableLedBlink = 0;
        EnableExtWrite = 0;
        EnableVolumeLeftUpdate = 1;
        EnableVolumeRightUpdate = 1;
        EnableVolumeMicUpdate = 1;
        EnableVolumeSpeakerUpdate = 1;
        UNK_RESET1 = 0; // unknown reset, both set high by Remote Play
        UNK_RESET2 = 0; // unknown reset, both set high by Remote Play
        UNK1 = 1;
        UNK2 = 0;
        UNK3 = 0;
        UNKPad = 0;
        Empty1 = 0;
        RumbleRight = 0; // weak
        RumbleLeft = 0; // strong
        LedRed = 105;
        LedGreen = 4;
        LedBlue = 32;
        LedFlashOnPeriod = 0;
        LedFlashOffPeriod = 0;
        ExtDataSend[7] = {}; // sent to I2C EXT port, stored in 8x8 byte block
        VolumeLeft = 0; // 0x00 - 0x4F inclusive
        VolumeRight = 0; // 0x00 - 0x4F inclusive
        VolumeMic = 0; // 0x00, 0x01 - 0x40 inclusive (0x00 is special behavior)
        VolumeSpeaker = 0; // 0x00 - 0x4F
        UNK_AUDIO1 = 0; // clamped to 1-64 inclusive, appears to be set to 5 for audio
        UNK_AUDIO2 = 0; // unknown, appears to be set to 1 for audio
        Pad[51] = {};
    }
};

template<int N> struct BTSetStateDataAndAudio {
    BTSetStateData State;
    BTAudio<N - 75> Audio;
};

template<int N> struct BTCRC {
    uint8_t Buff[N - 4];
    uint32_t CRC;
};

struct ReportOut05 {
    const uint8_t ReportID = 0x05;
    USBSetStateData State;

    ReportOut05() {
        State = USBSetStateData();
    }
};

struct ReportOut11 {
    union {
        BTCRC<78> CRC;
        struct {
            uint8_t ReportID; // 0x11
            uint8_t PollingRate : 6;
            uint8_t EnableCRC : 1;
            uint8_t EnableHID : 1;
            uint8_t EnableMic : 3;
            uint8_t UnkA4 : 1;
            uint8_t UnkB1 : 1;
            uint8_t UnkB2 : 1;
            uint8_t UnkB3 : 1;
            uint8_t EnableAudio : 1;
            union {
                BTSetStateData State;
                BTAudio<75> Audio;
            };
        } Data;
    };

    // Default constructor
    ReportOut11()
    {
        Data.ReportID = 0x11;
        Data.PollingRate = 0;
        Data.EnableCRC = 1;
        Data.EnableHID = 1;
        Data.EnableMic = 0;
        Data.UnkA4 = 0;
        Data.UnkB1 = 0;
        Data.UnkB2 = 1;
        Data.UnkB3 = 0;
        Data.EnableAudio = 0;
        Data.State = BTSetStateData();
    }
};

// Create a buffer to store reports in
const DWORD ds4_InBuffSize = 547; // 547 is smallest value that will receive DS4 report using ReadFileInputReport() via BT, USB can be as low as 64?
BYTE ds4_InReportBuf[ds4_InBuffSize] = { 0 };

// Create an output buffer for sending ReportOut11/ReportOut05 structures
BYTE ds4_OutReportBuf[sizeof(ReportOut11)];

// Holds the position of the relevent input data in HID report depending on USB/BT connection
size_t ds4DataOffset = 0;

HidDeviceManager DS4manager;
ReportOut11 ds4OutReport11;
ReportOut05 ds4OutReport05;
auto ds4StateBT = &ds4OutReport11.Data.State;
auto ds4StateUSB = &ds4OutReport05.State;

int ConsoleSelectDS4Dialog(std::vector<HidDeviceInfo>& devList) {
   int numJoysticks = devList.size();
   int selectedJoystickIndex =-1;

   while (selectedJoystickIndex < 0 && !APP_KILLED) {
       // Print the available joysticks
       std::cout << "Connected DS4 Controllers:" << std::endl;
       for (size_t i = 0; i < numJoysticks; i++)
       {
           std::wcout << 1 + i << ": " << devList[i].manufacturer << " " << devList[i].product << ": ";
           if (devList[i].serial.empty()) {
               std::wcout << " Wired" << std::endl;
           }
           else {
               std::wcout << " Wireless" << std::endl;
           }
       }

       // Prompt the user to select a joystick
       std::cout << "Select a joystick (enter the index): ";
       std::cin >> selectedJoystickIndex;
       --selectedJoystickIndex;
       while ((getchar()) != '\n');    // Clear keyboard buffer of enter press

       // Check if the selected index is valid
       if (selectedJoystickIndex < 0 || selectedJoystickIndex >= numJoysticks)
       {
           //std::cout << "Invalid index." << std::endl << std::endl;
           return -1;
       }
   }

   return selectedJoystickIndex;
}

bool ConnectToDS4Controller() {
    std::vector<HidDeviceInfo> devList;
    HidDeviceInfo* selectedDev = nullptr;

    devList = DS4manager.scanDevices(
        ANY,                // vendor id  // Sony:: 1356
        ANY,                // product id // DS4 v1:: 2508
        ANY,                // serial
        ANY,                // manufacturer
        L"Wireless Controller",                // product string // L"Wireless Controller"
        ANY,                // release
        ANY,                // usage page
        ANY                 // usage
    );

    if (devList.size() == 0) {
        std::cout << "\t No Connected DS4 Controllers \n Connect a controller and press a key..." << std::endl;
        _getch();
        return ConnectToDS4Controller();
    }
    if (devList.size() > 1) {
        // User selects from connected DS4 devices
        int idx = ConsoleSelectDS4Dialog(devList);
        if (idx == -1) {
            clearConsoleScreen();
            std::cout << "Invalid index." << std::endl;
            return ConnectToDS4Controller();
        }
        selectedDev = &devList[idx];
    }
    else{
        selectedDev = &devList[0];
    }
    
    if (selectedDev != nullptr) {
        if (DS4manager.OpenHidDevice(selectedDev)) {
            std::wcout << "Connected to : " << selectedDev->manufacturer << " " << selectedDev->product << std::endl;
            return true;
        }
    }

    return false;
}

bool ActivateDS4ExtendedReports() {
    if (ds4DataOffset == DS4_VIA_USB) {
        /// READ USB CALIBRATE REPORT 0x02 // size is 37 bytes
        if (DS4manager.ReadFeatureReport(0x02, ds4_InReportBuf, 37))
        {
            return 1;
        }
    }
    else { //(ds4DataOffset == DS4_VIA_BT) {
        /// READ BT CALIBRATE REPORT 0x05 // size is 41 bytes
        if (DS4manager.ReadFeatureReport(0x05, ds4_InReportBuf, 41))
        {
            return 1;
        }
    }
    return 0;
}

void InitDS4FeedbackBuffer() {
    switch (ds4DataOffset) {
    case DS4_VIA_BT: {
        // take first three bytes of report and place them in the buffer
        memcpy(ds4_OutReportBuf, &ds4OutReport11, 3);
        // to prevent a padding byte in the structure manually place 
        // the State of the report at buffer index 3
        memcpy(ds4_OutReportBuf + 3, ds4StateBT, sizeof(BTSetStateData));
    }
                   break;
    case DS4_VIA_USB: {
        // take first byte of report and place in the buffer
        memcpy(ds4_OutReportBuf, &ds4OutReport05, sizeof(ds4OutReport05));

        // to prevent a padding byte in the structure manually place 
        // the State of the report at buffer index 1
       /// memcpy(ds4_OutReportBuf + 1, ds4StateUSB, sizeof(USBSetStateData));

    }
                   break;
    }
}

void SetDS4LightBar(UINT8 r, UINT8 g, UINT8 b) {
    if (ds4DataOffset == DS4_VIA_BT) {
        ds4StateBT->LedRed = r;
        ds4StateBT->LedGreen = g;
        ds4StateBT->LedBlue = b;
    }
    else if(ds4DataOffset == DS4_VIA_USB) {
        ds4StateUSB->LedRed = r;
        ds4StateUSB->LedGreen = g;
        ds4StateUSB->LedBlue = b;
    }
}

void SetDS4RumbleValue(UINT8 r, UINT8 l) {
    if (ds4DataOffset == DS4_VIA_BT) {
        ds4StateBT->RumbleLeft = l;
        ds4StateBT->RumbleRight = r;
    }
    else if (ds4DataOffset == DS4_VIA_USB) {
        ds4StateUSB->RumbleLeft = l;
        ds4StateUSB->RumbleRight = r;
    }
}

bool SendDS4Update() {    
    if (ds4DataOffset == DS4_VIA_BT) {
        // Update the state of the report at buffer index 3
        memcpy(ds4_OutReportBuf + ds4DataOffset, ds4StateBT, sizeof(BTSetStateData));
        if (DS4manager.WriteOutputReport(ds4_OutReportBuf, DS4_BT_OUTPUT_REPORT_SIZE)) {
            return 1;
        }
    }
    else if (ds4DataOffset == DS4_VIA_USB) {
        // Update the state of the report at buffer index 1
        memcpy(ds4_OutReportBuf, &ds4OutReport05, sizeof(ds4OutReport05));
        if (DS4manager.WriteFileOutputReport(ds4_OutReportBuf, DS4_USB_OUTPUT_REPORT_SIZE)) {
            return 1;
        }
    }
    return 0;
}

bool GetDS4Report() {
    switch (HID_CONTROLLER_TYPE) {
    case(DS4Controller_TYPE):
        if (DS4manager.ReadFileInputReport(0x11, ds4_InReportBuf, ds4_InBuffSize)) {
            //uint8_t enableHID = (ds4_InReportBuf[1] >> 7) & 0x01; // report contains controller data
            if ((ds4_InReportBuf[1] >> 7) & 0x01)
            {
                return 1;
            }
            if (ds4DataOffset == DS4_VIA_USB)
                return 1;
        }
        break;

    case(NxProController_TYPE):
        if (NxProController::convertToDS4Report(&DS4manager, ds4_InReportBuf)) {
            return 1;
        }
        break;

    }
    return 0;
}
