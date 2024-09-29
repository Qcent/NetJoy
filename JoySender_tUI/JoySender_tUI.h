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

#include <cwchar>
#include <codecvt>
#include <fcntl.h>
#include <io.h>
#include <wchar.h>
#include <thread>
#include <functional>

#include "TCPConnection.h"
#include "FPSCounter.hpp"

#undef min
#undef max

#include "./../JoySender++/JoySender++.h"

#define APP_VERSION_NUM     L"2.0.0.0"

#define CANCELLED_FLAG      -2
#define DISCONNECT_ERROR    -3

byte RESTART_FLAG = 0;
byte MAPPING_FLAG = 0;
bool OLDMAP_FLAG = 0;

void signalHandler(int signal);

///////
/// textUI 
//////
#include "tUI/textUI.h"

void exitAppCallback(mouseButton& button);
void joystickSelectCallback(mouseButton& button);
void newControllerColorsCallback(mouseButton& button);
void printCurrentController(SDLJoystickData& activeGamepad);
char IpInputLoop();
int screenLoop(textUI& screen);
int tUISelectJoystickDialog(SDLJoystickData& joystick);
int tUISelectDS4Dialog(std::vector<HidDeviceInfo>devList, textUI& screen, int autoSelect);
std::string tUIGetHostAddress(textUI& screen);
void threadedEstablishConnection(TCPConnection& client, int& retVal);
int tUIMapTheseInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>& inputList);
int tUIRemapInputsScreen(SDLJoystickData& joystick);

void tUI_RECOLOR_MAIN_LOOP();
void tUI_REDRAW_MAIN_LOOP();
void JOYSENDER_tUI_BUILD_MAP_SCREEN();

#define MAP_BUTTON_CLICKED 10000  // a value to add to an input index to indicate it was clicked on

#pragma warning(disable : 4996)
// Converts strings to wide strings and back again
std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

int g_joystickSelected = -1;

// ?? ???? ????? ????? ??  ???????? //
// Defines for ease and frustration
#define THEME_FILE    "send.theme"

#define DEFAULT_EGG_BUTTS() \
mouseButton colorEgg(50, consoleHeight - 3, 1, L"©"); \
colorEgg.SetId(43); \
g_screen.AddButton(&colorEgg); \
colorEgg.setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        g_status |= COLOR_EGG_b; \
    } \
}); \
mouseButton borderEgg(21, 3, 3, L"tUI"); \
borderEgg.SetId(44); \
borderEgg.setCallback([](mouseButton& btn) { \
    if (btn.Status() & MOUSE_UP) { \
        btn.SetStatus(MOUSE_OUT); \
        if (!(g_status & BORDER_EGG_a)) { \
            MORPH_BORDER(); \
            if (generateRandomInt(1, 5) > 4) { \
                MAKE_PATTERNS(); \
                g_status |= PTRN_EGG_b; \
            } \
            HEARTS_N_SPADES_BUTTONS(); \
            g_status |= RECOL_tUI_f; \
        } \
    } \
}); \
g_screen.AddButton(&borderEgg); 


#define JOYSENDER_tUI_INIT_FOOTER_ANIMATION() \
mouseButton leftBorderPiece(3, 18, 2, L"\\\t\t\t \t"); \
mouseButton rightBorderPiece(68, 18, 5, rsideFooter); \
leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
int frameDelay = 0; \
int footFrameNum = 0; \
mouseButton ani(3, 18, 10, SendFooterAnimation[footFrameNum]); \
ani.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg); \
int lastAni1Frame = g_frameNum - 1; \
int lastAni2Frame = footFrameNum - 1; 


#define JOYSENDER_tUI_ANIMATE_FOOTER(){ \
if ((g_status & BORDER_EGG_a) && (lastAni2Frame != footFrameNum)) { \
    lastAni2Frame = footFrameNum; \
    ani.SetText(SendFooterAnimation[footFrameNum]); \
    for (int i = 0; i < 7; ++i) { \
        ani.SetPosition(3 + (i * 10), 18); \
        ani.Draw(); \
    } \
    leftBorderPiece.Draw(); \
    rightBorderPiece.Draw(); \
} \
if (frameDelay % 8 == 0) { \
    loopCount(footFrameNum, SEND_FOOTER_ANI_FRAME_COUNT); \
} \
++frameDelay; \
}


//
// text buffer setting functions

void setErrorMsg(const wchar_t* text, size_t length) {
    wcsncpy_s(errorPointer, length, text, _TRUNCATE);
    errorOut.SetText(errorPointer);
}

void updateFPS(const wchar_t* text, size_t length) {
    wcsncpy_s(fpsPointer, length, text, _TRUNCATE);
    fpsMsg.SetText(fpsPointer);
    fpsMsg.Draw();
}


// ********************************
//  mouseButton callback functions

 // experimental global to extend callback functionality without redesigning the entire class and API up to this point
void* g_extraData = nullptr;

// callback links buttons status with _partner
void ShareButtonStatusCallback(mouseButton& button) {
    auto other = button.GetPartner();
    other->SetStatus(button.Status());
    other->Update();
}

// used by quit button, sets APP_KILLED to true
void exitAppCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        APP_KILLED = true;
    }
}

// for selecting joystick by mouse click, will set g_joystickSelected to button._id
void joystickSelectCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        g_joystickSelected = button.GetId();
    }
    else if (button.Status() & MOUSE_HOVERED) {
        mouseButton* mapBtn = (mouseButton *) g_extraData;
        if (mapBtn == nullptr) return;

        MAPPING_FLAG = button.GetId();

        if (g_status & PTRN_EGG_b) {
            wchar_t (*bgFill)[19] = reinterpret_cast<wchar_t(*)[19]>(msgPointer3);
            for (int i = 0; i < 6; i++) {
                setCursorPosition(consoleWidth - 24, 6 + i);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << bgFill[i];
            }
        }
        else if(mapBtn->Settings()==0){
            mapBtn->Clear(fullColorSchemes[g_currentColorScheme].menuBg);
        }
        COORD loc = button.GetPosition();
        mapBtn->SetPosition(consoleWidth - 24, loc.Y - 1);
        mapBtn->SetSettings(0); // clear INACTIVE
        mapBtn->Update();
    }
}

// will generate a new color scheme for program and controller face
void newControllerColorsCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        WORD newRandomBG = generateRandomInt(0, 15) << 4;

        // update globals for universal color consistency
        g_simpleScheme = createRandomScheme();
        ColorScheme menuScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(g_simpleScheme, menuScheme, newRandomBG);
        g_currentColorScheme = RANDOMSCHEME;

        // draw messages
        g_status |= RECOL_tUI_f | REDRAW_tUI_f | REFLAG_tUI_f;
        g_status &= ~tUI_THEME_af;
    }
}

// handles status and color changes to the UI restart button array for restartButton[1] & [2]
void restartModeCallback(mouseButton& button) {
    static int lastStatus = 0;
    static std::wstring modeCheckValue(L"1");

    // if the status has not changed do nothing
    if (button.Status() == lastStatus) return;


    if (button.Status() & (MOUSE_HOVERED)) {

        // Get mode value from button text
        std::wstring modetxt = button.getTextPtr();

        // if mouse released on number restart in that mode
        if (button.Status() & MOUSE_UP) {
            /*
            if (modetxt == modeCheckValue)
                RESTART_FLAG = 2;
            else
                RESTART_FLAG = 3;
            */
            RESTART_FLAG = (modetxt != modeCheckValue) + 2;  // non branching

            for (int i = 0; i < 3; ++i)
                restartButton[i].SetStatus(MOUSE_OUT);

            return;
        }


        // create new text for button
        std::wstring modeText = L"[mode " + modetxt + L"] ";
        // store text in msgPointer3 
        swprintf(msgPointer3, modeText.size() + 1, L"%s", modeText.c_str());
        // update button text
        restartButton[3].SetText(msgPointer3);


        // correct button colors
        WORD correctColor;

        if (button.Status() & MOUSE_DOWN || restartButton[0].Status() & MOUSE_DOWN)
            correctColor = restartButton[0].getCurrentColor();
        else
            correctColor = button.getCurrentColor();


        restartButton[3].Draw(correctColor);

        /*
        if (modetxt == modeCheckValue) {
            restartButton[2].Draw(correctColor);
        }
        else {
            restartButton[1].Draw(correctColor);
        }
        */
        restartButton[(modetxt == modeCheckValue) + 1].Draw(correctColor); // non branching, yay!

        restartButton[1].SetDefaultColor(correctColor);
        restartButton[2].SetDefaultColor(correctColor);
    }

    else if (button.Status() == MOUSE_OUT) {
        restartButtonCover.Draw();
        restartButton[3].SetText(L"[mode] ");
        restartButton[3].Draw(restartButton[0].getCurrentColor());
    }
    lastStatus = button.Status();
}

// handles status and color changes to the UI restart button array for restartButton[0]
void restartStatusCallback(mouseButton& button) {
    static int lastStatus = 0;

    if (button.Status() & MOUSE_UP) {
        RESTART_FLAG = 1;
        for (int i = 0; i < 3; ++i)
            restartButton[i].SetStatus(MOUSE_OUT);
        return;
    }

    if (button.Status() == lastStatus) return;

    byte a = restartButton[1].Status();  // mode 1 button
    byte b = restartButton[2].Status();  // mode 2 button

    if ((a | b) & (MOUSE_DOWN | MOUSE_HOVERED)) {
        // mode 1 or 2 button is the target
        button.SetStatus(MOUSE_HOVERED);  // cancel mouse down on main restart button
    }
    else {
        // The status has changed // make colors match
        restartButton[1].SetDefaultColor(button.getCurrentColor());
        restartButton[1].Draw(button.getCurrentColor());

        restartButton[2].SetDefaultColor(button.getCurrentColor());
        restartButton[2].Draw(button.getCurrentColor());

        restartButton[3].Draw(button.getCurrentColor());
    }

    lastStatus = button.Status();
}

// callback for UI button that activates mapping screen
void mappingButtonCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        MAPPING_FLAG = 1;
    }
}

// callback for face buttons in mapping screen
void mappingControllerButtonsCallback(mouseButton& button) {
    static int msgDisplayed = 0;
    static textBox topMsg(CONSOLE_WIDTH / 2, 5, 15, 1, ALIGN_CENTER, L" Click To Map ", BRIGHT_MAGENTA);
    static textBox footerMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 1, 24, 1, ALIGN_CENTER, L" Click Button to Remap ", DEFAULT_TEXT);

    static tUIColorPkg colors(0,0,0,0);

    // update colors if needed
    if (!(fullColorSchemes[g_currentColorScheme].controllerColors == colors)) {
        colors = fullColorSchemes[g_currentColorScheme].controllerColors;
        topMsg.SetColor(colors.col3);
        footerMsg.SetColor(colors.col1);
    }
    
    // use g_extraData as a pointer to int currentHoveredButton shared with calling function 
    int* currentHoveredButton = static_cast<int*>(g_extraData);

    // 'Click To Map' messaging and currentHoveredButton logic *only if button not spoofed by joystick
    if(!(button.Status() & SETBYGAMEPAD))
    {
        // display 'click to map' message if not already displayed
        if (button.Status() & MOUSE_HOVERED && !msgDisplayed) {
            topMsg.Draw();
            footerMsg.SetColor(colors.col2);
            footerMsg.Draw();
        
            *currentHoveredButton = button.GetId();

            msgDisplayed = true;
        }
        else if (button.Status() == MOUSE_HOVERED && msgDisplayed == 1)
        {       // if it is displayed increment the msgDisplayed counter
            ++msgDisplayed; //covers conditions where buttons are right next to or overlapping

            *currentHoveredButton = button.GetId();
        }

        // decrement msgDisplayed counter and clear message from screen if 0
        else if (button.Status() == MOUSE_OUT && msgDisplayed) {
            --msgDisplayed;
            if (!msgDisplayed) {
                topMsg.Clear(colors.col2);
                footerMsg.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);
                footerMsg.Draw();
                *currentHoveredButton = -1;
            }
        }
    }

    // Share status with button partners // for highlighting
    auto other = button.GetPartner();
    if (other != nullptr) {
        other->SetStatus(button.Status());
        other->Update();
    }

    // If clicked and not spoofed by joystick, set a flag or something
    if ((button.Status() & MOUSE_DOWN) && !(button.Status() & SETBYGAMEPAD)){
        *currentHoveredButton += MAP_BUTTON_CLICKED; // add value higher then any conceivable input index
    }

    // if click released
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
    }
}

