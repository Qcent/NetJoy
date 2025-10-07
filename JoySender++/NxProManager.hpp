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

#include <array>
#include <optional>
#include <cstdint>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <cmath>

#include "HidManager.h"
#include "DS4OutputReports.h"

 /*** LARGELY BASED OFF THE CODE FOUND AT: https://github.com/MTCKC/ProconXInput/blob/master/Controller.cpp ***/
 /*** AND shinyquagsire23 repository https://github.com/shinyquagsire23/HID-Joy-Con-Whispering/ ***/
/***  ALL THANKS TO EXTRAORDINARY EFFORT BY dekuNukem https://github.com/dekuNukem/Nintendo_Switch_Reverse_Engineering/ ***/

using uchar = unsigned char;

// NxController hid commands
constexpr uchar rumbleCommand{ 0x48 };
constexpr uchar imuDataCommand{ 0x40 };
constexpr uchar ledCommand{ 0x30 };
constexpr uchar spiReadCommand{ 0x10 };
const std::array<uchar, 1> led{ 0x1 };
const std::array<uchar, 1> enable{ 0x01 };
const std::vector<uint8_t> userCalibSet_flag{ 0xB2, 0xA1 };

// NxCalibration spi addresses
constexpr uint16_t ls_factory_cal_addr = 0x603D;
constexpr uint16_t rs_factory_cal_addr = 0x6046;
constexpr uint16_t imu_factory_cal_addr = 0x6020;
constexpr uint16_t ls_user_cal_addr = 0x8012;
constexpr uint16_t rs_user_cal_addr = 0x801D;
constexpr uint16_t imu_user_cal_addr = 0x8028;
// calibration data lengths
constexpr size_t stick_cal_size = 9;
constexpr size_t imu_cal_size = 24;

// pollInput
constexpr uchar getInput{ 0x1f };
//const std::array<uchar, 0> empty{};

// nintendo hid product ids
#define JOYCON_L_BT (0x2006)
#define JOYCON_R_BT (0x2007)
#define PRO_CONTROLLER (0x2009)
#define JOYCON_CHARGING_GRIP (0x200e)


struct NxControllerInfo {
    byte battery;     // "Full=4", "Empty=0"
    bool charging;    // true if charging
    byte connection;  // "Joy-Con=0", "Pro/Charging Grip=1"
    bool usbPowered;  // true if powered by USB
    byte loadedCalibration = 0; // bitfield (imu | rs | ls) values (0000-0111), bits are set in high nibble if user cal loaded
    imuCalibValues calibValues;
};

// Scale Pro → DS4 (smaller movement emulated when using these values)
constexpr float ACCEL_SCALE = 0.000244f; // default values
constexpr float GYRO_SCALE = 0.070f; // default values
constexpr int16_t DFLT_ACCEL_SCALE = 16384;
constexpr int16_t DFLT_GYRO_SCALE = 13371;
constexpr int16_t DFLT_CALIB_OFFSET = 0;


// calibration for 6-axis calculations
imuCalibValues ImuCal = {
    ACCEL_SCALE,ACCEL_SCALE,ACCEL_SCALE,
    GYRO_SCALE,GYRO_SCALE,GYRO_SCALE,
    DFLT_CALIB_OFFSET, DFLT_CALIB_OFFSET, DFLT_CALIB_OFFSET,
    DFLT_CALIB_OFFSET, DFLT_CALIB_OFFSET, DFLT_CALIB_OFFSET,
    DFLT_GYRO_SCALE, DFLT_GYRO_SCALE, DFLT_GYRO_SCALE,
    DFLT_ACCEL_SCALE, DFLT_ACCEL_SCALE, DFLT_ACCEL_SCALE
};

using uchar = unsigned char;

struct HD_RumbleFrame {
    float freqHi = 160.0f;
    float ampHi = 0.0f;
    float freqLo = 160.0f;
    float ampLo = 0.0f;
    int durationMs = 0;
};

class NxRumble {
public:
    // Singleton accessor
    static NxRumble& instance(HidDeviceManager* manager = nullptr, bool bt = false, bool forceNew = false) {
        std::lock_guard<std::mutex> lock(instanceMutex);

        if (!inst || forceNew) {
            if (inst) {
                // Destroy old
                inst->stop();
                inst.reset();
            }
            if (!manager) {
                throw std::runtime_error("RumblePatternPlayer: HidManager required for initialization");
            }
            inst.reset(new NxRumble(manager, bt));
        }

        return *inst;
    }

    // Delete copy & move (singleton style)
    NxRumble(const NxRumble&) = delete;
    NxRumble& operator=(const NxRumble&) = delete;
    NxRumble(NxRumble&&) = delete;
    NxRumble& operator=(NxRumble&&) = delete;

    ~NxRumble() {
        stop();
    }

    void start() {
        if (running) return;
        running = true;
        worker = std::thread(&NxRumble::run, this);
    }

    void stop() {
        running = false;
        if (worker.joinable())
            worker.join();
    }

    void setFrame(const HD_RumbleFrame& frame) {
        std::lock_guard<std::mutex> lock(frameMutex);
        currentFrame = frame;
    }

private:
    HidDeviceManager* hidManager;
    bool bluetooth;
    std::thread worker;
    std::atomic<bool> running;

    std::mutex frameMutex;
    HD_RumbleFrame currentFrame;

    static std::unique_ptr<NxRumble> inst;
    static std::mutex instanceMutex;

    NxRumble(HidDeviceManager* manager, bool bt)
        : hidManager(manager), bluetooth(bt), running(false) {
    }

