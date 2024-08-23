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
#pragma once

#include <iostream>
#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <vector>
#include <algorithm>

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

#define ANY  0 /* for default hid device scanning parameter value */
// Define the GUID for HID class interface
DEFINE_GUID(GUID_DEVINTERFACE_HID, 0x4D1E55B2, 0xF16F, 0x11CF, 0x88, 0xCB, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30);

struct HidDeviceInfo {
    std::wstring       path;
    std::wstring      serial;
    std::wstring      manufacturer;
    std::wstring      product;
    unsigned short    vendorId        = 0;
    unsigned short    productId       = 0;
    unsigned short    release         = 0;
    unsigned short    usagePage       = 0;
    unsigned short    usage           = 0;
    int               interfaceNumber = 0;
};

class HidDeviceManager
{
public:
    HANDLE selectedDevice;
    HidDeviceInfo Info;

    HidDeviceManager()
    {
        selectedDevice = nullptr;
    }

    ~HidDeviceManager()
    {
        CloseDevice();
    }

    static void DisplayDevicesInfo(std::vector<HidDeviceInfo>& devList) {
        /* This Function shamelessly lifted from https://github.com/yigityuce/HidApi */
        // Prints HidDeviceInfo properties
        for (size_t i = 0; i < devList.size(); i++)
        {
            std::wcout << i + 1 << ". DEVICE\n"
                << "   Path        : " << devList[i].path.c_str() << std::endl
                << "   Manufacturer: " << devList[i].manufacturer << std::endl
                << "   Product     : " << devList[i].product << std::endl
                << "   Serial      : " << devList[i].serial << std::endl
                << "   Vendor Id   : " << devList[i].vendorId << std::endl
                << "   Product Id  : " << devList[i].productId << std::endl
                << "   Release No  : " << devList[i].release << std::endl
                << "   Usage Page  : " << devList[i].usagePage << std::endl
                << "   Usage       : " << devList[i].usage << std::endl
                << "   Interface   : " << devList[i].interfaceNumber << std::endl
                << "--------------------------------------------------" << std::endl;
        }
    }