// used by Done button, callback for ending mapping process/screen
void mappingDoneButtonCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        MAPPING_FLAG = 0;
        // reset controller buttons
        textUI buttonSetter;
        AddControllerButtons(buttonSetter);
        buttonSetter.SetButtonsCallback(nullptr);
        CoupleControllerButtons();

    }
}

// used by All Inputs button, callback for mapping all inputs
void mappingAllButtonsCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        MAPPING_FLAG = 2;
    }
}


// ********************************
// tUI Helper Functions

// Clears and sets a XUSB_REPORT from a vector of SDLButtonMapping::ButtonMapInput
void get_xbox_report_from_activeInputs(SDLJoystickData& joystick, const std::vector<SDLButtonMapping::ButtonMapInput>& activeInputs, XUSB_REPORT& xbox_report) {
    xbox_report = { 0 };
    for (const auto& activeInput : activeInputs) {
        if (activeInput.input_type == SDLButtonMapping::ButtonType::BUTTON) 
            get_xbox_report_common(joystick, activeInput, activeInput.value, xbox_report);
        else
            get_xbox_report_common(joystick, activeInput, activeInput.range, xbox_report);
    }
}

// returns a list of active joystick inputs
std::vector<SDLButtonMapping::ButtonMapInput> get_sdljoystick_input_list(const SDLJoystickData& joystick) {
    std::vector<SDLButtonMapping::ButtonMapInput> inputs;
    SDLButtonMapping::ButtonMapInput input;

    // Get the joystick state
    SDL_UpdateJoysticks();
    // clear un-needed joystick events 
    SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);

    // Iterate over all joystick axes
    for (int i = 0; i < joystick.num_axes; i++) {
        int axis_value = SDL_GetJoystickAxis(joystick._ptr, i); // / 32767.0f;
        if (std::abs(axis_value - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD) {
            input.set(SDLButtonMapping::ButtonType::STICK, i, axis_value < 0 ? ANALOG_RANGE_NEG : ANALOG_RANGE_POS);
            input.range = axis_value;
            inputs.push_back(input);
        }
    }

    // Iterate over all joystick buttons
    for (int i = 0; i < joystick.num_buttons; i++) {
        if (SDL_GetJoystickButton(joystick._ptr, i)) {
            input.set(SDLButtonMapping::ButtonType::BUTTON, i, 1);
            inputs.push_back(input);
        }
    }

    // Iterate over DPad hats and record their value
    for (int i = 0; i < joystick.num_hats; i++) {
        int hat_direction = SDL_GetJoystickHat(joystick._ptr, i);
        if (hat_direction != 0) {
            input.set(SDLButtonMapping::ButtonType::HAT, i, hat_direction);
            input.range = input.value;
            inputs.push_back(input);
        }
    }

    return inputs;
}

// Attempts to open an HID connection to a ds4 device
bool uiConnectToDS4Controller(HidDeviceInfo* selectedDev) {
    if (selectedDev != nullptr) {
        if (DS4manager.OpenHidDevice(selectedDev)) {
            //std::wcout << "Connected to : " << selectedDev->manufacturer << " " << selectedDev->product << std::endl;
            return true;
        }
    }
    return false;
}

// returns a list of DS4 controllers by HidDeviceInfo info
std::vector<HidDeviceInfo> getDS4ControllersList() {
    std::vector<HidDeviceInfo> devList;

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

    if (!devList.size()) {
         setErrorMsg(L" No DS4 Devices Connected! ", 28);
    }

    return devList;
}

// to allow for smooth animation, establish connection to host in a separate thread
void threadedEstablishConnection(TCPConnection& client, int& retVal) {
    while (!APP_KILLED && retVal == WSAEWOULDBLOCK)
    {
        retVal = client.establish_connection();
        if (!APP_KILLED && retVal == WSAEWOULDBLOCK)
            Sleep(50);
    }
}

// Prints the current controller header to screen
void printCurrentController(SDLJoystickData& activeGamepad) {
    if (g_mode == 1)
        g_outputText = " Using: " + activeGamepad.name + " ";
    else
    {
        g_outputText = " Using: DS4 Controller ";
        if (ds4DataOffset == DS4_VIA_BT) { g_outputText += "| Wireless |"; }
        else if (ds4DataOffset == DS4_VIA_USB) { g_outputText += "| USB |"; }
    }
    g_outputText += " ";
    setCursorPosition(8, 5);
    std::wcout << g_toWide(g_outputText);

    if (g_mode == 1)
        g_outputText = " Mode 1: (XBOX) ";
    else
        g_outputText = " Mode 2: (DS4) ";

    setCursorPosition(48, 5);
    std::wcout << g_toWide(g_outputText);
}

// accepts keyboard input for ip addresses
char IpInputLoop() {
    if (!IsAppActiveWindow()) { 
        return 'X'; 
    }

    if (checkKey('0', IS_PRESSED) || checkKey(VK_NUMPAD0, IS_PRESSED))
        return '0';
    else if (checkKey('1', IS_PRESSED) || checkKey(VK_NUMPAD1, IS_PRESSED))
        return '1';
    else if (checkKey('2', IS_PRESSED) || checkKey(VK_NUMPAD2, IS_PRESSED))
        return '2';
    else if (checkKey('3', IS_PRESSED) || checkKey(VK_NUMPAD3, IS_PRESSED))
        return '3';
    else if (checkKey('4', IS_PRESSED) || checkKey(VK_NUMPAD4, IS_PRESSED))
        return '4';
    else if (checkKey('5', IS_PRESSED) || checkKey(VK_NUMPAD5, IS_PRESSED))
        return '5';
    else if (checkKey('6', IS_PRESSED) || checkKey(VK_NUMPAD6, IS_PRESSED))
        return '6';
    else if (checkKey('7', IS_PRESSED) || checkKey(VK_NUMPAD7, IS_PRESSED))
        return '7';
    else if (checkKey('8', IS_PRESSED) || checkKey(VK_NUMPAD8, IS_PRESSED))
        return '8';
    else if (checkKey('9', IS_PRESSED) || checkKey(VK_NUMPAD9, IS_PRESSED))
        return '9';
    else if (checkKey(VK_BACK, IS_PRESSED))
        return 'B';
    else if (checkKey(VK_DELETE, IS_PRESSED))
        return 'D';
    else if (checkKey(VK_TAB, IS_PRESSED))
        return 'N';
    else if (checkKey(VK_OEM_PERIOD, IS_PRESSED) || checkKey(VK_DECIMAL, IS_PRESSED))
        return '.';
    else if (checkKey(VK_UP, IS_PRESSED))
        return '^';

    return 'X';
}

// will look for a saved controller mapping and open it or initiate the mapping process
void uiOpenOrCreateMapping(SDLJoystickData& joystick, textUI& screen) {
    // Convert joystick name to hex  
    std::string mapName = encodeStringToHex(joystick.name);
    // Check for a saved Map for selected joystick
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;
    if (result.first) {
        // File exists, try and load data
        int allGood = joystick.mapping.loadMapping(filePath.string());
        if (!allGood) {
            // invald data, delete file and try to remap controller
            std::filesystem::remove(result.second);
            return uiOpenOrCreateMapping(joystick, screen);
        }
    }
    else {
        // File does not exist
        
        // set up UI for xbox controller face
        BuildXboxFace();
        SetControllerButtonPositions(1);

        // set mapping flag = 2 to indicate that all inputs should be mapped
        MAPPING_FLAG = 2;
        // launch the mapping inputs screen
        tUIRemapInputsScreen(joystick);
    }
}


// ********************************
// Previous Host IP Memories
#define IPS_TO_REMEMBER     5
struct previousIPData {
    wchar_t address[IPS_TO_REMEMBER][16] = {L'\0'};
    BYTE index = 0;
};

previousIPData g_previousIPData;

void saveIPDataToFile() {
    std::string appdataFolder = g_getenv("APPDATA");
    appdataFolder += "\\" + std::string(APP_NAME);
    std::filesystem::path saveFolder = std::filesystem::path(appdataFolder);
    std::filesystem::path filename = saveFolder / ("previous.ips");
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(&g_previousIPData), sizeof(previousIPData));
        file.close();
    }
    else {
        //std::cerr << "Error: Unable to open pevious.ips file for writing." << std::endl;
    }
}

void loadIPDataFromFile() {
    std::string appdataFolder = g_getenv("APPDATA");
    appdataFolder += "\\" + std::string(APP_NAME);

    std::filesystem::path saveFolder = std::filesystem::path(appdataFolder);
    std::filesystem::path filename = saveFolder / ("previous.ips");

    previousIPData data;
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&data), sizeof(previousIPData));
        file.close();
    }
    else {
        //std::cerr << "Error: Unable to open pevious.ips file for reading." << std::endl;
        for (int i = 0; i < IPS_TO_REMEMBER; ++i) {
            data.address[i][0] = '\0';  // load empty address 'strings'
        }
    }

    data.index = 0;
    
    g_previousIPData = data;
}

int isDuplicateIP(const wchar_t newData[16]) {
    for (int i = 0; i < IPS_TO_REMEMBER; i++) {
        if (std::wcscmp(g_previousIPData.address[i], newData) == 0) {
            return i; // If duplicate is found, return index
        }
    }
    return -1; // If no duplicates found, return neg
}

bool pushNewIP(const wchar_t newData[16]) {
    int idx = isDuplicateIP(newData);
    if (idx == 0) return true;
    if (idx > 0) {
        memset(g_previousIPData.address[idx], '\0', 16);
    }
    for (int i = (idx>0 ? idx : (IPS_TO_REMEMBER - 2)); i >= 0; i--) {
        std::memcpy(g_previousIPData.address[i + 1], g_previousIPData.address[i], sizeof(g_previousIPData.address[i]));
    }
    std::memcpy(g_previousIPData.address[0], newData, sizeof(g_previousIPData.address[0]));

    return true;
}



// ********************************
// tUI Screens 