    void run() {
        while (running) {
            HD_RumbleFrame local;

            // copy current frame under mutex
            {
                std::lock_guard<std::mutex> lock(frameMutex);
                local = currentFrame;
            }

            if (local.durationMs > 0) {
                // Build rumble HID payload
                uint8_t rumbleBuf[8];
                buildRumbleFrameSafe(rumbleBuf, local.freqHi, local.ampHi,
                    local.freqLo, local.ampLo);
                sendRumbleFrame(rumbleBuf);

                // reduce duration
                {
                    std::lock_guard<std::mutex> lock(frameMutex);
                    currentFrame.durationMs -= 15;
                    if (currentFrame.durationMs < 0)
                        currentFrame.durationMs = 0;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(18));
        }
    }

    void sendRumbleFrame(const uint8_t* rumbleBuf) {
        if (!bluetooth) {
            uint8_t usbBuf[0x9 + 8] = {
                0x80, 0x92, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x10
            };
            memcpy(usbBuf + 0x9, rumbleBuf, 8);
            hidManager->WriteFileOutputReport(usbBuf, hidManager->devInfo.output_report_length);
        }
        else {
            uint8_t btBuf[1 + 8];
            btBuf[0] = 0x10;
            memcpy(btBuf + 1, rumbleBuf, 8);
            hidManager->WriteFileOutputReport(btBuf, hidManager->devInfo.output_report_length);
        }
    }

    // Encoding helpers 
    uint16_t encodeFrequency(float freq) {
        if (freq < 40.875f) freq = 40.875f;
        if (freq > 1252.572266f) freq = 1252.572266f;
        int hf = (int)roundf(log2f(freq / 10.0f) * 32.0f);
        int low = hf & 0xF;
        int high = (hf >> 4) & 0xFF;
        return (high << 8) | low;
    }

    uint16_t encodeAmplitude(float amp, uint16_t freqEnc) {
        if (amp < 0.0f) amp = 0.0f;
        if (amp > 1.0f) amp = 1.0f;
        int hf = freqEnc & 0xF;
        int high = (freqEnc >> 8) & 0xFF;
        int encodedAmp = (int)(amp * 0xFFFF);
        encodedAmp = (encodedAmp >> (hf + 1)) & 0xFFFF;
        return encodedAmp | (high << 8);
    }

    void buildRumbleFrame(uint8_t* buf, float freqHi, float ampHi, float freqLo, float ampLo) {
        uint16_t freqEncHi = encodeFrequency(freqHi);
        uint16_t ampEncHi = encodeAmplitude(ampHi, freqEncHi);
        uint16_t freqEncLo = encodeFrequency(freqLo);
        uint16_t ampEncLo = encodeAmplitude(ampLo, freqEncLo);

        buf[0] = freqEncHi & 0xFF;
        buf[1] = (freqEncHi >> 8) & 0xFF;
        buf[2] = ampEncHi & 0xFF;
        buf[3] = (ampEncHi >> 8) & 0xFF;

        buf[4] = freqEncLo & 0xFF;
        buf[5] = (freqEncLo >> 8) & 0xFF;
        buf[6] = ampEncLo & 0xFF;
        buf[7] = (ampEncLo >> 8) & 0xFF;
    }

    void buildRumbleFrameSafe(uint8_t* buf, float freqHi, float ampHi, float freqLo, float ampLo) {
        uint16_t freqEncHi = encodeFrequency(freqHi);
        uint16_t ampEncHi = encodeAmplitude(ampHi, freqEncHi);
        uint16_t freqEncLo = encodeFrequency(freqLo);
        uint16_t ampEncLo = encodeAmplitude(ampLo, freqEncLo);

        // --- Pack left Joy-Con/(motor) (High + Low band) ---
        buf[0] = freqEncHi & 0xFF;         // HF lower nibble
        buf[1] = (freqEncHi >> 8) & 0xFF;  // HF upper byte
        buf[2] = ampEncHi & 0xFF;          // LF frequency or amplitude LSB
        buf[3] = (ampEncHi >> 8) & 0xFF;   // LF amplitude MSB

        // --- Pack right Joy-Con/(motor) (High + Low band) ---
        buf[4] = freqEncLo & 0xFF;
        buf[5] = (freqEncLo >> 8) & 0xFF;
        buf[6] = ampEncLo & 0xFF;
        buf[7] = (ampEncLo >> 8) & 0xFF;

        // --- Clamp to dekuNukem's safe ranges ---
        // High band lower nibble (HF LSB)
        if (buf[0] < 0x04) buf[0] = 0x04;
        if (buf[4] < 0x04) buf[4] = 0x04;
        if (buf[0] > 0xFC) buf[0] = 0xFC;
        if (buf[4] > 0xFC) buf[4] = 0xFC;

        // High band upper byte (HF MSB)
        if (buf[1] > 0xFC) buf[1] = 0xFC;
        if (buf[5] > 0xFC) buf[5] = 0xFC;

        // Low band frequency
        if (buf[2] < 0x01) buf[2] = 0x01;
        if (buf[6] < 0x01) buf[6] = 0x01;
        if (buf[2] > 0x7F) buf[2] = 0x7F;
        if (buf[6] > 0x7F) buf[6] = 0x7F;

        // Low band amplitude
        if (buf[3] < 0x40) buf[3] = 0x40;
        if (buf[7] < 0x40) buf[7] = 0x40;
        if (buf[3] > 0x72) buf[3] = 0x72;
        if (buf[7] > 0x72) buf[7] = 0x72;
    }
};

// Static/Singleton members
std::unique_ptr<NxRumble> NxRumble::inst = nullptr;
std::mutex NxRumble::instanceMutex;


class NxProController {
public:
    static constexpr size_t exchangeLen{ 0x16A };  // procontroller input reports size 362 bytes/0x16A
    using exchangeArray = std::optional<std::array<uchar, exchangeLen>>;
    NxControllerInfo info = {};

private:
    HidDeviceManager* hidManager;   // external manager 
    std::array<uchar, exchangeLen> _ret{}; // reuse this array for all reads from controller
    uchar txCounter{ 0 };
    bool bluetooth = true;
    WORD controller_type = 0;

