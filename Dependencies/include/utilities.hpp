#pragma once
extern volatile sig_atomic_t APP_KILLED;

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

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}