int tUISelectJoystickDialog(SDLJoystickData& joystick) {
    constexpr int START_LINE = 7;
    constexpr int START_COL = 20;
    constexpr int MAX_JOYSTICKS = 9;

    int numJoysticks = 0;
    SDL_JoystickID* joystick_list = nullptr;
    joystick_list = SDL_GetJoysticks(&numJoysticks);

    if (!numJoysticks) {
        errorOut.SetPosition(consoleWidth / 2, START_LINE+1, 50, 0, ALIGN_CENTER);
    }
    else {
        errorOut.SetPosition(consoleWidth / 2, 4, 50, 0, ALIGN_CENTER);
    }

    // Enforce max joysticks
    numJoysticks = std::min(numJoysticks, MAX_JOYSTICKS);
    mouseButton* availableJoystickBtn = new mouseButton[numJoysticks];

    JOYSENDER_tUI_INIT_FOOTER_ANIMATION();

    // for drawing back covered up bg patterns
    bool loadedBgFill = false;
    constexpr int LINES_TO_FILL = 6;
    constexpr int fill1len = 18, fill1x = consoleWidth-24, fill1y = 7;
    wchar_t (*bgFill)[fill1len + 1] = reinterpret_cast<wchar_t(*)[fill1len + 1]>(msgPointer3);


    // setup some txt
    wcsncpy_s(msgPointer1, 21, L" Select a Joystick: ", _TRUNCATE);
    output1.SetText(msgPointer1);

    // map from sellect screen
    mouseButton remapSelectScreenButton(CONSOLE_WIDTH + 10, 8, 18, L"\t\t\t\t╔════════════╗◄───╢ MAP INPUTS ║\t\t\t\t╚════════════╝");
    remapSelectScreenButton.SetSettings(INACTIVE);
    remapSelectScreenButton.setCallback([](mouseButton& btn) {
        if (btn.Settings() & INACTIVE) return;
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            MAPPING_FLAG += 200;
        }
        });
    g_extraData = (void*)&remapSelectScreenButton;
    g_screen.AddButton(&remapSelectScreenButton);

    // handy helpers
    auto cleanMemory = [&]() {
        SDL_free(joystick_list);
        for (int i = 0; i < numJoysticks; ++i) {
            g_screen.RemoveButton(&availableJoystickBtn[i]);
            delete[] availableJoystickBtn[i].getTextPtr();
        }
        g_screen.ClearButtonsExcept(HEAP_BTN_IDs);
        delete[] availableJoystickBtn;
    };
    auto reset_shared_resources = [&]() {
        g_screen.SetBackdrop(JoySendMain_Backdrop);
        g_extraData = (void*)&remapSelectScreenButton;
        output1.SetPosition(14, 5, 50, 1, ALIGN_LEFT);
        wcsncpy_s(msgPointer1, 21, L" Select a Joystick: ", _TRUNCATE);
        output1.SetText(msgPointer1);
        errorOut.SetPosition(consoleWidth / 2, 4, 50, 0, ALIGN_CENTER); // unconfirmed?
        };
    auto re_color = [&]() {
        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
        g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        ani.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);    
        COLOR_EGGS();
        WORD bg = fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY;
        remapSelectScreenButton.SetColors({ 
            makeSafeColors(WORD(fullColorSchemes[g_currentColorScheme].menuColors.col1 | bg)),
            makeSafeColors(WORD(fullColorSchemes[g_currentColorScheme].menuColors.col2 | bg)),
            makeSafeColors(WORD(fullColorSchemes[g_currentColorScheme].menuColors.col4 | bg)),
            WORD(fullColorSchemes[g_currentColorScheme].menuColors.col1 | bg), });

        };
    auto draw_screen = [&]() {
        // get pattern behind map input button
        if (~(g_status & tUI_THEME_af) || !loadedBgFill) {
            for (int i = 0; i < LINES_TO_FILL; i++) {
                getCharsAtPosition(fill1x, fill1y - 1 + i, fill1len, bgFill[i]);
                bgFill[i][fill1len] = L'\0';
            }
            loadedBgFill = true;
        }
        tUI_DRAW_BG_AND_BUTTONS();
        errorOut.Draw();
        output1.Draw();
        if (numJoysticks) {
            setCursorPosition(START_COL + 7, 17);
            std::wcout << " (1";
            if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
            std::wcout << ") Select  ";
        }
        PRINT_EGG_X();
        };

    // populate selectable buttons and add to screen 
    for (int i = 0; i < numJoysticks; ++i)
    {
        // Determine the length of the char string
        size_t charLen = strlen(SDL_GetJoystickNameForID(joystick_list[i]));
        // Allocate memory for the wchar_t string
        wchar_t* wideStr = new wchar_t[charLen + 6]; // +4 for the index string, +1 for a space and +1 null terminator
        // Convert the char string to a wchar_t string // store in message3 as temp location 
        mbstowcs(msgPointer3, SDL_GetJoystickNameForID(joystick_list[i]), charLen + 1);

        // Copy to wideStr with a formatted index
        swprintf(wideStr, L"(%d) %s ", i+1, msgPointer3);

        // create a button with joystick index and name
        availableJoystickBtn[i] = mouseButton(START_COL, START_LINE + i, charLen + 5, wideStr);
        // set the id for selection index
        availableJoystickBtn[i].SetId(i);
        // set callback function to enable selection
        availableJoystickBtn[i].setCallback(&joystickSelectCallback);
        // add the button to the screen
        g_screen.AddButton(&availableJoystickBtn[i]);
    }

    quitButton.SetPosition(10, 17);
    g_screen.AddButton(&quitButton);

    g_status |= RECOL_tUI_f;

    // set this to an invalid index
    g_joystickSelected = -1;

    // scan for mouse and keyboard input
    while (!APP_KILLED && g_joystickSelected < 0) {
        Sleep(30);

        // scan for new connected joysticks and recursively launch function if new joysticks are detected
        int newConnections = 0;
        SDL_free(joystick_list);
        joystick_list = SDL_GetJoysticks(&newConnections);
        if (newConnections != numJoysticks) {
            setErrorMsg(L"\0", 1); // clear errors
            cleanMemory();
            return tUISelectJoystickDialog(joystick);
        }

        // scan screen & keyboard input
        if (IsAppActiveWindow()) {
            screenLoop(g_screen);
            checkForQuit();

            if (getKeyState(0x31) || getKeyState(VK_NUMPAD1))
                g_joystickSelected = 0;
            else if (getKeyState(0x32) || getKeyState(VK_NUMPAD2))
                g_joystickSelected = 1;
            else if (getKeyState(0x33) || getKeyState(VK_NUMPAD3))
                g_joystickSelected = 2;
            else if (getKeyState(0x34) || getKeyState(VK_NUMPAD4))
                g_joystickSelected = 3;
            else if (getKeyState(0x35) || getKeyState(VK_NUMPAD5))
                g_joystickSelected = 4;
            else if (getKeyState(0x36) || getKeyState(VK_NUMPAD6))
                g_joystickSelected = 5;
            else if (getKeyState(0x37) || getKeyState(VK_NUMPAD7))
                g_joystickSelected = 6;
            else if (getKeyState(0x38) || getKeyState(VK_NUMPAD8))
                g_joystickSelected = 7;
            else if (getKeyState(0x39) || getKeyState(VK_NUMPAD9))
                g_joystickSelected = 8;
        }

        tUI_UPDATE_INTERFACE(re_color, draw_screen);
        JOYSENDER_tUI_ANIMATE_FOOTER();

        if (MAPPING_FLAG > 199) {
            int stickIdx = MAPPING_FLAG-200;
            if (ConnectToJoystick(stickIdx, joystick)) {
                BuildJoystickInputData(joystick);

                std::string mapName = encodeStringToHex(joystick.name);
                auto result = check_for_saved_mapping(mapName);
                std::filesystem::path filePath = result.second;
                if (result.first){
                    joystick.mapping.loadMapping(filePath.string());
                }
                JOYSENDER_tUI_BUILD_MAP_SCREEN();
                tUIRemapInputsScreen(joystick);
                //clean up after mapping
                {
                    SDL_CloseJoystick(joystick._ptr);
                    reset_shared_resources();
                    loadedBgFill = false;
                    errorOut.SetText(L"\0"); // error message is bastardized by remap screen and can safely be discarded
                    quitButton.SetPosition(10, 17);
                    //g_screen.AddButton(&quitButton);
                    tUI_AUTO_SET_SUIT_POSITIONS();
                }
                g_status |= RECOL_tUI_f;
            }
            MAPPING_FLAG = 0;
        }

        if (RESTART_FLAG) {
            cleanMemory();
            return 0;
        }

        // if selection has been assigned
        if (g_joystickSelected > -1) {
            // Check if the selected index is invalid
            if (g_joystickSelected < 0 || g_joystickSelected >= numJoysticks)
            {
                setErrorMsg(L" << Invalid Joystick Index >> ", 31);
                errorOut.Draw();
                g_joystickSelected = -1;
            }
        }       
    }

    if (APP_KILLED) {
        cleanMemory();
        return 0;
    }

    // Open the selected joystick
    ConnectToJoystick(g_joystickSelected, joystick);
    if (joystick._ptr == nullptr)
    {
        setErrorMsg(L"Failed to open joystick.", 25);
        errorOut.Draw();
        APP_KILLED = true;
        cleanMemory();
        return 0;
    }

    cleanMemory();

    return 1;
}

int tUISelectDS4Dialog(std::vector<HidDeviceInfo> devList, textUI& screen, int autoSelect) {
    constexpr int START_LINE = 7;
    constexpr int START_COL = 18;
    g_extraData = nullptr; // set to null for joystick select callback

    JOYSENDER_tUI_INIT_FOOTER_ANIMATION();

    {  // adjust mode button
        mouseButton* modeBut = g_screen.GetButtonById(99);
        modeBut->SetText(L" Mode 2 ");
    }

    int numJoysticks = devList.size();
    g_joystickSelected = -1;

    if (!numJoysticks) {
        errorOut.SetPosition(consoleWidth / 2, START_LINE + 1, 50, 0, ALIGN_CENTER);
    }
    else {
        errorOut.SetPosition(consoleWidth / 2, 4, 50, 0, ALIGN_CENTER);
        if (autoSelect) {
            g_joystickSelected = 0;
        }
    }

    std::vector<mouseButton> gamepadButtons;
    auto cleanMemory = [&]() {
        for (std::vector<mouseButton>::iterator it = gamepadButtons.begin(); it != gamepadButtons.end(); ++it) {
            screen.RemoveButton(&(*it));
            delete[] it->getTextPtr();
        }
    };

    auto re_color = [&]() {
        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
        g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        COLOR_EGGS();
        leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        ani.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        };
    auto draw_screen = [=]() {
        /*if (g_status & PTRN_EGG_b) {
            MAKE_PATTERNS();
            setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
            no_whitespace_Draw(JoySendMain_Backdrop, 0, 0, 74, 1555);

            setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
            setCursorPosition(50, 17);
            std::wcout << L"©░Quinnco.░2024";
        }
        else {
            g_screen.DrawBackdrop();
        }
        //g_screen.DrawButtons();
        //errorOut.Draw();
        */
        ////////////////////////////////////////
        tUI_DRAW_BG_AND_BUTTONS();
        errorOut.Draw();
        output1.Draw();
        // show select options in footer
        if (numJoysticks) {
            setCursorPosition(27, 17);
            std::wcout << " (1";
            if (numJoysticks > 1) std::wcout << "-" << numJoysticks;
            std::wcout << ") Select  ";
        }
        PRINT_EGG_X();
        };

    // Populate selectable buttons and add to screen 
    for (int i = 0; i < numJoysticks; ++i)
    {
        std::wstring mfg = devList[i].vendorId == 1356 ? L"Sony" : devList[i].manufacturer.c_str();
        std::wstring prod = devList[i].productId == 2508 ? L"DS4 Controller" : devList[i].product.c_str();

#define BUTTON_NAME_STRING L"(%d) %ls %ls :%ls ", 1 + i, mfg.c_str(), prod.c_str(), devList[i].serial.empty() ? L" Wired" : L" Wireless"


        // Calculate the length of the formatted string
        int bufferSize = swprintf(nullptr, 0, BUTTON_NAME_STRING);

        // Allocate memory for the wchar_t string
        wchar_t* wideStr = new wchar_t[bufferSize + 1]; // +1 for the null terminator

        // Format the string and store it in wideStr
        swprintf(wideStr, bufferSize + 1, BUTTON_NAME_STRING);

        mouseButton btn(START_COL, START_LINE, bufferSize, wideStr);
        btn.SetId(i);
        
        // set callback function to enable selection
        btn.setCallback(&joystickSelectCallback);

        gamepadButtons.push_back(btn);
        screen.AddButton(&gamepadButtons.back());
    }

    quitButton.SetPosition(10, 17);
    screen.AddButton(&quitButton);

    // add messages to screen
    wcsncpy_s(msgPointer1, 29, L" Connected DS4 Controllers: ", _TRUNCATE);
    output1.SetText(msgPointer1);
    output1.SetPosition(15, 5, 50, 1, ALIGN_LEFT);

    // Draw the screen with the available joysticks
    g_status |= RECOL_tUI_f;

    // scan for mouse and keyboard input
    while (!APP_KILLED && g_joystickSelected < 0) {
        if (IsAppActiveWindow()) {
            screenLoop(screen);
            checkForQuit();
            if (APP_KILLED) {
                break;
            }

            if (getKeyState(0x31) || getKeyState(VK_NUMPAD1))
                g_joystickSelected = 0;
            else if (getKeyState(0x32) || getKeyState(VK_NUMPAD2))
                g_joystickSelected = 1;
            else if (getKeyState(0x33) || getKeyState(VK_NUMPAD3))
                g_joystickSelected = 2;
            else if (getKeyState(0x34) || getKeyState(VK_NUMPAD4))
                g_joystickSelected = 3;
            else if (getKeyState(0x35) || getKeyState(VK_NUMPAD5))
                g_joystickSelected = 4;
            else if (getKeyState(0x36) || getKeyState(VK_NUMPAD6))
                g_joystickSelected = 5;
            else if (getKeyState(0x37) || getKeyState(VK_NUMPAD7))
                g_joystickSelected = 6;
            else if (getKeyState(0x38) || getKeyState(VK_NUMPAD8))
                g_joystickSelected = 7;
            else if (getKeyState(0x39) || getKeyState(VK_NUMPAD9))
                g_joystickSelected = 8;
        }

        if (RESTART_FLAG) {
            cleanMemory();
            return 0;
        }

        // if selection has been assigned
        if (g_joystickSelected > -1) {
            // Check if the selected index is valid
            if (g_joystickSelected < 0 || g_joystickSelected >= numJoysticks) {
                setErrorMsg(L" << Invalid Controller Index >> ", 33);
                errorOut.Draw();
                g_joystickSelected = -1;
            }
        }

        // scan for new connected joysticks and recursively launch function if new joysticks are detected
        auto newConnections = getDS4ControllersList();
        if (newConnections.size() != numJoysticks) {
            if (newConnections.size()) {
                setErrorMsg(L"\0", 1); // clear errors
            }
            cleanMemory();
            return tUISelectDS4Dialog(newConnections, screen, autoSelect);
        }


        tUI_UPDATE_INTERFACE(re_color, draw_screen);
        JOYSENDER_tUI_ANIMATE_FOOTER();

        Sleep(30);
    }

    if (APP_KILLED) {
        cleanMemory();
        return -1;
    }

    // Open the selected joystick
    if (!uiConnectToDS4Controller(&devList[g_joystickSelected]))
    {
        setErrorMsg(L"Failed to open joystick.", 25);
        errorOut.Draw();
        APP_KILLED = true;
        cleanMemory();
        return 0;
    }

    cleanMemory();

    return 1;
}

