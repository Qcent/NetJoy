#pragma once
#include <ViGEm/Client.h>
#include <csignal>
#include <conio.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "VIGEmClient.lib")

#include <thread>
#include <mutex>
std::mutex mtx;  // Mutex for synchronizing access to shared variables
std::string feedbackData; // For sending rumble data back to joySender

volatile sig_atomic_t APP_KILLED = 0;
void signalHandler(int signal);

std::vector<std::string> split(const std::string& str, char delimiter);
std::string formatDecimalString(const std::string& str, size_t numDigits);
void hideConsoleCursor();
void showConsoleCursor();
void repositionConsoleCursor(int linesfwd = 1, int offset = 0);
void clearConsoleLine(const std::string& text);
void overwriteFPS(const std::string& text);
void overwriteLatency(const std::string& text);

// Define the callback functions for Rumble Support 
VOID CALLBACK xbox_rumble( PVIGEM_CLIENT Client, PVIGEM_TARGET Target, 
    UCHAR LargeMotor, UCHAR SmallMotor, UCHAR LedNumber, LPVOID UserData){
#if 1
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
#if 1
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


std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}
std::string formatDecimalString(const std::string& str, size_t numDigits) {
    size_t decimalPos = str.find('.');
    if (decimalPos == std::string::npos) {
        // No decimal point found, return the original 
        // string with decimal and padded zeros
        std::string paddedStr = str + ".";
        paddedStr.append(numDigits, '0');
        return paddedStr;
    }
    size_t strLen = str.length();
    size_t numDecimals = strLen - decimalPos - 1;
    if (numDecimals <= numDigits) {
        // No need to truncate or pad, return the original string
        return str;
    }
    // Truncate the string to the desired number of decimal places
    std::string truncatedStr = str.substr(0, decimalPos + numDigits + 1);
    // Pad with zeros if necessary
    if (numDecimals < numDigits) {
        truncatedStr.append(numDigits - numDecimals, '0');
    }

    return truncatedStr;
}
void hideConsoleCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}
void showConsoleCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandle, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &cursorInfo);
}
void repositionConsoleCursor(int lines, int offset) {
    if (lines < 0) // Move the cursor up by the specified number of lines
        std::cout << "\033[" << abs(lines) << "F";
    else if (lines > 0) // Move the cursor down by the specified number of lines
        std::cout << "\033[" << lines << "E";
    // Move the cursor to the specified offset from the start of the line
    std::cout << "\033[" << offset << "G";
}
void clearConsoleLine(const std::string& text) {
    std::cout << "\033[K";
}
void overwriteFPS(const std::string& text) {
    // Move the cursor to the beginning of the last line
    std::cout << "\033[F";
    // Write the new text
    std::cout << text + "  " << std::endl;
}
void overwriteLatency(const std::string& text) {
    repositionConsoleCursor(-1, 27);
    std::cout << text << std::endl;
}

// Function captures all input from the keyboard to clear buffer
void swallowInput()
{
    while (_kbhit())
    {
        _getch();
    }
}