    void parseBatteryAndConnection(const BYTE* buf) {

        uint8_t b = buf[2]; // byte #2
        uint8_t highNibble = (b & 0xF0) >> 4;
        uint8_t lowNibble = b & 0x0F;

        // Battery level
        switch (highNibble & 0xE) { // ignore charging bit for batt level
        case 8: info.battery = 4; break;  //"Full"; 
        case 6: info.battery = 3; break;  //"Medium";
        case 4: info.battery = 2; break;  //"Low";
        case 2: info.battery = 1; break;  //"Critical";
        case 0: info.battery = 0; break;  //"Empty";
        default: info.battery = 255; break;// "Unknown";
        }

        // Charging status (bit 0 of high nibble)
        info.charging = (highNibble & 1);

        // Connection type
        uint8_t connType = (lowNibble >> 1) & 0x3;
        if (connType == 3) info.connection = 0;  // Joy-Con
        else if (connType == 0) info.connection = 1; // Pro / Grip
        else info.connection = 255;

        // Power source
        info.usbPowered = (lowNibble & 1);
    }

    // --- Conversion to DS4 Reports  ---
    constexpr static _DS4_DPAD_DIRECTIONS extDpad2DS4(uint8_t dpadByte) {
        switch (dpadByte & 0x0F) {
        case 0x02: return DS4_BUTTON_DPAD_NORTH;      // Up
        case 0x06: return DS4_BUTTON_DPAD_NORTHEAST;  // Up + Right
        case 0x04: return DS4_BUTTON_DPAD_EAST;       // Right
        case 0x05: return DS4_BUTTON_DPAD_SOUTHEAST;  // Down + Right
        case 0x01: return DS4_BUTTON_DPAD_SOUTH;      // Down
        case 0x09: return DS4_BUTTON_DPAD_SOUTHWEST;  // Down + Left
        case 0x08: return DS4_BUTTON_DPAD_WEST;       // Left
        case 0x0A: return DS4_BUTTON_DPAD_NORTHWEST;  // Up + Left
        default:   return DS4_BUTTON_DPAD_NONE;       // Nothing pressed
        }
    }

    constexpr static _DS4_DPAD_DIRECTIONS simpleDpad2DS4(uint8_t dpadByte) {
        switch (dpadByte & 0x0F) {
        case 0x00: return DS4_BUTTON_DPAD_NORTH;      // Up
        case 0x01: return DS4_BUTTON_DPAD_NORTHEAST;  // Up + Right
        case 0x02: return DS4_BUTTON_DPAD_EAST;       // Right
        case 0x03: return DS4_BUTTON_DPAD_SOUTHEAST;  // Down + Right
        case 0x04: return DS4_BUTTON_DPAD_SOUTH;      // Down
        case 0x05: return DS4_BUTTON_DPAD_SOUTHWEST;  // Down + Left
        case 0x06: return DS4_BUTTON_DPAD_WEST;       // Left
        case 0x07: return DS4_BUTTON_DPAD_NORTHWEST;  // Up + Left
        default:   return DS4_BUTTON_DPAD_NONE;       // Nothing pressed
        }
    }

    static void setDS4ExtReportSticks(const uint8_t* data, DS4_REPORT_EX& ds4_report) {
        // Left Stick 
        const uint8_t* leftData = data + 6;
        /* OG Code *//*
        uint16_t rawLeftX = leftData[0] | ((leftData[1] & 0x0F) << 8);
        uint16_t rawLeftY = (leftData[1] >> 4) | (leftData[2] << 4);

        ds4_report.Report.bThumbLX = static_cast<uint8_t>((rawLeftX * 255) / 4095);
        ds4_report.Report.bThumbLY = static_cast<uint8_t>(255 - ((rawLeftY * 255) / 4095));
        */
        const uint16_t rawLeftX = leftData[0] | ((leftData[1] & 0x0F) << 8);
        const uint16_t rawLeftY = (leftData[1] >> 4) | (leftData[2] << 4);

        // Fast scaling from 12-bit to 8-bit (approx raw * 255 / 4095)
        const uint8_t lx = static_cast<uint8_t>((rawLeftX * 255 + 2047) >> 12);
        const uint8_t ly = static_cast<uint8_t>((rawLeftY * 255 + 2047) >> 12);

        ds4_report.Report.bThumbLX = lx;
        ds4_report.Report.bThumbLY = 255 - ly;

        // Right Stick 
        const uint8_t* rightData = data + 9;
        /* OG Code *//*
        uint16_t rawRightX = rightData[0] | ((rightData[1] & 0x0F) << 8);
        uint16_t rawRightY = (rightData[1] >> 4) | (rightData[2] << 4);

        ds4_report.Report.bThumbRX = static_cast<uint8_t>((rawRightX * 255) / 4095);
        ds4_report.Report.bThumbRY = static_cast<uint8_t>(255 - ((rawRightY * 255) / 4095));
        */
        const uint16_t rawRightX = rightData[0] | ((rightData[1] & 0x0F) << 8);
        const uint16_t rawRightY = (rightData[1] >> 4) | (rightData[2] << 4);

        // Fast scaling from 12-bit to 8-bit (approx raw * 255 / 4095)
        const uint8_t rx = static_cast<uint8_t>((rawRightX * 255 + 2047) >> 12);
        const uint8_t ry = static_cast<uint8_t>((rawRightY * 255 + 2047) >> 12);

        ds4_report.Report.bThumbRX = rx;
        ds4_report.Report.bThumbRY = 255 - ry;
    }