std::string tUIGetHostAddress(SDLJoystickData& activeGamepad) {
    int octNum = 0;
    bool validIP = 0;
    bool firstDot = 0;
    bool makeConnection = false;
    bool warningShown = 0;
    bool errorShown = 0;
    std::string host_address;

    // for drawing back covered up bg patterns
    bool loadedBgFill = false;
    constexpr int fill1len = 50, fill1x = 36-fill1len/2, fill1y = 9;
    constexpr int fill2len = 25, fill2x = (consoleWidth / 2) - fill2len/2, fill2y = 13;
    wchar_t* refill_1 = msgPointer3;
    wchar_t* refill_2 = refill_1 + fill1len+1;

    JOYSENDER_tUI_INIT_FOOTER_ANIMATION();
    DEFAULT_EGG_BUTTS();
    tUI_THEME_BUTTONS();
    if (g_status & BORDER_EGG_a) {
            HEARTS_N_SPADES_BUTTONS();
    }

    g_previousIPData.index = 0;

    //set Colors values
    WORD& bg_color = fullColorSchemes[g_currentColorScheme].menuBg;
    tUIColorPkg screenButtonsCol = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);

    COORD octPos[4] = {
        {29, 10},
        {33, 10},
        {37, 10},
        {41, 10}
    };

    textInput octet[4] = {  //int x, int y, int w, int maxLength
        textInput(29, 10, 3, 3, ALIGN_CENTER, screenButtonsCol.col1),
        textInput(33, 10, 3, 3, ALIGN_CENTER, screenButtonsCol.col1),
        textInput(37, 10, 3, 3, ALIGN_CENTER, screenButtonsCol.col1),
        textInput(41, 10, 3, 3, ALIGN_CENTER, screenButtonsCol.col1)
    };

    // Lambda for setting octet cursor position
    auto setOctetCursorPos = [&]() {
        setCursorPosition(octPos[octNum].X + (octet[octNum].getCursorPosition() > 0) * 1, octPos[octNum].Y);
    };
    // Lambda function for IP address validation
    auto validIPAddress = [](const std::string& ipAddress) {
        if (ipAddress == "0.0.0.0" || ipAddress == "255.255.255.255") {
            return false;
        }
        std::regex pattern(R"(^((25[0-5]|(2[0-4]|1\d|[1-9]|)\d)\.?\b){4}$)"); // better pattern but matches all 0s and broadcast addresses
        
        std::smatch match;
        return std::regex_match(ipAddress, match, pattern);
    };
    // Lambda for building host address
    auto buildAndTestIpAddress = [&]() {
        // Construct the IP address string
        std::wstring ipAddress = octet[0].getText();
        for (int i = 1; i < 4; ++i) {
            ipAddress += L"." + std::wstring(octet[i].getText());
        }
        host_address = g_converter.to_bytes(ipAddress);

        if (validIPAddress(host_address)) {
            if (loadedBgFill) {
                setCursorPosition(fill1x, fill1y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_1;
            }
            else {
                errorOut.Clear(bg_color);
            }

            errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);
            setErrorMsg(L" Valid IP ! ", 38);
            errorOut.Draw();
            output1.Draw();
            errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
            setOctetCursorPos();
            errorShown = true;
            return 1;
        }
        else if (errorShown)
        {
            errorShown = false;
            if (loadedBgFill) {
                setCursorPosition(fill1x, fill1y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_1;

                setCursorPosition(fill2x, fill2y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_2;
            }
            else {
                errorOut.Clear(bg_color);
                output1.Clear(bg_color);
            }
            setOctetCursorPos();
        }
        return 0;
    };

    // Lambda for loading previous ip address
    auto loadPreviousIP = [&]() {
        // load ip into octets
        int dotCount = 0;

        octet[dotCount].Clear();
        octet[dotCount].clearBuffer();
        //octet[dotCount].cursorToBegin();

        wchar_t* charArray = g_previousIPData.address[g_previousIPData.index];
        for (size_t i = 0; i < 16; i++) {
            if (charArray[i] == '\0') {
                if(!i){
                    octet[i].insert(L'0');
                    octet[i].cursorToBegin();
                    for (int j = 1; j < 4; ++j) {
                        octet[j].Clear();
                        octet[j].clearBuffer();
                        octet[j].insert(L'0');
                        octet[j].cursorToBegin();
                        octet[j].Draw();
                    }
                }
                i = 16;
            }
            else if (charArray[i] == '.') {
                //octet[dotCount].insert('\0');
                octet[dotCount].Draw();
                dotCount++;
                octet[dotCount].Clear();
                octet[dotCount].clearBuffer();
                octet[dotCount].cursorToBegin();
            }
            else {
                octet[dotCount].insert(charArray[i]);
                // need to check for end of array \0
            }
        }
        octet[dotCount].Draw();

        // increment index
        if (++g_previousIPData.index > IPS_TO_REMEMBER-1) {
            g_previousIPData.index = 0;
        }
      
        return dotCount;
    };


    for (int i = 0; i < 4; ++i) {
        octet[i].insert(L'0');
        octet[i].cursorToBegin();
        g_screen.AddInput(&octet[i]);
    }

    quitButton.SetPosition(10, 17);
    g_screen.AddButton(&quitButton);


    auto re_color = [&]() {
        loadedBgFill = false;
        bg_color = fullColorSchemes[g_currentColorScheme].menuBg;
        screenButtonsCol = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
        //tUI_SET_BACKDROP_COLOR(0);
        g_screen.SetBackdropColor(bg_color);
        g_screen.SetButtonsColors(screenButtonsCol);
        g_screen.SetInputsColors(screenButtonsCol);
        output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); // Press Enter To Connect
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        COLOR_EGGS();
        leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        ani.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);        
        };
    auto draw_screen = [&]() {
        /*
        if (g_status & PTRN_EGG_b) {
            MAKE_PATTERNS();
            setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
            no_whitespace_Draw(JoySendMain_Backdrop, 0, 0, 74, 1555);

            setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
            setCursorPosition(50, 17);
            std::wcout << L"©░Quinnco.░2024";

            //if (!loadedBgFill) { // if pattern is drawn we need a new copy of it because they dont often repeat
                getCharsAtPosition(fill1x, fill1y, fill1len, refill_1);
                refill_1[fill1len] = L'\0';
                getCharsAtPosition(fill2x, fill2y, fill2len, refill_2);
                refill_2[fill2len] = L'\0';
                loadedBgFill = true;
            //}

        }
        else {
            g_screen.DrawBackdrop();
        }
        */
        tUI_DRAW_BG_AND_BUTTONS();
        if (g_status & PTRN_EGG_b) {
            getCharsAtPosition(fill1x, fill1y, fill1len, refill_1);
            refill_1[fill1len] = L'\0';
            getCharsAtPosition(fill2x, fill2y, fill2len, refill_2);
            refill_2[fill2len] = L'\0';
            loadedBgFill = true;
        }
        else {
            g_screen.DrawBackdrop();
        }
        PRINT_EGG_X();
        setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
        printCurrentController(activeGamepad);
        //g_screen.DrawButtons();
        //errorOut.Draw(); // errors not shown on this screen 
        g_screen.DrawInputs();
        setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
        setCursorPosition(12, 8);
        std::wcout << L" Enter IP Address Of Host: ";

        setTextColor(screenButtonsCol.col1);
        setCursorPosition(28, 10);
        std::wcout << L"   .   .   .   ";
        g_screen.DrawInputs();
        
        setCursorPosition(29, 10);
        };


    // set colors and draw
    g_status |= RECOL_tUI_f;

    errorOut.SetPosition(consoleWidth / 2, 9, 50, 1, ALIGN_CENTER);
    output1.SetText(L" Press Enter to Connect ");
    output1.SetPosition(consoleWidth / 2, 13, 25, 1, ALIGN_CENTER);
           
    //setCursorPosition(29, 10);
    showConsoleCursor();
    
    while (!APP_KILLED && !makeConnection) {
        checkForQuit();
        screenLoop(g_screen);

        tUI_UPDATE_INTERFACE(re_color, draw_screen);
        JOYSENDER_tUI_ANIMATE_FOOTER();

        for (int i = 0; i < 4; ++i) {
            if (octet[i].Status() & ACTIVE_INPUT) {
                validIP = buildAndTestIpAddress();
                octNum = i;        
            }
        }
        setOctetCursorPos();

        char num = IpInputLoop();
        // B: backspace, D: delete, N: next octet, .: dot, X: nothing, ^: recall recent ip
        if (num == '^') {
            octNum = loadPreviousIP();
            validIP = buildAndTestIpAddress();

            if (octNum) {
                octet[octNum].cursorToEnd();
            }
            else {
                octet[octNum].back();
            }
            setOctetCursorPos();
        }
        else if (num == '.') {
            if (!octet[octNum].getCursorPosition() && !firstDot) {
                firstDot = 1;
            }
            else {
                num = 'N';
            }
        }
        else if (num == 'B') {
            errorShown = false;

            if (loadedBgFill) {
                setCursorPosition(fill1x, fill1y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_1;

                setCursorPosition(fill2x, fill2y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_2;
            }
            else {
                errorOut.Clear(bg_color);
                output1.Clear(bg_color);
            }

            if (octet[octNum].getCursorPosition() == 0) {
                if (!octet[octNum].getLength()) {
                    octet[octNum].insert(L'0');
                    octet[octNum].Draw();
                }
                octNum = std::max(0, octNum - 1);
                setOctetCursorPos();
            }
            
            octet[octNum].Clear();
            octet[octNum].back();
            octet[octNum].Draw();         

            validIP = buildAndTestIpAddress();
        }
        else if (num == 'D') {
            octet[octNum].Clear();
            octet[octNum].del();
            octet[octNum].Draw();

            validIP = buildAndTestIpAddress();
        }
        else if (octNum < 4 && num != 'N' && num != 'X') {
            octet[octNum].overwrite(num);
            octet[octNum].Draw();

            validIP = buildAndTestIpAddress();
        }
        if (num == 'N' || octet[octNum].getCursorPosition() > 2) {
            if (!octet[octNum].getLength()) {
                octet[octNum].insert(L'0');
                octet[octNum].Draw();
            }
                    
            firstDot = 0;
            octNum = std::min(octNum+1,4);
            octet[octNum].cursorToBegin();
            setOctetCursorPos();
        }
                
        if (validIP) {
            if (checkKey(VK_RETURN, IS_PRESSED) && IsAppActiveWindow()) {
                makeConnection = true;
            }
        }
        else if (octNum == 4 && !errorShown && !buildAndTestIpAddress()) {
                validIP = false;   
                setErrorMsg(L" Invalid IP Address ", 21);
                errorOut.Draw();
                errorShown = true;
                octNum--;
                setOctetCursorPos();       
        }
        else if (getKeyState(VK_RETURN) && IsAppActiveWindow()) {
            warningShown = true;
            setErrorMsg(L" Invalid IP Address ", 21);
            errorOut.Draw();
            setOctetCursorPos();
        }
        else if (warningShown && !getKeyState(VK_RETURN)) {
            warningShown = false;
            if (loadedBgFill) {
                setCursorPosition(fill1x, fill1y);
                setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                std::wcout << refill_1;
            }
            else {
                errorOut.Clear(bg_color);
            }
            //errorOut.Clear(fullColorSchemes[g_currentColorScheme].menuBg);
            errorShown = false;
            setOctetCursorPos();
        }

        if (num == 'X')
            Sleep(20);
    }
    
    hideConsoleCursor();
    g_screen.ClearInputs();
    g_screen.ClearButtonsExcept(HEAP_BTN_IDs);
    buildAndTestIpAddress();

    
    if (pushNewIP(g_converter.from_bytes(host_address).c_str())) {
        saveIPDataToFile();
    }

    return host_address;
}

int tUIRemapInputsScreen(SDLJoystickData& joystick) {
    textUI screen;
    //appendWindowTitle(g_hWnd, "Remap Buttons!"));
    std::vector<SDLButtonMapping::ButtonName> inputList = std::vector<SDLButtonMapping::ButtonName>();
    static bool changes = false;

    // Convert joystick name to hex mapfile name   
    std::string mapName = encodeStringToHex(joystick.name);
    // Get file path for a mapfile
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;
    bool mapExists = result.first;

    constexpr int MAX_INPUTS_SHOWN = 12;

    // move common buttons
    {
        mouseButton* btn = g_screen.GetButtonById(46);
        if (btn != nullptr) {
            btn->SetPosition(4, 1);
            btn = g_screen.GetButtonById(47);
            btn->SetPosition(consoleWidth - 4, 1);
        }
    }

    // for drawing back covered up bg patterns
    bool loadedBgFill = false;
    constexpr int fill1len = 15, fill1x = 1, fill1y = 3;
    constexpr int fill2len = 14, fill2x = (consoleWidth - fill2len), fill2y = 3;
    constexpr int fill3len = 26, fill3x = 23, fill3y = consoleHeight-5;
    wchar_t leftFill[fill1len+1];
    wchar_t rightFill[MAX_INPUTS_SHOWN][fill2len+1];
    wchar_t bottomFill[fill3len+1];

    // Set up screen elements
    // ****************************

    // add eggs from g_screen
    auto load_eggs = [&]()
        {
            mouseButton* btn = g_screen.GetButtonById(42);
            if (btn != nullptr)
                screen.AddButton(btn);
            btn = g_screen.GetButtonById(41);
            if (btn != nullptr)
                screen.AddButton(btn);
            btn = g_screen.GetButtonById(45);
            if (btn != nullptr)
                screen.AddButton(btn);
            btn = g_screen.GetButtonById(46);
            if (btn != nullptr)
                screen.AddButton(btn);
            btn = g_screen.GetButtonById(47);
            if (btn != nullptr)
                screen.AddButton(btn);
            tUI_SET_SUIT_POSITIONS(SUIT_POSITIONS_MAP_STACKED());
        };
    //custom egg
    mouseButton uiEgg(34, 4, 6, L"(-Q-)"); 
    uiEgg.setCallback([](mouseButton& btn) { 
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT); 
            if (!(g_status & BORDER_EGG_a | tUI_LOADED_f)) {                        
                MORPH_BORDER();   
                //MAKE_PATTERNS(); 
                g_status |= PTRN_EGG_b; 
                HEARTS_N_SPADES_BUTTONS(); 
                tUI_THEME_BUTTONS();
                g_status |= MISC_FLAG_f; // using this to signal we need to add the eggs to map screen

                // move common buttons for remap screen
                mouseButton* btn = g_screen.GetButtonById(46);
                if (btn != nullptr) {
                    btn->SetPosition(4, 1);
                    btn = g_screen.GetButtonById(47);
                    btn->SetPosition(consoleWidth - 4, 1);
                }

                g_status |= RECOL_tUI_f; 
            } 
        } 
        }); 
    screen.AddButton(&uiEgg);

    screen.SetBackdrop(XBOX_Backdrop);
    textUI otherButtons;
    mouseButton dummyBtn; // used to spoof a callback button click
    dummyBtn.SetStatus(MOUSE_UP);

    // set screen title to msgPointer1
    const int maxTitleLength = consoleWidth - 8;
    //   message + prefix + null terminator
    int titleLength = 20 + (mapExists ? 3 : 0) + 1;
    swprintf(msgPointer1, titleLength, L" %sMapping Inputs For: ", mapExists ? L"Re-" : L"");

    wchar_t* gamepadName_ptr = msgPointer1 + titleLength-1;
    // convert joystick.name to wchar_t* and store in gamepadName_ptr pointer
    mbstowcs(msgPointer3, joystick.name.c_str(), joystick.name.size());
    swprintf(gamepadName_ptr, joystick.name.size()+2, L" %s ", msgPointer3);

    // create a separate text area for the joystick name // for color change possibilities
    textBox gamepadName(consoleWidth/2 - (titleLength+joystick.name.size())/2 + titleLength-1, 1, joystick.name.size()+3, 1, ALIGN_LEFT, gamepadName_ptr, DEFAULT_TEXT);

    titleLength += joystick.name.size();

    // Set screen title position and text
    output1.SetPosition(consoleWidth / 2, 1, maxTitleLength, 1, ALIGN_CENTER);
    output1.SetText(msgPointer1);

    textBox& screenTitle = output1;

    // Set up a current mapped Input area
    textBox& currentInputTitle = output2;
    currentInputTitle.SetPosition(9, 2, 17, 1, ALIGN_CENTER);
    swprintf(msgPointer2, 17, L" Input Mapping: "); // title
    currentInputTitle.SetText(msgPointer2);

    // an area to update with the hovered button input value
    textBox currentInputMap = output2;
    wchar_t* currentInputText = msgPointer2 + 16;
    currentInputMap.SetPosition(9, 3, 15, 1, ALIGN_CENTER);

    // set global g_extraData to hoveredButton for access by callback function
    int hoveredButton = -1;
    g_extraData = static_cast<void*>(&hoveredButton);

    // Set up a Detected Input area
    output3.SetPosition(63, 2, 18, 1, ALIGN_CENTER);
    swprintf(msgPointer3, 18, L" Detected Input: "); // title
    output3.SetText(msgPointer3);

    textBox& detectedInputTitle = output3;

    textBox detectedInput = output3;
    detectedInput.SetPosition(CONSOLE_WIDTH -1, 3, 14, MAX_INPUTS_SHOWN, ALIGN_RIGHT);

    wchar_t* detectedInputs = msgPointer3 + 16;
    swprintf(detectedInputs, 9, L" (NONE) "); // data display    
    detectedInput.SetText(detectedInputs);

    // Set up and add buttons to screen
    {
        textUI buttonSetter;
        AddControllerButtons(buttonSetter);
        buttonSetter.SetButtonsCallback(mappingControllerButtonsCallback);
    }
    AddControllerButtons(screen);
    if (g_status & BORDER_EGG_a | tUI_LOADED_f) {
        load_eggs();
    }
    errorOut.SetText(L"\0");
    quitButton.SetPosition(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE + 1);
    screen.AddButton(&quitButton);
    otherButtons.AddButton(&quitButton);

    // Map All button
    mouseButton mapAllButton(CONSOLE_WIDTH / 2 - 8, XBOX_QUIT_LINE - 2, 17, L" (A) All Inputs  ");
    mapAllButton.setCallback(mappingAllButtonsCallback);
    screen.AddButton(&mapAllButton);
    otherButtons.AddButton(&mapAllButton);

    // Done button
    mouseButton doneMappingButton(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE, 11, L" (D) Done  ");
    doneMappingButton.setCallback(mappingDoneButtonCallback);
    screen.AddButton(&doneMappingButton);
    otherButtons.AddButton(&doneMappingButton);

    // Save and Cancel Buttons
    mouseButton cancelButton(CONSOLE_WIDTH / 2 + 7, XBOX_QUIT_LINE, 13, L" (C) Cancel  ");
    mouseButton saveMapButton(CONSOLE_WIDTH / 2 - 17, XBOX_QUIT_LINE, 11, L" (S) Save  ");
    screen.AddButton(&cancelButton);
    otherButtons.AddButton(&cancelButton);
    screen.AddButton(&saveMapButton);
    otherButtons.AddButton(&saveMapButton);

    // Save conformation message
    textBox saveMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 3, 24, 1, ALIGN_CENTER, L" Mapping Saved! ", fullColorSchemes[g_currentColorScheme].menuColors.col3);
    // create Click To Map footer highlight area
    textBox footerMsg(CONSOLE_WIDTH / 2, XBOX_QUIT_LINE - 1, 24, 1, ALIGN_CENTER, L" Click Button to Remap ", DEFAULT_TEXT);
    // create a # characters to redraw a portion of the controller outline
    mouseButton outlineRedraw(57, 6, 8, L"##\t\t\t\t\t\t\t##\t\t\t\t\t\t\t##\t\t\t\t\t\t\t##\t\t\t\t\t\t\t#&\t\t\t\t\t\t\t#\\\t\t\t\t\t\t\t#\t\t\t\t\t\t\t\t#\t\t\t\t\t\t\t#");
    // create a space character to redraw a portion of the controller face
    mouseButton faceRedraw(58, 8, 6, L" \t\t\t\t\t  \t\t\t\t   \t\t\t    \t\t     \t            ");
    // *********
    
    // this should provide the ability to make saveMsg message disappear
    FPSCounter timer;
    bool timing = false;
    auto vanishingMessage = [&]() {
        // get_elapsed time returns a double representing seconds
        if (timing && timer.get_elapsed_time() > 3) {
            timing = false;
            if (g_status & PTRN_EGG_b) {
                    setCursorPosition(fill3x, fill3y);
                    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                    std::wcout << bottomFill;
            }
            else {
                saveMsg.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
            }
        }

    };
    
    auto re_color = [&]() {
        screen.SetBackdropColor(DIAMOND_COLOR());
        screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);

        currentInputTitle.SetColor(BRIGHT_BLUE);
        currentInputMap.SetColor(YELLOW);
        output3.SetColor(BRIGHT_RED | BLUE AS_BG);        
        outlineRedraw.SetDefaultColor(fullColorSchemes[g_currentColorScheme].controllerBg);
        faceRedraw.SetDefaultColor(fullColorSchemes[g_currentColorScheme].controllerColors.col1);
        
        tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
        screenTitle.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);
        gamepadName.SetColor(buttonColors.col1);
        footerMsg.SetColor(buttonColors.col1);
        otherButtons.SetButtonsColors(buttonColors);

        COLOR_EGGS();
        uiEgg.SetColors({ fullColorSchemes[g_currentColorScheme].controllerBg,
            fullColorSchemes[g_currentColorScheme].controllerBg,
            (WORD)(fullColorSchemes[g_currentColorScheme].controllerColors.col2 FG_ONLY 
                | fullColorSchemes[g_currentColorScheme].controllerBg BG_ONLY), 
            0x0000 });

        loadedBgFill = false;
        };
    auto draw_screen = [&]() {
        if (g_status & PTRN_EGG_b) {
            setCursorPosition(0, 0);
            setTextColor(DIAMOND_COLOR());
            std::wcout << JoySendMain_Backdrop;
            tUI_SET_BG_AND_SHOULDER_BUTTONS();

            if (~(g_status & tUI_THEME_af) || !loadedBgFill) {
                getCharsAtPosition(fill1x, fill1y, fill1len, leftFill);
                leftFill[fill1len] = L'\0';
                for (int i = 0; i < MAX_INPUTS_SHOWN; i++) {
                    getCharsAtPosition(fill2x, fill2y+i, fill2len, rightFill[i]);
                    rightFill[i][fill2len] = L'\0';
                }
                getCharsAtPosition(fill3x, fill3y, fill3len, bottomFill);
                bottomFill[fill3len] = L'\0';
                loadedBgFill = true;
            }
        }
        else if (g_status & BORDER_EGG_a) {
            loadedBgFill = false;
            setCursorPosition(0, 0);
            setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
            std::wcout << JoySendMain_Backdrop;
            tUI_CLEAR_PTRN_AREA(fullColorSchemes[g_currentColorScheme].controllerBg);
        }
        else {
            //screen.DrawBackdrop();
        }

        tUI_DRAW_CONTROLLER_FACE();
        /*
        if (g_status & PTRN_EGG_b) {
            ReDrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].menuBg, 1, (g_status & PTRN_EGG_b));
        }
        else if (g_status & BORDER_EGG_a) {
            ReDrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, 1, (g_status & BORDER_EGG_a));
        }
        else {
            ReDrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, 1 );
        }
        */
        otherButtons.DrawButtons();
        screenTitle.Draw();
        gamepadName.Draw();
        currentInputTitle.Draw();
        detectedInputTitle.Draw();
        detectedInput.Draw();
        footerMsg.Draw();
        screen.DrawButtons();
        errorOut.Draw();
        PRINT_EGG_X();
        };

    re_color();
    draw_screen();

    XUSB_REPORT dummyReport = { 0 };
    std::vector<SDLButtonMapping::ButtonMapInput> activeInputs, lastInputs;
    int inputPtr = 0;
    int lastHovered = -1;
    int inputsListed = 0;
    int mouseState = 0;

    // Logic Loop
    while (!APP_KILLED && MAPPING_FLAG) {

        if (theme_mtx.try_lock()) {
            theme_mtx.unlock();

            vanishingMessage();

            // get activeInputs to identify noisy Inputs // temporary?
            activeInputs = get_sdljoystick_input_list(joystick);

            // update detected input area and controller buttons
            if (lastInputs != activeInputs) {
                // Create dummy report for controller buttons
                get_xbox_report_from_activeInputs(joystick, activeInputs, dummyReport);
                buttonStatesFromXboxReport(dummyReport);

                // clear active input area
                if (g_status & PTRN_EGG_b) {
                    for (int i = 0; i < inputsListed; i++) {
                        setCursorPosition(fill2x, fill2y + i);
                        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                        std::wcout << rightFill[i];
                    }

                }
                else if (g_status & BORDER_EGG_a) {
                    detectedInput.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
                    for (int i = 0; i < inputsListed; i++) {
                        setCursorPosition(consoleWidth - 4, fill2y + i);
                        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                        wprintf(L"%.5ls", &JoySendMain_Backdrop[(consoleWidth + 2) * (fill2y + 1 + i) - 6]); ;
                    }
                }
                else {
                    detectedInput.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
                }

                detectedInput.SetText(L" (NONE) ");
                if (inputsListed > 3) { // after four inputs controller could get drawn over
                    outlineRedraw.Draw(); // redraw controller edge
                    faceRedraw.Draw();  // and face
                }


                inputPtr = 0;
                // write active inputs text data to memory
                int i = 0;
                inputsListed = i;
                for (auto input : activeInputs) {
                    inputsListed = i + 1;
                    // get formatted string
                    auto txt = SDLButtonMapping::displayInput(input);
                    // add formatted input to wchar_t pointer
                    swprintf(detectedInputs + inputPtr, txt.size() + 4, L"%s %S ", i ? L"\n" : L"", txt.c_str());
                    // update inputPtr
                    inputPtr += txt.size() + 2 + (i > 0);

                    if (++i == MAX_INPUTS_SHOWN) // limit inputs displayed on screen // also prevents buffer overflow
                        break;
                }

                // display active inputs on screen
                if (activeInputs.size()) {
                    // update display
                    detectedInput.SetText(detectedInputs);
                }

                detectedInput.Draw();
                lastInputs = activeInputs;
            }

            mouseState = screenLoop(screen); // a button callback, called through screenLoop, will set hoveredButton

            if (theme_mtx.try_lock()) {
                theme_mtx.unlock();
                // display current mapping for hovered button
                if (hoveredButton != lastHovered) {
                    if (g_status & PTRN_EGG_b) {
                        setCursorPosition(fill1x, fill1y);
                        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                        std::wcout << leftFill;
                    }
                    else if (g_status & BORDER_EGG_a) {
                        currentInputMap.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
                        setCursorPosition(2, fill1y);
                        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
                        wprintf(L"%.3ls", &JoySendMain_Backdrop[((consoleWidth + 2) * fill1y) + 2]);
                    }
                    else {
                        currentInputMap.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
                    }

                    if (hoveredButton > -1) {

                        auto txt = SDLButtonMapping::displayInput(
                            joystick.mapping.buttonMaps[static_cast<SDLButtonMapping::ButtonName>(hoveredButton - (hoveredButton > MAP_BUTTON_CLICKED - 1) * MAP_BUTTON_CLICKED)]
                        );
                        swprintf(currentInputText, txt.size() + 3, L" %S  ", txt.c_str());
                        currentInputMap.SetText(currentInputText);
                        currentInputMap.Draw();
                    }
                }
                lastHovered = hoveredButton;

                // Check for screen Hot Keys
                if (getKeyState(VK_SHIFT) && IsAppActiveWindow()) {
                    // Map All
                    if (checkKey('A', IS_RELEASED)) {
                        dummyBtn.SetStatus(MOUSE_UP);
                        mappingAllButtonsCallback(dummyBtn);
                    }
                    // Done
                    else if (checkKey('D', IS_RELEASED)) {
                        dummyBtn.SetStatus(MOUSE_UP);
                        mappingDoneButtonCallback(dummyBtn);
                        if (changes && OLDMAP_FLAG) {
                            OLDMAP_FLAG == false;
                        }
                    }
                    // Cancel
                    else if (checkKey('C', IS_RELEASED)) {
                        cancelButton.SetStatus(MOUSE_UP);
                        mouseState = MOUSE_UP;
                    }
                    // Save
                    else if (checkKey('S', IS_RELEASED)) {
                        saveMapButton.SetStatus(MOUSE_UP);
                        mouseState = MOUSE_UP;
                    }
                    // Quit
                    else if (checkKey('Q', IS_RELEASED)) {
                        dummyBtn.SetStatus(MOUSE_UP);
                        exitAppCallback(dummyBtn);
                    }
                }
                else {
                    // still check all keys to keep state current
                    checkKey('A', IS_RELEASED);
                    checkKey('D', IS_RELEASED);
                    checkKey('C', IS_RELEASED);
                    checkKey('S', IS_RELEASED);
                    checkKey('Q', IS_RELEASED);
                }

                // Save & Cancel buttons
                if (mouseState == MOUSE_UP) {
                    if (saveMapButton.Status() & MOUSE_UP)
                    {
                        if (changes) {
                            joystick.mapping.saveMapping(filePath.string());
                            saveMsg.SetText(L" Mapping Saved! ");
                            if (OLDMAP_FLAG) {
                                OLDMAP_FLAG == false;
                            }
                        }
                        else {
                            saveMsg.SetText(L" Nothing has Changed! ");
                        }
                        changes = false;
                        saveMsg.Draw();
                        timer.reset_timer();
                        timing = true;

                        saveMapButton.SetStatus(MOUSE_OUT);
                        saveMapButton.Update();
                    }
                    else if (cancelButton.Status() & MOUSE_UP)
                    {
                        if (changes && mapExists) {
                            joystick.mapping.loadMapping(filePath.string());
                        }
                        cancelButton.SetStatus(MOUSE_OUT);

                        mouseButton dummyBtn;
                        dummyBtn.SetStatus(MOUSE_UP);
                        mappingDoneButtonCallback(dummyBtn);
                    }
                }

                // Map the clicked button
                if (MAPPING_FLAG && mouseState == MOUSE_UP && hoveredButton > MAP_BUTTON_CLICKED - 1) {
                    saveMapButton.Clear();
                    cancelButton.Clear();

                    changes = true;
                    std::vector<SDLButtonMapping::ButtonName> inputList;
                    inputList.push_back(static_cast<SDLButtonMapping::ButtonName>(hoveredButton - MAP_BUTTON_CLICKED));

                    if (tUIMapTheseInputs(joystick, inputList)) {
                        // will return non zero if canceled
                        changes = false;
                    }

                    // re draw everything
                    draw_screen();
                }

                // Map all inputs
                if (MAPPING_FLAG == 2) {
                    saveMapButton.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);
                    cancelButton.Clear(fullColorSchemes[g_currentColorScheme].controllerBg);

                    changes = true;
                    std::vector<SDLButtonMapping::ButtonName> inputList;
                    // Create a list of all inputs
                    inputList.insert(inputList.end(), joystick.mapping.stickButtonNames.begin(), joystick.mapping.stickButtonNames.end());
                    inputList.insert(inputList.end(), joystick.mapping.shoulderButtonNames.begin(), joystick.mapping.shoulderButtonNames.end());
                    inputList.insert(inputList.end(), joystick.mapping.triggerButtonNames.begin(), joystick.mapping.triggerButtonNames.end());
                    inputList.insert(inputList.end(), joystick.mapping.thumbButtonNames.begin(), joystick.mapping.thumbButtonNames.end());
                    inputList.insert(inputList.end(), joystick.mapping.dpadButtonNames.begin(), joystick.mapping.dpadButtonNames.end());
                    inputList.insert(inputList.end(), joystick.mapping.genericButtonNames.begin(), joystick.mapping.genericButtonNames.end());

                    if (tUIMapTheseInputs(joystick, inputList)) {
                        // will return non zero if canceled
                        if (mapExists) {
                            // undo any changes by reloading saved map
                            joystick.mapping.loadMapping(filePath.string());
                            changes = false;
                        }
                    }
                    // clear input already assigned message if it was set
                    errorOut.SetText(L"\0");

                    // turn down the mapping flag to indicate not to map all inputs again
                    MAPPING_FLAG = 1;

                    // reset hovered button state **avoids memory fault??
                    lastHovered = hoveredButton = -1;

                    // re draw everything
                    /*
                    ReDrawControllerFace(screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, 1);
                    screen.DrawButtons();
                    otherButtons.DrawButtons();

                    screenTitle.Draw();
                    gamepadName.Draw();

                    currentInputTitle.Draw();
                    detectedInputTitle.Draw();
                    footerMsg.Draw();  // click to map message bottom
                    */
                    draw_screen();


                    // if no saved mapping exists auto save current mapping
                    if (!mapExists && !APP_KILLED) {
                        mapExists = joystick.mapping.saveMapping(filePath.string());
                        if (mapExists) {
                            changes = false;
                            saveMsg.Draw();
                            timer.reset_timer();
                            timing = true;
                        }
                    }
                }

                if (g_status & MISC_FLAG_f) {
                    g_status &= ~MISC_FLAG_f;
                    load_eggs();
                }
                tUI_UPDATE_INTERFACE(re_color, draw_screen);
            }
        }
        Sleep(20);
    }

    if (APP_KILLED) {
        return 0;
    }

    // clean up spill over border text
    {
        WORD col = fullColorSchemes[g_currentColorScheme].menuBg;
        currentInputTitle.Clear(col);
        currentInputMap.Clear(col);
        detectedInputTitle.Clear(col);
        detectedInput.Clear(col);
    }
    // move common buttons back
    {
        mouseButton* btn = g_screen.GetButtonById(46);
        if (btn != nullptr) {
            btn->SetPosition(3, 2);
            btn = g_screen.GetButtonById(47);
            btn->SetPosition(consoleWidth - 3, 2);
        }
    }
    return 1;
}

int tUIMapTheseInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>& inputList) {

    //
    // Set up UI elements
    const int OVERLAY_START_COL = 24;
    const int OVERLAY_START_LINE = 13;
    textUI mapOverlay;

    tUIColorPkg UIcolors(
        fullColorSchemes[g_currentColorScheme].menuBg,       // body
        (fullColorSchemes[g_currentColorScheme].menuBg FG_ONLY) | (fullColorSchemes[g_currentColorScheme].controllerBg BG_ONLY), //outline
        makeSafeColors( button_Guide_highlight.getSelectColor() ),        // input id & topmsg
        (fullColorSchemes[g_currentColorScheme].menuBg FG_ONLY) | (button_Guide_highlight.getDefaultColor() BG_ONLY)  // outlineTop
    );

    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);

    //errorOut.SetPosition(consoleWidth/2 - 14, 6);

    mouseButton cancelButton(CONSOLE_WIDTH / 2 - 6, XBOX_QUIT_LINE, 20, L" (C) Cancel  ");
    mouseButton skipButton(CONSOLE_WIDTH / 2 - 9, XBOX_QUIT_LINE - 1, 20, L" (Esc) Skip Input  ");
    quitButton.SetPosition(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE + 1);

    mapOverlay.AddButton(&cancelButton);
    mapOverlay.AddButton(&skipButton);
    mapOverlay.AddButton(&quitButton);

    mapOverlay.SetButtonsColors(buttonColors);

    // make this overlay out of mouse buttons because they support the '\t' special character to advance the cursor
    mouseButton MapScreen_Overlay_outlineTop(OVERLAY_START_COL, OVERLAY_START_LINE, 25, L"\t\t\t___________________\t\t\t");
    mouseButton MapScreen_Overlay_outline(OVERLAY_START_COL, OVERLAY_START_LINE + 1, 25, L"\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t//\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\\\{\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t}");
    mouseButton MapScreen_Overlay_body(OVERLAY_START_COL, OVERLAY_START_LINE + 1, 25, L"\t\t\t   Set Input For   \t\t\t\t\t                     \t\t\t                       \t");
    mouseButton MapScreen_Overlay_bodyBottom(OVERLAY_START_COL, OVERLAY_START_LINE + 4, 25, L"                                                  ");
    MapScreen_Overlay_outlineTop.SetDefaultColor(UIcolors.col4);
    MapScreen_Overlay_outline.SetDefaultColor(UIcolors.col2);
    MapScreen_Overlay_body.SetDefaultColor(UIcolors.col1);
    MapScreen_Overlay_bodyBottom.SetDefaultColor(fullColorSchemes[g_currentColorScheme].controllerBg);

    // areas for inputToMap name to be displayed
    textInput inputID(CONSOLE_WIDTH / 2, OVERLAY_START_LINE + 2, 25, 25, ALIGN_CENTER, UIcolors.col3);
    textInput topMsg(CONSOLE_WIDTH / 2, 5, 30, 30, ALIGN_CENTER, UIcolors.col3);

    // Draw the Overlay
    MapScreen_Overlay_outlineTop.Draw();
    MapScreen_Overlay_outline.Draw();
    MapScreen_Overlay_body.Draw();
    MapScreen_Overlay_bodyBottom.Draw();
    mapOverlay.DrawButtons();

    // Create a map to ensure no input is used twice
    std::unordered_map<std::string, bool> receivedInput;

    // Map all inputs in inputList
    for (const auto& inputToMap : inputList) {
        if (APP_KILLED) return 0;
        std::string inputName = SDLButtonMapping::getButtonNameString(inputToMap);
        SDLButtonMapping::ButtonType inputType = get_input_type(inputName);

        // Update current input name
        g_outputText = " " + input_verb(inputType) + " " + format_input_name(inputName) + " ";
        // Directly write to topMsg wchar_t array //
        mbstowcs(topMsg.getText(), g_outputText.c_str(), g_outputText.size() + 1);
        topMsg.SetText(topMsg.getText());

        g_outputText = " " + format_input_name(inputName) + " ";
        // Directly write to inputID wchar_t array //
        mbstowcs(inputID.getText(), g_outputText.c_str(), g_outputText.size() + 1);
        inputID.SetText(inputID.getText());

        topMsg.Draw();
        inputID.Draw();

        // draw the button as if it were selected
        mouseButton* buttonToMap = buttonIdMap[inputToMap];

        buttonToMap->SetStatus(MOUSE_DOWN);
        buttonToMap->Update();


        SDLButtonMapping::ButtonMapInput received_input;
        bool settingInput = true;

        while (settingInput && !APP_KILLED) {

            screenLoop(mapOverlay);

            // Check for screen Hot Keys
            if (IsAppActiveWindow()) {
                // Cancel
                if ( (getKeyState(VK_SHIFT) && checkKey('C', IS_RELEASED)) || cancelButton.Status() & MOUSE_UP) {
                    cancelButton.SetStatus(MOUSE_OUT);
                    MAPPING_FLAG = 1;

                    buttonToMap->SetStatus(MOUSE_OUT);
                    buttonToMap->Update();

                    topMsg.Clear(UIcolors.col1);
                    inputID.Clear(UIcolors.col1);

                    return -1;
                }
                // Skip
                else if (checkKey(VK_ESCAPE, IS_RELEASED) || skipButton.Status() & MOUSE_UP ) {
                    skipButton.SetStatus(MOUSE_OUT);
                    settingInput = false;
                }
                // Quit
                checkForQuit();
            }
            else {
                // still check all keys to keep state current
                checkKey('C', IS_RELEASED);
                checkKey(VK_ESCAPE, IS_RELEASED);
            }

            if (received_input.input_type == SDLButtonMapping::ButtonType::UNSET) {
                // Receive an input signature
                get_sdljoystick_mapping_input(joystick, received_input);
            }

            if (received_input.input_type != SDLButtonMapping::ButtonType::UNSET) {
                // Create a key from the input
                if ((inputType == SDLButtonMapping::ButtonType::BUTTON ||
                    inputType == SDLButtonMapping::ButtonType::SHOULDER ||
                    inputType == SDLButtonMapping::ButtonType::HAT ||
                    inputType == SDLButtonMapping::ButtonType::THUMB)
                    && received_input.input_type == SDLButtonMapping::ButtonType::STICK) {
                    if ((received_input.value == ANALOG_RANGE_NEG_TO_POS)
                        || (received_input.value == ANALOG_RANGE_POS_TO_NEG)) {
                        received_input.range = received_input.value;
                        received_input.value = (received_input.range == ANALOG_RANGE_NEG_TO_POS) ? -1 : 1;
                    }
                }

                std::string inputKey = std::to_string(static_cast<int>(received_input.input_type)) + "_" + std::to_string(received_input.index) + "_" + std::to_string(received_input.value);

                if (receivedInput.find(inputKey) == receivedInput.end()) {
                    receivedInput[inputKey] = true;
                    settingInput = false;
                    while (there_is_sdljoystick_input(joystick) && !APP_KILLED) {
                        Sleep(20);
                    }
                }
                else {
                    received_input = SDLButtonMapping::ButtonMapInput();
                    setErrorMsg( L"<< Input already assigned! >>", 30 );
                    errorOut.Draw();
                    while (there_is_sdljoystick_input(joystick) && !APP_KILLED) {
                        Sleep(20);
                    }
                    errorOut.Clear(fullColorSchemes[g_currentColorScheme].controllerColors.col1);
                }
            }
            else {
                Sleep(20);
            }
        }

        if (!settingInput) {
            // Map received input to inputID
            joystick.mapping.buttonMaps[inputToMap] = received_input;

            buttonToMap->SetStatus(MOUSE_OUT);
            buttonToMap->Update();

            topMsg.Clear(fullColorSchemes[g_currentColorScheme].controllerColors.col1);
            inputID.Clear(UIcolors.col1);
        }
    }

    // ********************  
    joystick.mapping.populateExtraMaps();

    return 0;
}



