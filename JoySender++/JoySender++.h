#pragma once
#include <iostream>
#include <conio.h>
#include <csignal>
#include "TCPConnection.h"
#include "ArgumentParser.hpp"
#include "FPSCounter.hpp"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

// GLOBAL VARIABLES
constexpr auto APP_NAME = "NetJoy";

std::string g_outputText;
int g_joystickSelected = -1;

volatile sig_atomic_t APP_KILLED = 0;
void signalHandler(int signal);

// Function safely returns environment variables
std::string g_getenv(const char* variableName);
// Some Console Output, ANSI helper functions
void enableANSI();
void hideConsoleCursor();
void showConsoleCursor();
void clearConsoleScreen();
//void repositionConsoleCursor(char fwd, int linesFwd = 1, int offset = 0);
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
// Function waits for no keyboard presses to be detected before returning
void wait_for_no_keyboard_input();
// Function determines if app if the active window
bool IsAppActiveWindow();
// Main Loop Function 
int joySender(Arguments& args);