    static void setDS4SimpleSticks(const uint8_t* data, DS4_REPORT_EX& ds4_report) {
        // Left Stick 
        const uint8_t* leftData = data + 4;
        uint16_t rawLeftX = leftData[0] | (leftData[1] << 8);
        uint16_t rawLeftY = leftData[2] | (leftData[3] << 8);

        ds4_report.Report.bThumbLX = static_cast<uint8_t>((rawLeftX * 255) / 65535);
        ds4_report.Report.bThumbLY = static_cast<uint8_t>((rawLeftY * 255) / 65535);

        // Right Stick
        const uint8_t* rightData = data + 8;
        uint16_t rawRightX = rightData[0] | (rightData[1] << 8);
        uint16_t rawRightY = rightData[2] | (rightData[3] << 8);

        ds4_report.Report.bThumbRX = static_cast<uint8_t>((rawRightX * 255) / 65535);
        ds4_report.Report.bThumbRY = static_cast<uint8_t>((rawRightY * 255) / 65535);
    }

    inline static int16_t read_int16_le(const uint8_t* p) noexcept {
        // reinterpret as unsigned so shifts behave well
        uint16_t val = *reinterpret_cast<const uint16_t*>(p);
#if defined(_M_IX86) || defined(_M_X64)
        // On x86/x64, unaligned access is fine
        return static_cast<int16_t>(_byteswap_ushort(val));
#else
        // Fallback for architectures where unaligned access is not allowed
        return static_cast<int16_t>(p[0] | (p[1] << 8));
#endif
    }

    static void setDS4ImuValues(const uint8_t* data, DS4_REPORT_EX& ds4_report, imuCalibValues& cal) {
        constexpr int sampleCount = 3;
        constexpr int sampleSize = 12; // 6x int16 per sample

        //int16_t accelX = 0, accelY = 0, accelZ = 0, xNULL = 0;
        //int16_t gyroX = 0, gyroY = 0, gyroZ = 0;

       // for (int i = 0; i < sampleCount; ++i) {  //## removing averaging the 3 samples, for now
        const uint8_t* sample = data + 13;// +i * sampleSize;
            /*
            int16_t ax = static_cast<int16_t>(sample[0] | (sample[1] << 8));
            int16_t ay = static_cast<int16_t>(sample[2] | (sample[3] << 8));
            int16_t az = static_cast<int16_t>(sample[4] | (sample[5] << 8));
            int16_t gx = static_cast<int16_t>(sample[6] | (sample[7] << 8));
            int16_t gy = static_cast<int16_t>(sample[8] | (sample[9] << 8));
            int16_t gz = static_cast<int16_t>(sample[10] | (sample[11] << 8));
            
            accelX += ax;
            accelY += ay;
            accelZ += az;
            gyroX += gx;
            gyroY += gy;
            gyroZ += gz;
            */

        int16_t accelX = read_int16_le(sample + 0);
        int16_t accelY = read_int16_le(sample + 2);
        int16_t accelZ = read_int16_le(sample + 4);
        int16_t gyroX = read_int16_le(sample + 6);
        int16_t gyroY = read_int16_le(sample + 8);
        int16_t gyroZ = read_int16_le(sample + 10);
        /* }
        
        // Average
        accelX /= sampleCount;
        accelY /= sampleCount;
        accelZ /= sampleCount;
        gyroX /= sampleCount;
        gyroY /= sampleCount;
        gyroZ /= sampleCount;

        */
       
        // Convert

        // PITCH        //## passing the raw values is working, removing extra math (calibration calulations) for now
        ds4_report.Report.wAccelX = -accelY;// static_cast<int16_t>(-(accelY * cal.raw.accelScaleY) / cal.accel_divisor[1]); // left/right tilt
        // YAW
        ds4_report.Report.wAccelY = accelZ;//static_cast<int16_t>((accelX * cal.raw.accelScaleX) / cal.accel_divisor[0]);  // back/forward tilt
        // ROLL
        ds4_report.Report.wAccelZ = accelX;//static_cast<int16_t>((accelZ * cal.raw.accelScaleZ) / cal.accel_divisor[2]);  // left/right rotation

        // PITCH
        ds4_report.Report.wGyroX = -gyroY;// mult_frac(static_cast<int16_t>(JC_IMU_PREC_RANGE_SCALE * -(gyroY - cal.raw.gyroOffsetY)), cal.raw.gyroScaleY, cal.gyro_divisor[1]);
        // YAW
        ds4_report.Report.wGyroY = gyroZ;//mult_frac(static_cast<int16_t>(JC_IMU_PREC_RANGE_SCALE * (gyroX - cal.raw.gyroOffsetX)), cal.raw.gyroScaleX, cal.gyro_divisor[0]);
        // ROLL
        ds4_report.Report.wGyroZ = gyroX;//mult_frac(static_cast<int16_t>(JC_IMU_PREC_RANGE_SCALE * (gyroZ - cal.raw.gyroOffsetZ)), cal.raw.gyroScaleZ, cal.gyro_divisor[2]);

#if DEVTEST && defined(NetJoyTUI) // for visual on 6 axis data
        static size_t frames = 0;
        if (frames++ % 7 == 0) {
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
            std::wcout << L"X: " << accelX << L"   \tXg: " << gyroX << L"       \r\n";
            std::wcout << L"Y: " << accelY << L"   \tYg: " << gyroY << L"       \r\n";
            std::wcout << L"Z: " << accelZ << L"   \tZg: " << gyroZ << L"       \r\n";
        }
#endif
    }

    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ //
public:
    NxProController(HidDeviceManager* manager)
        : hidManager(manager), controller_type(manager->devInfo.productId) {
        bluetooth = wcscmp(hidManager->devInfo.serial.c_str(), L"000000000001");
    }