// ********************************
//joySendertUI() Helpers

#define JOYSENDER_tUI_CX_HANDSHAKE(){ \
std::string txSettings = std::to_string(args.fps) + ":" + std::to_string(args.mode); \
allGood = client.send_data(txSettings.c_str(), static_cast<int>(txSettings.length())); \
if (allGood < 1) { \
    swprintf(errorPointer, 48, L" << Connection To %S Failed >> ", args.host.c_str()); \
    errorOut.SetText(errorPointer); \
    break; \
} \
bytesReceived = client.receive_data(buffer, buffer_size); \
if (bytesReceived > 0) { \
    inConnection = true; \
    failed_connections = 0; \
} \
else { \
    swprintf(errorPointer, 46, L" << Connection To: %S Failed >> ", args.host.c_str()); \
    errorOut.SetText(errorPointer); \
    break; \
}} 

void JOYSENDER_tUI_INIT_UI() {
    // Set Up for first tUI screen
    // global flags set by ui and callback functions
    OLDMAP_FLAG = 0;
    RESTART_FLAG = 0;
    MAPPING_FLAG = 0;

    // set element properties
    CoupleControllerButtons(); // sets up controller outline/highlight coupling for nice looks & button ids
    g_screen.SetBackdrop(JoySendMain_Backdrop);
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    output1.SetPosition(14, 5, 50, 1, ALIGN_LEFT); // output1 is a textarea used for g_screen headers/titles
    errorOut.SetPosition(consoleWidth / 2, 4, 50, 0, ALIGN_CENTER); // used for error messaging
    //setErrorMsg(L"\0", 1);
    fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT);                // fps output
    quitButton.setCallback(&exitAppCallback);
    newColorsButton.setCallback(&newControllerColorsCallback);
    quitButton.SetPosition(10, 17);

}

