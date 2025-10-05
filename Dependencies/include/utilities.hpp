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
#include<Windows.h>
#include <string>
#include <tchar.h>
#include <vector>

// Function safely return environment variables
std::string g_getenv(const char* variableName) {
    std::string value;
    size_t requiredSize = 0;
    getenv_s(&requiredSize, nullptr, 0, variableName);
    if (requiredSize > 0) {
        value.resize(requiredSize);
        getenv_s(&requiredSize, &value[0], requiredSize, variableName);
        value.pop_back(); // Remove the null-terminating character
    }
    return value;
}
// Some Console Output, ANSI helper functions
void enableANSI()
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD consoleMode;
    GetConsoleMode(consoleHandle, &consoleMode);

    // Enable ANSI escape sequences support
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    SetConsoleMode(consoleHandle, consoleMode);
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
void clearConsoleScreen()
{
    std::cout << "\033[2J\033[H";
}
void repositionConsoleCursor(int lines, int offset) {
    if (lines < 0) // Move the cursor up by the specified number of lines
        std::cout << "\033[" << abs(lines) << "F";
    else if (lines > 0) // Move the cursor down by the specified number of lines
        std::cout << "\033[" << lines << "E";
    // Move the cursor to the specified offset from the start of the line
    std::cout << "\033[" << offset << "G";
}
void clearConsoleLine() {
    std::cout << "\033[K";
}

// Function to detect if keyboard key is pressed
bool getKeyState(int KEYCODE) {
    return GetAsyncKeyState(KEYCODE) & 0x8000;
}
// Function to convert strings to wide strings
std::wstring g_toWide(std::string& str) {
    std::wstring wideString(str.begin(), str.end());
    return wideString;
}
std::string formatDecimalString(const std::string& str, UINT8 numDigits) {
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
// Function captures all input from the keyboard to clear buffer
void swallowInput()
{
    while (_kbhit())
    {
        _getch();
    }
}
// Function waits for no keyboard presses to be detected before returning
void wait_for_no_keyboard_input() {
    bool input = true;
    while (input) {
        input = false;
        // Check if any key is currently being pressed
        for (int i = 48; i < 90; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                input = true;
            }
        }
    }
    swallowInput();
}
// Function determines if app is the active window
bool IsAppActiveWindow()
{
    HWND consoleWindow = GetConsoleWindow();
    HWND foregroundWindow = GetForegroundWindow();

    return (consoleWindow == foregroundWindow);
}

#define IS_PRESSED      1
#define IS_RELEASED     0
std::unordered_map<int, bool> g_keyStateMap;
// function checks for KEYCODE state & logs to dict. returns true when state change mirrors the bool pressed
bool checkKey(int key, bool pressed) {
    bool state = GetAsyncKeyState(key) & 0x8000;
    if (state != g_keyStateMap[key]) {
        g_keyStateMap[key] = state;
        if (state) {
            // key is pressed
            if (pressed)
                return 1;
        }
        else {
            // key is released
            if (!pressed)
                return 1;
        }
    }
    return 0;
}

// checks if the Q button has been pressed and sets APP_KILLED to true
void checkForQuit() {
    if (IsAppActiveWindow() && checkKey('Q', IS_PRESSED) && getKeyState(VK_SHIFT)) // Q for Quit
        APP_KILLED = true;
}

bool checkForBack() {
    return (IsAppActiveWindow() && checkKey('B', IS_PRESSED) && getKeyState(VK_SHIFT)); // B for Back
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int findLastWhitespaceBefore(const char* str, size_t maxLen = 27) {
    if (!str) return 0;

    size_t len = std::strlen(str);
    if (len < maxLen) {
        return len;
    }

    for (int i = static_cast<int>(maxLen); i >= 0; --i) {
        if (std::isspace(static_cast<unsigned char>(str[i]))) {
            return i;
        }
    }

    return maxLen; // no whitespace found
}

// returns index of last seen match
int replaceXEveryNth(TCHAR* source, size_t sourceSize, const TCHAR* target, const TCHAR* replacement, int max_targets, int skip = 0) {
    int retVal = -1;
    size_t targetLen = _tcslen(target);
    size_t replacementLen = _tcslen(replacement);
    if (replacementLen > targetLen) {
        return retVal; // cannot replace with a larger length
    }
    int occurrences = 0;

    for (int i = 0; i < sourceSize; i++) {
        bool match = false;
        if (source[i] == _T('\0'))
            return retVal;
        if (source[i] == target[0]) {
            match = true;
            int j = 1;
            while (j < targetLen) {
                if (source[i + j] != target[j]) {
                    match = false;
                    break;
                }
                j++;
            }
        }
        if (match) {
            retVal = i;
            if (skip && (occurrences % (skip - 1) != 0)) {
                ++occurrences;
                continue;
            }
            ++occurrences;
            for (int j = 0; j < replacementLen; j++) {
                source[i + j] = replacement[j];
            }
            i += replacementLen - 1;
        }
        if (occurrences >= max_targets)
            break;
    }
    return retVal;
}

bool getCharsAtPosition(int x, int y, int len, TCHAR* buffer) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD charsRead = 0;
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };

    // Resize buffer to store the characters
    //buffer.resize(len, L' ');

    // Read console characters starting at (x, y)
    if (!ReadConsoleOutputCharacter(hConsole, &buffer[0], len, coord, &charsRead)) {
        return false;  // Reading failed
    }

    // Shrink buffer if fewer characters were read
    //buffer.resize(charsRead);
    return true;
}

void displayBytes(const byte* buffer, int bufferSize, int width = 72) {
    const int endl = (width / 3);
    for (int i = 0; i < bufferSize; i++) {
#ifdef NetJoyTUI
        wprintf(L"%02X ", buffer[i]);
#else
        printf("%02X ", buffer[i]);
#endif

        if ((i + 1) % endl == 0) {

#ifdef NetJoyTUI
            wprintf(L"\r\n");
#else
            printf("\r\n");
#endif    
        }

    }

#ifdef NetJoyTUI
    wprintf(L"\r\n");
#else
    printf("\r\n");
#endif    
}