    // Sets info.loadedCalibration using 3 bit encoding of which calibration data is present:
    // Bit 0 (1): Left stick | Bit 1 (2): Right stick | Bit 2 (4): IMU calibration
    // factory calib in low nibble and user calib in high nibble
    byte loadCalibration() {
        // ---- Stick Calibration ----
        auto userCalCheck = spi_read(ls_user_cal_addr-0x2, 0x2);
        auto stickDataRaw = (userCalCheck == userCalibSet_flag) ? 
            spi_read(ls_user_cal_addr, stick_cal_size) :
            spi_read(ls_factory_cal_addr, stick_cal_size);
        bool lsCalib = parseStickCalibration(calib.leftStick, stickDataRaw);
        info.loadedCalibration |= lsCalib << ((userCalCheck == userCalibSet_flag) * 4);

        userCalCheck = spi_read(rs_user_cal_addr - 0x2, 0x2);
        stickDataRaw = (userCalCheck == userCalibSet_flag) ?
            spi_read(rs_user_cal_addr, stick_cal_size) :
            spi_read(rs_factory_cal_addr, stick_cal_size);
        bool rsCalib = parseStickCalibration(calib.rightStick, stickDataRaw);
        info.loadedCalibration |= rsCalib << ((userCalCheck == userCalibSet_flag) * 4)+1;

        // ---- IMU Calibration ----
        userCalCheck = spi_read(imu_user_cal_addr - 0x2, 0x2);
        auto imuRaw = (userCalCheck == userCalibSet_flag) ?
            spi_read(imu_user_cal_addr, imu_cal_size) :
            spi_read(imu_factory_cal_addr, imu_cal_size);
        bool imuCalib = parseImuCalibration(calib.imu, imuRaw);
        info.loadedCalibration |= imuCalib << ((userCalCheck == userCalibSet_flag) * 4) + 2;

        // ---- Set Calibration Values ----
        if (imuCalib) {
            ImuCal.raw = calib.imu;
            applyAccelCalibration();
            applyGyroCalibration();
        }
        // no valid stick calibration data was available for testing
        // so no stick calibration applied (getting good results on sticks as is)

        return info.loadedCalibration;
    }