int JOYSENDER_tUI_SELECT_JOYSTICK(SDLJoystickData& activeGamepad, Arguments& args, int& allGood) {
    
    tUI_LOAD_THEME();
    tUI_THEME_BUTTONS();

    mouseButton* applyTheme = g_screen.GetButtonById(46);
    mouseButton* saveTheme = g_screen.GetButtonById(47);
    mouseButton colorEgg(50, consoleHeight - 3, 1, L"©");
    mouseButton borderEgg(21, 3, 3, L"tUI");
    {
        colorEgg.SetId(43);
        g_screen.AddButton(&colorEgg);
        colorEgg.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {

                btn.SetStatus(MOUSE_OUT);
                g_status |= COLOR_EGG_b;
            }
            });

        borderEgg.SetId(44);
        borderEgg.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                if (!(g_status & BORDER_EGG_a)) {

                    MORPH_BORDER();
                    if (generateRandomInt(1, 5) > 4) {

                        MAKE_PATTERNS();
                        g_status |= PTRN_EGG_b;
                    }
                    HEARTS_N_SPADES_BUTTONS();
                    g_status |= RECOL_tUI_f | REDRAW_tUI_f;
                }
            }
            });
        g_screen.AddButton(&borderEgg);
    }

    tUI_THEME_AUTO_ACTIVATION();
    tUI_THEME_RESTART();

    g_screen.DrawBackdrop(); // for clear whitespace updates we need to laydown some color

    mouseButton changeModeButton(55, 3, 9, L" Mode 1 ");
    changeModeButton.SetId(99);
    changeModeButton.setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            RESTART_FLAG = (g_mode == 2) ? 2 : 3;
            g_screen.RemoveButtonById(99);
        }
        });
    g_screen.AddButton(&changeModeButton);

    switch (args.mode) {
    case 1: {   // SDL MODE
        if (!args.select) { // not auto select
            allGood = tUISelectJoystickDialog(activeGamepad);
            if (!allGood) {
                if (RESTART_FLAG) return 0;
                SDL_Quit();
                return 0;
            }
        }
        else {
            allGood = ConnectToJoystick(0, activeGamepad); //select the first gamepad: index 0;
            if (!allGood) {
                g_screen.DrawBackdrop();
                errorOut.SetPosition(consoleWidth / 2, 7);
                setErrorMsg(L" Unable to connect to a device !! ", 35);
                errorOut.Draw();

                SDL_Quit();
                return 0;
            }
        }
    }
          break;
    case 2: {   // DS4 MODE
        g_screen.DrawBackdrop();
        allGood = tUISelectDS4Dialog(getDS4ControllersList(), g_screen, args.select);
        if (!allGood) {
            if (RESTART_FLAG) return 0;
            g_screen.DrawBackdrop();
            errorOut.SetPosition(consoleWidth / 2, 7);
            setErrorMsg(L" Unable to connect to a DS4 device !! ", 39);
            errorOut.Draw();
            return 0;
        }
    }
          break;
    default:
        g_screen.DrawBackdrop();
        errorOut.SetPosition(consoleWidth / 2, 7);
        setErrorMsg(L" ERROR:: Unsupported Mode !! ", 30);
        errorOut.Draw();
        return 0;
    }
    return 1;
}