    bool OpenHidDevice(HidDeviceInfo* newDevice) {
        selectedDevice = CreateFile(newDevice->path.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (IsDeviceOpen()) {
            Info.path = newDevice->path;
            Info.serial = newDevice->serial;
            Info.manufacturer = newDevice->manufacturer;
            Info.product = newDevice->product;
            Info.vendorId = newDevice->vendorId;
            Info.productId = newDevice->productId;
            Info.release = newDevice->release;
            Info.usagePage = newDevice->usagePage;
            Info.usage = newDevice->usage;
            Info.interfaceNumber = newDevice->interfaceNumber;
            return true;
        }
        return 0;
    }

    bool IsDeviceOpen()
    {
        return (selectedDevice != INVALID_HANDLE_VALUE);
    }

    void CloseDevice()
    {
        if (selectedDevice != INVALID_HANDLE_VALUE)
        {
            CloseHandle(selectedDevice);
            selectedDevice = INVALID_HANDLE_VALUE;
        }
    }

    bool ReadInputReport(DWORD id, BYTE* buffer, DWORD bufferSize)
    {
        if (selectedDevice == INVALID_HANDLE_VALUE)
        {
            std::cerr << "No HID device is currently open." << std::endl;
            return false;
        }
        buffer[0] = id;
        if (!HidD_GetInputReport(selectedDevice, buffer, bufferSize))
        {
            std::cerr << "Failed to read input report: " << id << " from the selected HID device." << std::endl;
            return false;
        }

        return true;
    }

    bool WriteOutputReport(const BYTE* buffer, DWORD bufferSize)
    {
        if (selectedDevice == INVALID_HANDLE_VALUE)
        {
            std::cerr << "No HID device is currently open." << std::endl;
            return false;
        }

        if (!HidD_SetOutputReport(selectedDevice, const_cast<PVOID>(static_cast<const void*>(buffer)), bufferSize))
        {
            std::cerr << "Failed to write output report to the selected HID device." << std::endl;
            return false;
        }

        return true;
    }

    bool ReadFeatureReport(DWORD id, BYTE* buffer, DWORD bufferSize)
    {
        // Ensure the device handle is valid
        if (selectedDevice == INVALID_HANDLE_VALUE)
        {
            std::cerr << "Invalid device handle." << std::endl;
            return false;
        }

        // Set the report ID as required (if applicable)
        buffer[0] = id; // Report ID for Feature Input Report (example)

        // Read the Feature report from the device
        if (!HidD_GetFeature(selectedDevice, buffer, bufferSize))
        {
            DWORD error = GetLastError();
            std::cerr << "Failed to read Feature report from the HID device. Error: " << error << std::endl;
            return false;
        }

        return true;
    }

    bool ReadFileInputReport(DWORD id, BYTE* buffer, DWORD bufferSize)
    {
        if (selectedDevice == INVALID_HANDLE_VALUE)
        {
            std::cerr << "No HID device is currently open." << std::endl;
            return false;
        }

        // Create an OVERLAPPED structure for asynchronous I/O
        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (overlapped.hEvent == NULL)
        {
            std::cerr << "Failed to create event for overlapped I/O." << std::endl;
            return false;
        }

        // Set the report ID in the first byte of the buffer
        buffer[0] = static_cast<BYTE>(id);

        // Read the input report using ReadFile with overlapped I/O
        if (!ReadFile(selectedDevice, buffer, bufferSize, NULL, &overlapped))
        {
            DWORD errorCode = GetLastError();

            // If the operation is pending, wait for it to complete
            if (errorCode == ERROR_IO_PENDING)
            {
                if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
                {
                    // Get the result of the overlapped operation
                    DWORD bytesRead = 0;
                    if (GetOverlappedResult(selectedDevice, &overlapped, &bytesRead, FALSE))
                    {
                        // Read operation completed successfully
                        return true;
                    }
                    else
                    {
                        // Read operation failed
                        std::cerr << "Failed to read input report: " << id << " from the selected HID device." << std::endl;
                    }
                }
                else
                {
                    // Wait for single object failed
                    std::cerr << "Failed to wait for overlapped I/O operation to complete." << std::endl;
                }
            }
            else
            {
                // Read operation failed immediately
                std::cerr << "Failed to read input report: " << id << " from the selected HID device." << std::endl;
            }

            // Close the event handle
            CloseHandle(overlapped.hEvent);

            return false;
        }

        // Close the event handle
        CloseHandle(overlapped.hEvent);

        return true;
    }

    bool WriteFileOutputReport(const BYTE* buffer, DWORD bufferSize) {

        if (selectedDevice == INVALID_HANDLE_VALUE)
        {
            std::cerr << "No HID device is currently open." << std::endl;
            return false;
        }

        // Create an OVERLAPPED structure for asynchronous I/O
        OVERLAPPED overlapped = { 0 };
        overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (overlapped.hEvent == NULL)
        {
            std::cerr << "Failed to create event for overlapped I/O." << std::endl;
            return false;
        }

        // Write the output report using WriteFile with overlapped I/O
        if (!WriteFile(selectedDevice, buffer, bufferSize, NULL, &overlapped))
        {
            DWORD errorCode = GetLastError();

            // If the operation is pending, wait for it to complete
            if (errorCode == ERROR_IO_PENDING)
            {
                if (WaitForSingleObject(overlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
                {
                    // Get the result of the overlapped operation
                    DWORD bytesWritten = 0;
                    if (GetOverlappedResult(selectedDevice, &overlapped, &bytesWritten, FALSE))
                    {
                        // Write operation completed successfully
                        //delete[] outputBuffer;
                        return true;
                    }
                    else
                    {
                        // Write operation failed
                        std::cerr << "Failed to write output report to the selected HID device." << std::endl;
                        //PrintErrorMessage(GetLastError());
                    }
                }
                else
                {
                    // Wait for single object failed
                    std::cerr << "Failed to wait for overlapped I/O operation to complete." << std::endl;
                }
            }
            else
            {
                // Write operation failed immediately
                std::cerr << "Failed to write output report to the selected HID device." << std::endl;
                //PrintErrorMessage(errorCode);
            }

            // Close the event handle and clean up allocated memory
            CloseHandle(overlapped.hEvent);
            //delete[] outputBuffer;

            return false;
        }

        // Close the event handle and clean up allocated memory
        CloseHandle(overlapped.hEvent);
        //delete[] outputBuffer;

        return true;
    }

    bool Flush() {
        return HidD_FlushQueue(selectedDevice);
    }

    std::vector<HidDeviceInfo> scanDevices(unsigned short _vendorId,
        unsigned short _productId,
        const  wchar_t* _serial,
        const wchar_t* _manufacturer,
        const wchar_t* _product,
        unsigned short _release,
        unsigned short _usagePage,
        unsigned short _usage)
    {
        /* This Function shamelessly lifted from https://github.com/yigityuce/HidApi */

        std::vector<HidDeviceInfo> devices;

        BOOL                                res;
        int                                 deviceIndex = 0;
        SP_DEVINFO_DATA                     devinfoData;
        SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
        SP_DEVICE_INTERFACE_DETAIL_DATA_A*  deviceInterfaceDetailData = NULL;
        GUID                                InterfaceClassGuid = { 0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30} };

        memset(&devinfoData, 0x0, sizeof(devinfoData));
        memset(&deviceInterfaceData, 0x0, sizeof(deviceInterfaceData));
        devinfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        HDEVINFO deviceInfoSet = SetupDiGetClassDevsA(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

        while (SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &InterfaceClassGuid, deviceIndex, &deviceInterfaceData))
        {
            HANDLE          writeHandle = INVALID_HANDLE_VALUE;
            DWORD           requiredSize = 0;
            HIDD_ATTRIBUTES attrib;

            res = SetupDiGetDeviceInterfaceDetailA(deviceInfoSet, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

            deviceInterfaceDetailData = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)malloc(requiredSize);
            deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

            res = SetupDiGetDeviceInterfaceDetailA(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, NULL, NULL);

            if (!res)
            {
                free(deviceInterfaceDetailData);
                deviceIndex++;
                continue;
            }

            for (int i = 0; ; i++)
            {
                char driverName[256];

                res = SetupDiEnumDeviceInfo(deviceInfoSet, i, &devinfoData);
                if (!res) break;

                res = SetupDiGetDeviceRegistryPropertyA(deviceInfoSet,
                    &devinfoData,
                    SPDRP_CLASS,
                    NULL,
                    (PBYTE)driverName,
                    sizeof(driverName),
                    NULL);
                if (!res) break;

                if (strcmp(driverName, "HIDClass") == 0)
                {
                    res = SetupDiGetDeviceRegistryPropertyA(deviceInfoSet,
                        &devinfoData,
                        SPDRP_DRIVER,
                        NULL,
                        (PBYTE)driverName,
                        sizeof(driverName),
                        NULL);
                    if (res) break;
                }
            }


            if (!res)
            {
                free(deviceInterfaceDetailData);
                deviceIndex++;
                continue;
            }


            writeHandle = CreateFileA(deviceInterfaceDetailData->DevicePath,
                0,
                (FILE_SHARE_READ | FILE_SHARE_WRITE),
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0);


            if (writeHandle == INVALID_HANDLE_VALUE)
            {
                CloseHandle(writeHandle);
                continue;
            }

            attrib.Size = sizeof(HIDD_ATTRIBUTES);

            PHIDP_PREPARSED_DATA ppData = NULL;
            HIDP_CAPS caps;
            wchar_t   serial[256];
            wchar_t   manufa[256];
            wchar_t   produc[256];
            bool      isPreparsed = false;
            bool      isAttr = false;
            bool      isSerial = false;
            bool      isManufa = false;
            bool      isProduc = false;


            if (HidD_GetPreparsedData(writeHandle, &ppData))
            {
                if (HidP_GetCaps(ppData, &caps) == 0x110000)
                {
                    isPreparsed = true;
                }
                HidD_FreePreparsedData(ppData);
            }

            if (HidD_GetAttributes(writeHandle, &attrib)) { isAttr = true; }
            if (HidD_GetSerialNumberString(writeHandle, serial, sizeof(serial))) { isSerial = true; }
            if (HidD_GetManufacturerString(writeHandle, manufa, sizeof(manufa))) { isManufa = true; }
            if (HidD_GetProductString(writeHandle, produc, sizeof(produc))) { isProduc = true; }


            if (((_vendorId == ANY) or (isAttr and (_vendorId == attrib.VendorID))) and
                ((_productId == ANY) or (isAttr and (_productId == attrib.ProductID))) and
                ((_serial == ANY) or (isSerial and (wcscmp(_serial, serial) == 0))) and
                ((_manufacturer == ANY) or (isManufa and (wcscmp(_manufacturer, manufa) == 0))) and
                ((_product == ANY) or (isProduc and (wcscmp(_product, produc) == 0))) and
                ((_release == ANY) or (isAttr and (_release == attrib.VersionNumber))) and
                ((_usagePage == ANY) or (isPreparsed and (_usagePage == caps.UsagePage))) and
                ((_usage == ANY) or (isPreparsed and (_usage == caps.Usage)))
                )
            {
                HidDeviceInfo temp;
                std::string nPath = std::string(deviceInterfaceDetailData->DevicePath);
                std::transform(nPath.begin(), nPath.end(), nPath.begin(), ::toupper);
                temp.path = g_toWide(nPath);

                if (isAttr)
                {
                    temp.vendorId = attrib.VendorID;
                    temp.productId = attrib.ProductID;
                    temp.release = attrib.VersionNumber;
                }

                if (isPreparsed)
                {
                    temp.usagePage = caps.UsagePage;
                    temp.usage = caps.Usage;
                }

                if (isSerial) { temp.serial = serial; }
                if (isManufa) { temp.manufacturer = manufa; }
                if (isProduc) { temp.product = produc; }


                temp.interfaceNumber = 0;
                size_t foundAt = nPath.find("&mi_");
                if (foundAt != std::string::npos)
                {
                    foundAt += 4;
                    temp.interfaceNumber = strtol(&(nPath[foundAt]), NULL, 16);
                }

                devices.push_back(temp);
            }

            CloseHandle(writeHandle);
            free(deviceInterfaceDetailData);
            deviceIndex++;
        }

        SetupDiDestroyDeviceInfoList(deviceInfoSet);

        return devices;
    }

private:

};