    static bool convertToDS4Report(HidDeviceManager* hidManager, BYTE* ds4_report_buffer, imuCalibValues &cal = ImuCal) {
        static std::array<uchar, exchangeLen> Nx_report{};
        if (!hidManager->ReadFileInputReport(getInput, Nx_report.data(), (DWORD)exchangeLen))
            return false;

        /* Convert Nx_report.data() to a valad DS4 report and store in ds4_report_buffer */
        DS4_REPORT_EX* ds4_report = (DS4_REPORT_EX*)ds4_report_buffer;
        ds4_report->Report.wButtons = ds4_report->Report.bSpecial = 0;

        switch (Nx_report.data()[0]) {
        //case(0x21):
        case(0x30):  // extended reports
        case(0x31): case(0x32): case(0x33):
            /* OG Code
            if (Nx_report.data()[3] & 0x01) ds4_report->Report.wButtons |= DS4_BUTTON_SQUARE;   // Y
            if (Nx_report.data()[3] & 0x02) ds4_report->Report.wButtons |= DS4_BUTTON_TRIANGLE; // X
            if (Nx_report.data()[3] & 0x04) ds4_report->Report.wButtons |= DS4_BUTTON_CROSS;    // B
            if (Nx_report.data()[3] & 0x08) ds4_report->Report.wButtons |= DS4_BUTTON_CIRCLE;   // A

            if (Nx_report.data()[3] & 0x40) ds4_report->Report.wButtons |= DS4_BUTTON_SHOULDER_RIGHT; // R1
            if (Nx_report.data()[3] & 0x80) ds4_report->Report.wButtons |= DS4_BUTTON_TRIGGER_RIGHT;  // R2

            if (Nx_report.data()[5] & 0x40) ds4_report->Report.wButtons |= DS4_BUTTON_SHOULDER_LEFT;  // L1
            if (Nx_report.data()[5] & 0x80) ds4_report->Report.wButtons |= DS4_BUTTON_TRIGGER_LEFT;   // L2

            if (Nx_report.data()[4] & 0x04) ds4_report->Report.wButtons |= DS4_BUTTON_THUMB_RIGHT; // R3
            if (Nx_report.data()[4] & 0x08) ds4_report->Report.wButtons |= DS4_BUTTON_THUMB_LEFT;  // L3

            if (Nx_report.data()[4] & 0x01) ds4_report->Report.wButtons |= DS4_BUTTON_SHARE;   // Minus/Select
            if (Nx_report.data()[4] & 0x02) ds4_report->Report.wButtons |= DS4_BUTTON_OPTIONS; // Plus/Start

            if (Nx_report.data()[4] & 0x10) ds4_report->Report.bSpecial |= DS4_SPECIAL_BUTTON_PS;   // Home
            if (Nx_report.data()[4] & 0x20) ds4_report->Report.bSpecial |= DS4_SPECIAL_BUTTON_TOUCHPAD; // Capture

            ds4_report->Report.wButtons |= extDpad2DS4(Nx_report.data()[5]);        // Dpad
            ds4_report->Report.bTriggerR = (Nx_report.data()[3] & 0x80) ? 255 : 0;  // Analog right trigger
            ds4_report->Report.bTriggerL = (Nx_report.data()[5] & 0x80) ? 255 : 0;  // Analog left trigger
            */
        {
            /* Optimized Code v1*//*
            const uint8_t b3 = Nx_report.data()[3];
            const uint8_t b4 = Nx_report.data()[4];
            const uint8_t b5 = Nx_report.data()[5];

            // --- Buttons (word field)
            ds4_report->Report.wButtons =
                ((b3 & 0x01) ? DS4_BUTTON_SQUARE : 0) |
                ((b3 & 0x02) ? DS4_BUTTON_TRIANGLE : 0) |
                ((b3 & 0x04) ? DS4_BUTTON_CROSS : 0) |
                ((b3 & 0x08) ? DS4_BUTTON_CIRCLE : 0) |
                ((b3 & 0x40) ? DS4_BUTTON_SHOULDER_RIGHT : 0) |
                ((b3 & 0x80) ? DS4_BUTTON_TRIGGER_RIGHT : 0) |
                ((b5 & 0x40) ? DS4_BUTTON_SHOULDER_LEFT : 0) |
                ((b5 & 0x80) ? DS4_BUTTON_TRIGGER_LEFT : 0) |
                ((b4 & 0x04) ? DS4_BUTTON_THUMB_RIGHT : 0) |
                ((b4 & 0x08) ? DS4_BUTTON_THUMB_LEFT : 0) |
                ((b4 & 0x01) ? DS4_BUTTON_SHARE : 0) |
                ((b4 & 0x02) ? DS4_BUTTON_OPTIONS : 0);

            // --- DPad
            ds4_report->Report.wButtons |= extDpad2DS4(b5);

            // --- Special buttons (byte field)
            ds4_report->Report.bSpecial =
                ((b4 & 0x10) ? DS4_SPECIAL_BUTTON_PS : 0) |
                ((b4 & 0x20) ? DS4_SPECIAL_BUTTON_TOUCHPAD : 0);

            // --- Analog triggers (byte fields)
            ds4_report->Report.bTriggerR = (b3 >> 7) * 255; // bit 7 → 0 or 255
            ds4_report->Report.bTriggerL = (b5 >> 7) * 255; // bit 7 → 0 or 255
            */
            /* Optimized Code v2*/
            const uint8_t b3 = Nx_report.data()[3];
            const uint8_t b4 = Nx_report.data()[4];
            const uint8_t b5 = Nx_report.data()[5];

            uint16_t& buttons = ds4_report->Report.wButtons;
            // Map each source bit to its DS4 bit position
            buttons |= ((b3 >> 0) & 1) << 4;   // Square
            buttons |= ((b3 >> 1) & 1) << 7;   // Triangle
            buttons |= ((b3 >> 2) & 1) << 5;   // Cross
            buttons |= ((b3 >> 3) & 1) << 6;   // Circle
            buttons |= ((b3 >> 6) & 1) << 9;   // R1
            buttons |= ((b3 >> 7) & 1) << 11;  // R2

            buttons |= ((b5 >> 6) & 1) << 8;   // L1
            buttons |= ((b5 >> 7) & 1) << 10;  // L2

            buttons |= ((b4 >> 2) & 1) << 15;  // R3
            buttons |= ((b4 >> 3) & 1) << 14;  // L3
            buttons |= ((b4 >> 0) & 1) << 12;  // Share
            buttons |= ((b4 >> 1) & 1) << 13;  // Options

            buttons |= extDpad2DS4(b5);        // DPad

            // Special buttons (8-bit)
            uint8_t& special = ds4_report->Report.bSpecial;
            special |= ((b4 >> 4) & 1) << 0;  // PS
            special |= ((b4 >> 5) & 1) << 1;  // Touchpad
            
            // Triggers
            ds4_report->Report.bTriggerR = -(int8_t)((b3 >> 7) & 1); // expands 0→0,1→0xFF
            ds4_report->Report.bTriggerL = -(int8_t)((b5 >> 7) & 1);

        }
            setDS4ExtReportSticks(Nx_report.data(), *ds4_report);
            setDS4ImuValues(Nx_report.data(), *ds4_report, cal);

            break;
            
        case(0x3F):  // basic report

            if (Nx_report.data()[1] & 0x01) ds4_report->Report.wButtons |= DS4_BUTTON_CROSS;    // B
            if (Nx_report.data()[1] & 0x02) ds4_report->Report.wButtons |= DS4_BUTTON_CIRCLE;   // A
            if (Nx_report.data()[1] & 0x04) ds4_report->Report.wButtons |= DS4_BUTTON_SQUARE;   // Y
            if (Nx_report.data()[1] & 0x08) ds4_report->Report.wButtons |= DS4_BUTTON_TRIANGLE; // X

            if (Nx_report.data()[1] & 0x10) ds4_report->Report.wButtons |= DS4_BUTTON_SHOULDER_LEFT;  // L1 
            if (Nx_report.data()[1] & 0x20) ds4_report->Report.wButtons |= DS4_BUTTON_SHOULDER_RIGHT; // R1
            if (Nx_report.data()[1] & 0x40) ds4_report->Report.wButtons |= DS4_BUTTON_TRIGGER_LEFT;   // L2
            if (Nx_report.data()[1] & 0x80) ds4_report->Report.wButtons |= DS4_BUTTON_TRIGGER_RIGHT;  // R2
            
            ds4_report->Report.bTriggerL = (Nx_report.data()[1] & 0x40) ? 255 : 0;  // Analog left trigger
            ds4_report->Report.bTriggerR = (Nx_report.data()[1] & 0x80) ? 255 : 0;  // Analog right trigger

            if (Nx_report.data()[2] & 0x01) ds4_report->Report.wButtons |= DS4_BUTTON_SHARE;        // Minus/Select
            if (Nx_report.data()[2] & 0x02) ds4_report->Report.wButtons |= DS4_BUTTON_OPTIONS;      // Plus/Start
            if (Nx_report.data()[2] & 0x04) ds4_report->Report.wButtons |= DS4_BUTTON_THUMB_RIGHT;  // R3
            if (Nx_report.data()[2] & 0x08) ds4_report->Report.wButtons |= DS4_BUTTON_THUMB_LEFT;   // L3

            if (Nx_report.data()[2] & 0x10) ds4_report->Report.bSpecial |= DS4_SPECIAL_BUTTON_PS;   // Home
            if (Nx_report.data()[2] & 0x20) ds4_report->Report.bSpecial |= DS4_SPECIAL_BUTTON_TOUCHPAD; // Capture

            ds4_report->Report.wButtons |= simpleDpad2DS4(Nx_report.data()[3]);  // Dpad

            break;
        }
        /*-------------------------------*/

        return true;
    }