void JOYSENDER_tUI_OPMODE_INIT(SDLJoystickData& activeGamepad, Arguments& args, int& allGood) {
    if (args.mode == 2) {

        // Get a report from the device to determine what type of connection it has
        Sleep(5); // a fresh report should be generated every 4ms from the ds4 device
        GetDS4Report();

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
        uiOpenOrCreateMapping(activeGamepad, g_screen);

        if (g_outputText.find(OLDMAP_WARNING_MSG) != std::string::npos) {
            OLDMAP_FLAG = true;
        }
    }

}

void JOYSENDER_tUI_ANIMATED_CX(SDLJoystickData& activeGamepad,TCPConnection& client, Arguments& args, int& allGood) {
    g_screen.ClearButtonsExcept(HEAP_BTN_IDs);
    JOYSENDER_tUI_INIT_FOOTER_ANIMATION();
    DEFAULT_EGG_BUTTS();
    tUI_THEME_BUTTONS();
    if (g_status & BORDER_EGG_a) {
        HEARTS_N_SPADES_BUTTONS();
    }

    auto re_color = [&]() {
        g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
        g_screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        
        output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1);    // title/heading text
        errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
        output2.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4);

        COLOR_EGGS();
        leftBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        rightBorderPiece.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        ani.SetDefaultColor(fullColorSchemes[g_currentColorScheme].menuBg);
        };
    auto draw_screen = [&]() {

        tUI_DRAW_BG_AND_BUTTONS();
        setTextColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
        printCurrentController(activeGamepad);
        errorOut.Draw();
        output1.Draw();
        PRINT_EGG_X();
        };

    // Setup Connecting Screen
    errorOut.SetPosition(consoleWidth / 2, 7, 50, 0, ALIGN_CENTER);
    mouseButton cancelButton(CONSOLE_WIDTH / 2 - 7, 17, 13, L" (C) Cancel  ");
    g_screen.AddButton(&cancelButton);
    quitButton.SetPosition(10, 17);
    g_screen.AddButton(&quitButton);

    // setup connecting animation
    output2.SetPosition(consoleWidth / 2, 10, 38, 0, ALIGN_CENTER);
    // set up connecting message
    int len = args.host.size() + 18;
    swprintf(msgPointer1, len, L" Connecting To: %s ", std::wstring(args.host.begin(), args.host.end()).c_str());
    output1.SetPosition(consoleWidth / 2, 9, len, 1, ALIGN_CENTER);
    output1.SetText(msgPointer1);

    // Draw Screen
    g_status |= RECOL_tUI_f | REDRAW_tUI_f;


    // ### establish the connection in separate thread to animate connecting dialog
    allGood = WSAEWOULDBLOCK;
    std::thread connectThread(threadedEstablishConnection, std::ref(client), std::ref(allGood));
    // animation and input loop
    int lastFrame = -1;
    while (!APP_KILLED && allGood == WSAEWOULDBLOCK) {
        if (lastFrame != g_frameNum) {
            output2.SetText(ConnectAnimation[g_frameNum]);
            output2.Draw();
            lastFrame = g_frameNum;
        }

        int mouseState = screenLoop(g_screen);

        // check for hot keys
        if (getKeyState(VK_SHIFT) && IsAppActiveWindow()) {
            if (getKeyState('Q')) {
                APP_KILLED = TRUE;
            }
            else if (getKeyState('C')) {
                mouseState = MOUSE_UP;
                cancelButton.SetStatus(MOUSE_UP);
            }
        }

        // check Cancel button
        if (mouseState == MOUSE_UP) {
            if (cancelButton.Status() & MOUSE_UP)
            {
                cancelButton.SetStatus(MOUSE_OUT);

                // stop connection thread
                allGood = CANCELLED_FLAG;
                break;
            }
        }

        if (frameDelay % 5 == 0) {
            countUpDown(g_frameNum, CX_ANI_FRAME_COUNT);  // bounce
            //loopCount(g_frameNum, CX_ANI_FRAME_COUNT);    // loop
        }

        if (!APP_KILLED && allGood == WSAEWOULDBLOCK)
            Sleep(20);
        if (APP_KILLED || allGood != WSAEWOULDBLOCK)
            break; // if connection can happen within 20ms don't bother drawing the screen

        tUI_UPDATE_INTERFACE(re_color, draw_screen);
        JOYSENDER_tUI_ANIMATE_FOOTER();
    }
    // clean up
    connectThread.detach();
    g_screen.ClearButtonsExcept(HEAP_BTN_IDs);
    // ###
}

void JOYSENDER_tUI_RESET_AFTER_MAP(Arguments& args) {
    /*
    // reset output1
    wcsncpy_s(msgPointer1, 40, g_converter.from_bytes(" << Connected to: " + args.host + "  >> ").c_str(), _TRUNCATE);
    output1.SetText(msgPointer1);
    output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
    output1.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);
    // output 3 & fps   
    output3.SetText(L" FPS:");
    output3.SetColor(fullColorSchemes[g_currentColorScheme].controllerBg);

    if (g_status & BORDER_EGG_a) {
        output1.SetPosition(6, 1, 43, 1, ALIGN_LEFT);
        cxMsg.SetPosition(23, 1, 38, 1, ALIGN_LEFT);
        output3.SetPosition(55, 1, 5, 1, ALIGN_LEFT);
        fpsMsg.SetPosition(60, 1, 7, 1, ALIGN_LEFT);
    }
    else {
        output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
        cxMsg.SetPosition(20, 1, 38, 1, ALIGN_LEFT);
        output3.SetPosition(46, 1, 5, 1, ALIGN_LEFT);
        fpsMsg.SetPosition(51, 1, 7, 1, ALIGN_LEFT);
    }

    // re add g_screen buttons
    g_screen.AddButton(&newColorsButton);
    g_screen.AddButton(&mappingButton);
    g_screen.AddButton(&restartButton[1]);  // mode 1
    g_screen.AddButton(&restartButton[2]);  // mode 2
    g_screen.AddButton(&restartButton[0]);  // main restart
    g_screen.AddButton(&quitButton);

    // reset quit button
    quitButton.setCallback(&exitAppCallback);
    quitButton.SetPosition(consoleWidth / 2 - 5, XBOX_QUIT_LINE);

    */

    ///////////////////////////////////////////////

    quitButton.SetPosition(consoleWidth / 2 - 5, XBOX_QUIT_LINE);
    newColorsButton.SetPosition(consoleWidth / 2 - 8, XBOX_QUIT_LINE - 3);

    restartButton[0].SetPosition(CONSOLE_WIDTH / 2 - 12, XBOX_QUIT_LINE - 1);
    restartButton[1].SetPosition(CONSOLE_WIDTH / 2 - 8, XBOX_QUIT_LINE - 1);
    restartButton[2].SetPosition(CONSOLE_WIDTH / 2 - 6, XBOX_QUIT_LINE - 1);
    restartButton[3].SetPosition(CONSOLE_WIDTH / 2 + 6, XBOX_QUIT_LINE - 1);

    swprintf(msgPointer1, 40, L" << Connection To: %S  >> ", args.host.c_str());
    swprintf(cxPointer, 18, L" %S ", args.host.c_str());
    cxMsg.SetPosition(21, 1, 38, 1, ALIGN_LEFT);
    cxMsg.SetText(cxPointer);

    //SetControllerButtonPositions(g_mode);

    output1.SetText(msgPointer1);
    output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
    output3.SetPosition(46, 1, 5, 1, ALIGN_LEFT);
    output3.SetText(L" FPS:");
        

    if ((g_status & BORDER_EGG_a) || (g_status & tUI_LOADED_f)) {
        tUI_THEME_BUTTONS();
        HEARTS_N_SPADES_BUTTONS();
        tUI_SET_WIDE_MSG_LAYOUT(args.host.c_str());
    }
    else {
        g_screen.DeleteButton(46); // apply theme
        g_screen.DeleteButton(47); // save theme
    }
    tUI_AUTO_SET_SUIT_POSITIONS();


    //////////////////////////////////////////


    g_status |= RECOL_tUI_f;
    MAPPING_FLAG = 0;
}

void JOYSENDER_tUI_BUILD_MAP_SCREEN() {
    int QUITLINE;
    if (g_mode == 2) {
        g_screen.SetBackdrop(DS4_Backdrop);
        QUITLINE = DS4_QUIT_LINE;
        BuildDS4Face();
    }
    else {
        g_screen.SetBackdrop(XBOX_Backdrop);
        QUITLINE = XBOX_QUIT_LINE;
        BuildXboxFace();
    }
    errorOut.SetPosition(consoleWidth / 2, 6, 50, 0, ALIGN_CENTER);
    SetControllerButtonPositions(g_mode);
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);
    SetControllerFace(g_screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, g_mode);

    controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
    tUI_SET_SUIT_POSITIONS(SUIT_POSITIONS_MAP_STACKED());
}