    void parseConnection(const exchangeArray& buf) {
        parseBatteryAndConnection((const BYTE*)&buf);
    }

    exchangeArray GetReport() {
        if (!hidManager->ReadFileInputReport(getInput, _ret.data(), (DWORD)exchangeLen))
            return {};

        return _ret;
    }

    template<size_t len>
    exchangeArray exchange(const std::array<uchar, len>& data) {
        if (!hidManager || !hidManager->IsDeviceOpen())
            return {};

        /* Make sure the right number of bytes are passed to WriteFile. Windows
           expects the number of bytes which are in the _longest_ report (plus
           one for the report number) bytes even if the data is a report
           which is shorter than that. Windows gives us this value in
           caps.OutputReportByteLength. If a user passes in fewer bytes than this,
           create a temporary buffer which is the proper size. */
        size_t report_length = hidManager->devInfo.output_report_length;
        unsigned char* buf;
        if (len >= report_length) {
            /* The user passed the right number of bytes. Use the buffer as-is. */
            buf = (unsigned char*)data.data();
        }
        else {
            /* Create a temporary buffer and copy data,
               pad with zeros. */
            buf = (unsigned char*)malloc(report_length);
            memcpy(buf, data.data(), len);
            memset(buf + len, 0, report_length - len);
        }

        // Write to HID
        if (!hidManager->WriteFileOutputReport(buf, (DWORD)report_length)) {
            if (buf != data.data())
                free(buf);
            return {};
        }
        if (buf != data.data())
            free(buf);

        // Read response
        if (!hidManager->ReadFileInputReport(0x00, _ret.data(), (DWORD)exchangeLen))
            return {};

        return _ret;
    }

    template<size_t len>
    exchangeArray sendCommand(uchar command, const std::array<uchar, len>& data) {
        // +9 for USB case, +1 for Bluetooth case
        constexpr size_t extra = 0x9;
        std::array<uchar, len + extra> buf{};

        if (!bluetooth) {
            // USB initialization
            buf[0x0] = 0x80;
            buf[0x1] = 0x92;
            buf[0x3] = 0x31;
            buf[0x8] = command;

            if constexpr (len > 0) {
                memcpy(buf.data() + 0x9, data.data(), len);
            }
        }
        else {
            // Bluetooth initialization
            buf[0x0] = command;

            if constexpr (len > 0) {
                memcpy(buf.data() + 0x1, data.data(), len);
            }
        }

        return exchange(buf);
    }

    template<size_t len>
    exchangeArray sendSubcommand(uchar command, uchar subcommand, const std::array<uchar, len>& data) {
        std::array<uchar, 10 + len> buf{
            static_cast<uchar>(txCounter++ & 0xF),
            0x00, 0x01, 0x40, 0x40,
            0x00, 0x01, 0x40, 0x40,
            subcommand
        };

        if constexpr (len > 0) {
            memcpy(buf.data() + 10, data.data(), len);
        }

        return sendCommand(command, buf);
    }

    exchangeArray sendSimpleRumble(uchar leftMotor, uchar rightMotor) {
        std::array<uchar, 9> buf{
            static_cast<uchar>(txCounter++ & 0xF),
            0x80, 0x00, 0x40, 0x40,
            0x80, 0x00, 0x40, 0x40
        };

        if (leftMotor != 0) {
            buf[1] = 0x08;
            buf[2] = leftMotor;
        }
        if (rightMotor != 0) {
            buf[5] = 0x10;
            buf[6] = rightMotor;
        }
        return sendCommand(0x10, buf);
    }

private:
    // Stick / IMU Calibiation things 

    struct StickCalibrationData {
        uint16_t centerX, centerY;
        uint16_t minX, maxX;
        uint16_t minY, maxY;
    };

    struct CalibrationData {
    public:
        StickCalibrationData leftStick = {0};
        StickCalibrationData rightStick = { 0 };
        ImuCalibrationData imu = { 0 };
    } calib;
   
    // read SPI flash block 
    std::vector<uint8_t> spi_read(uint32_t offs, uint8_t size) {
        std::array<uchar, 5> req{};
        req[0] = static_cast<uchar>(offs & 0xFF);
        req[1] = static_cast<uchar>((offs >> 8) & 0xFF);
        req[2] = static_cast<uchar>((offs >> 16) & 0xFF);
        req[3] = static_cast<uchar>((offs >> 24) & 0xFF);
        req[4] = size;

        constexpr int max_read_count = 100;
        int read_count = 0;
        while (read_count < max_read_count) {
            ++read_count;
            auto responseOpt = sendSubcommand(0x01, spiReadCommand, req);
            if (!responseOpt) continue; // retry

            const auto& response = *responseOpt;

            if ((uint8_t)response[0] == (uint8_t)0x21) {  // 0x21 is subcommand response report id
                // Extract the offset field from response
                uint32_t resp_offs = *(uint32_t*)(&response[0xF + (bluetooth ? 0 : 10)]);
                if (resp_offs != offs) {
                    continue; // wrong address, retry
                }
            }
            else continue; // wrong report

            // Copy out the data bytes
            std::vector<uint8_t> data(size);
            memcpy(data.data(), &response[0x14 + (bluetooth ? 0 : 10)], size);
            return data;
        }

        //std::cerr << "ERROR: Read error or timeout at offset 0x"
            //<< std::hex << offs << " for " << std::dec << (int)size << " bytes\n";
        return {};
    }

    bool parseStickCalibration(StickCalibrationData& stick, const std::vector<uint8_t>& raw) {
        if (raw.size() == 0) return false;
        stick.centerX = ((raw[1] << 8) & 0xF00) | raw[0];
        stick.centerY = (raw[2] << 4) | (raw[1] >> 4);
        stick.minX = ((raw[4] << 8) & 0xF00) | raw[3];
        stick.maxX = (raw[5] << 4) | (raw[4] >> 4);
        stick.minY = ((raw[7] << 8) & 0xF00) | raw[6];
        stick.maxY = (raw[8] << 4) | (raw[7] >> 4);
        return true;
    }

    bool parseImuCalibration(ImuCalibrationData& imu, const std::vector<uint8_t>& raw) {
        if (raw.size() == 0) return false;
        imu.gyroOffsetX = static_cast<int16_t>(raw[0] | (raw[1] << 8));
        imu.gyroOffsetY = static_cast<int16_t>(raw[2] | (raw[3] << 8));
        imu.gyroOffsetZ = static_cast<int16_t>(raw[4] | (raw[5] << 8));

        imu.accelOffsetX = static_cast<int16_t>(raw[6] | (raw[7] << 8));
        imu.accelOffsetY = static_cast<int16_t>(raw[8] | (raw[9] << 8));
        imu.accelOffsetZ = static_cast<int16_t>(raw[10] | (raw[11] << 8));

        imu.gyroScaleX = static_cast<int16_t>(raw[12] | (raw[13] << 8));
        imu.gyroScaleY = static_cast<int16_t>(raw[14] | (raw[15] << 8));
        imu.gyroScaleZ = static_cast<int16_t>(raw[16] | (raw[17] << 8));

        imu.accelScaleX = static_cast<int16_t>(raw[18] | (raw[19] << 8));
        imu.accelScaleY = static_cast<int16_t>(raw[20] | (raw[21] << 8));
        imu.accelScaleZ = static_cast<int16_t>(raw[22] | (raw[23] << 8));
        
        return true;
    }

    // Apply accelerometer calibration
    void applyAccelCalibration() {
        int16_t offset = 0;
        int16_t scale = 0;     

        offset = calib.imu.accelOffsetX; scale = calib.imu.accelScaleX; 
        ImuCal.accel_divisor[0] = info.calibValues.accel_divisor[0] = (scale - offset);

        offset = calib.imu.accelOffsetY; scale = calib.imu.accelScaleY;
        ImuCal.accel_divisor[1] = info.calibValues.accel_divisor[1] = (scale - offset);
    
        offset = calib.imu.accelOffsetZ; scale = calib.imu.accelScaleZ;
        ImuCal.accel_divisor[2] = info.calibValues.accel_divisor[2] = (scale - offset);
    }

    // Apply gyroscope calibration (degrees per second)
    void applyGyroCalibration() {
        int16_t offset = 0;
        int16_t scale = 0;

        offset = calib.imu.gyroOffsetX; scale = calib.imu.gyroScaleX;
        ImuCal.gyro_divisor[0] = info.calibValues.gyro_divisor[0] = (scale - offset);

        offset = calib.imu.gyroOffsetY; scale = calib.imu.gyroScaleY;
        ImuCal.gyro_divisor[1] = info.calibValues.gyro_divisor[1] = (scale - offset);

        offset = calib.imu.gyroOffsetZ; scale = calib.imu.gyroScaleZ;
        ImuCal.gyro_divisor[2] = info.calibValues.gyro_divisor[2] = (scale - offset);

    }
};






