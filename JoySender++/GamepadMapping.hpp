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
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <SDL/SDL.h>


class SDLButtonMapping {
public:
    enum class ButtonType {
        HAT,
        STICK,
        THUMB,
        TRIGGER,
        SHOULDER,
        BUTTON,
        UNSET
    };

    enum class ButtonName {
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        START,
        BACK,
        LEFT_THUMB,
        RIGHT_THUMB,
        LEFT_SHOULDER,
        RIGHT_SHOULDER,
        GUIDE,
        A,
        B,
        X,
        Y,
        LEFT_TRIGGER,
        RIGHT_TRIGGER,
        LEFT_STICK_LEFT,
        LEFT_STICK_UP,
        LEFT_STICK_RIGHT,
        LEFT_STICK_DOWN,
        RIGHT_STICK_LEFT,
        RIGHT_STICK_UP,
        RIGHT_STICK_RIGHT,
        RIGHT_STICK_DOWN,
    };

    class ButtonMapInput {
    public:
        ButtonType input_type;
        int index;
        int value;

        ButtonMapInput(ButtonType input_type = ButtonType::UNSET, int index = -1, int value = 0)
            : input_type(input_type), index(index), value(value) {}

        void set(ButtonType input_type, int index, int value) {
            this->input_type = input_type;
            this->index = index;
            this->value = value;
        }

        void clear() {
            input_type = ButtonType::UNSET;
            index = -1;
            value = 0;
        }
    };

    using ButtonMap = std::unordered_map<ButtonName, ButtonMapInput>;

    ButtonMap buttonMaps;
    std::vector<ButtonName> stickButtonNames;
    std::vector<ButtonName> triggerButtonNames;
    std::vector<ButtonName> thumbButtonNames;
    std::vector<ButtonName> shoulderButtonNames;
    std::vector<ButtonName> dpadButtonNames;
    std::vector<ButtonName> genericButtonNames;

    SDLButtonMapping()
        : buttonMaps({{ButtonName::DPAD_UP, ButtonMapInput()},
                      {ButtonName::DPAD_DOWN, ButtonMapInput()},
                      {ButtonName::DPAD_LEFT, ButtonMapInput()},
                      {ButtonName::DPAD_RIGHT, ButtonMapInput()},
                      {ButtonName::START, ButtonMapInput()},
                      {ButtonName::BACK, ButtonMapInput()},
                      {ButtonName::LEFT_THUMB, ButtonMapInput()},
                      {ButtonName::RIGHT_THUMB, ButtonMapInput()},
                      {ButtonName::LEFT_SHOULDER, ButtonMapInput()},
                      {ButtonName::RIGHT_SHOULDER, ButtonMapInput()},
                      //{ButtonName::GUIDE, ButtonMapInput()}, // useless and annoying button on pc
                      {ButtonName::A, ButtonMapInput()},
                      {ButtonName::B, ButtonMapInput()},
                      {ButtonName::X, ButtonMapInput()},
                      {ButtonName::Y, ButtonMapInput()},
                      {ButtonName::LEFT_TRIGGER, ButtonMapInput()},
                      {ButtonName::RIGHT_TRIGGER, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_LEFT, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_UP, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_RIGHT, ButtonMapInput()},
                      {ButtonName::LEFT_STICK_DOWN, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_LEFT, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_UP, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_RIGHT, ButtonMapInput()},
                      {ButtonName::RIGHT_STICK_DOWN, ButtonMapInput()}}),
        stickButtonNames(generateButtonList(ButtonType::STICK)),
        triggerButtonNames(generateButtonList(ButtonType::TRIGGER)),
        thumbButtonNames(generateButtonList(ButtonType::THUMB)),
        shoulderButtonNames(generateButtonList(ButtonType::SHOULDER)),
        dpadButtonNames(generateButtonList(ButtonType::HAT)),
        genericButtonNames(generateGenericButtonList()) {}

    std::vector<ButtonName> getSetButtonNames() const {
        std::vector<ButtonName> setButtons;
        for (const auto& buttonMap : buttonMaps) {
            if (buttonMap.second.input_type != ButtonType::UNSET) {
                setButtons.push_back(buttonMap.first);
            }
        }
        return setButtons;
    }

    std::vector<ButtonName> getUnsetButtonNames() const {
        std::vector<ButtonName> unsetButtons;
        for (const auto& buttonMap : buttonMaps) {
            if (buttonMap.second.input_type == ButtonType::UNSET) {
                unsetButtons.push_back(buttonMap.first);
            }
        }
        return unsetButtons;
    }

    int saveMapping(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            std::vector<std::tuple<ButtonName, ButtonType, int, int>> buttonList;
            for (const auto& buttonMap : buttonMaps) {
                const auto& buttonName = buttonMap.first;
                const auto& buttonInput = buttonMap.second;
                buttonList.emplace_back(buttonName, buttonInput.input_type, buttonInput.index, buttonInput.value);
            }

            const std::size_t tupleSize = sizeof(std::tuple<ButtonName, ButtonType, int, int>);
            const std::size_t dataSize = buttonList.size() * tupleSize;

            file.write(reinterpret_cast<const char*>(buttonList.data()), static_cast<std::streamsize>(dataSize));
            file.close();
            return 1; // Successfully saved button maps
        }
        else {
            return -1; // Error: Failed to open file
        }
    }

    int loadMapping(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            std::size_t fileSize = static_cast<std::size_t>(file.tellg());
            file.seekg(0, std::ios::beg);

            if (fileSize % sizeof(std::tuple<ButtonName, ButtonType, int, int>) != 0) {
                file.close();
                return -2; // Error: Invalid file size
            }

            std::vector<std::tuple<ButtonName, ButtonType, int, int>> buttonList(fileSize / sizeof(std::tuple<ButtonName, ButtonType, int, int>));
            file.read(reinterpret_cast<char*>(buttonList.data()), static_cast<std::streamsize>(fileSize));

            buttonMaps.clear();
            for (const auto& buttonTuple : buttonList) {
                const auto& buttonName = std::get<0>(buttonTuple);
                const auto& buttonInputType = std::get<1>(buttonTuple);
                const auto& buttonIndex = std::get<2>(buttonTuple);
                const auto& buttonValue = std::get<3>(buttonTuple);

                ButtonMapInput buttonInput(buttonInputType, buttonIndex, buttonValue);
                buttonMaps[buttonName] = buttonInput;
            }

            file.close();
            return 1; // Successfully loaded button maps
        }
        else {
            return -1; // Error: Failed to open file
        }
    }

    static std::string getButtonNameString(ButtonName buttonName) {
        switch (buttonName) {
        case ButtonName::DPAD_UP: return "DPAD_UP";
        case ButtonName::DPAD_DOWN: return "DPAD_DOWN";
        case ButtonName::DPAD_LEFT: return "DPAD_LEFT";
        case ButtonName::DPAD_RIGHT: return "DPAD_RIGHT";
        case ButtonName::START: return "START";
        case ButtonName::BACK: return "BACK";
        case ButtonName::LEFT_THUMB: return "LEFT_THUMB";
        case ButtonName::RIGHT_THUMB: return "RIGHT_THUMB";
        case ButtonName::LEFT_SHOULDER: return "LEFT_SHOULDER";
        case ButtonName::RIGHT_SHOULDER: return "RIGHT_SHOULDER";
        case ButtonName::GUIDE: return "GUIDE";
        case ButtonName::A: return "A";
        case ButtonName::B: return "B";
        case ButtonName::X: return "X";
        case ButtonName::Y: return "Y";
        case ButtonName::LEFT_TRIGGER: return "LEFT_TRIGGER";
        case ButtonName::RIGHT_TRIGGER: return "RIGHT_TRIGGER";
        case ButtonName::LEFT_STICK_LEFT: return "LEFT_STICK_LEFT";
        case ButtonName::LEFT_STICK_UP: return "LEFT_STICK_UP";
        case ButtonName::LEFT_STICK_RIGHT: return "LEFT_STICK_RIGHT";
        case ButtonName::LEFT_STICK_DOWN: return "LEFT_STICK_DOWN";
        case ButtonName::RIGHT_STICK_LEFT: return "RIGHT_STICK_LEFT";
        case ButtonName::RIGHT_STICK_UP: return "RIGHT_STICK_UP";
        case ButtonName::RIGHT_STICK_RIGHT: return "RIGHT_STICK_RIGHT";
        case ButtonName::RIGHT_STICK_DOWN: return "RIGHT_STICK_DOWN";

        default: return "UNKNOWN";
        }
    }

    static std::string getButtonTypeString(ButtonType buttonType) {
        switch (buttonType) {
        case ButtonType::HAT: return "DPAD";
        case ButtonType::STICK: return "STICK";
        case ButtonType::THUMB: return "THUMB";
        case ButtonType::TRIGGER: return "TRIGGER";
        case ButtonType::BUTTON: return "BUTTON";
        case ButtonType::SHOULDER: return "SHOULDER";
        case ButtonType::UNSET: return "UNSET";
        default: return "UNKNOWN";
        }
    }

    static std::string getInputValueString(ButtonType buttonType, int value) {
        switch (buttonType) {
        case ButtonType::HAT: {
            std::ostringstream oss;
            oss << ' ' << std::showbase << std::internal << std::setfill('0') << std::setw(4) << std::hex << value;
            return oss.str();
        }

        case ButtonType::STICK: {
            if (value > 0) return "+";
            return "-";
        }
        }
        return "";
    }

    std::string displayButtonMaps() {
        std::string output;
        for (const auto& buttonMap : buttonMaps) {
            const auto& buttonName = buttonMap.first;
            const auto& buttonInput = buttonMap.second;
            output += "Name: " + getButtonNameString(buttonName) + ", Input Type: " + getButtonTypeString(buttonInput.input_type) +
                ", Index: " + std::to_string(buttonInput.index) +
                ", Value: " + std::to_string(buttonInput.value) + "\r\n";
        }
        return output;
    }

    std::string displayInput(ButtonName buttonName) {
        if (buttonMaps.count(buttonName) > 0) {
            const auto& buttonInput = buttonMaps.at(buttonName);
            return "Name: " + getButtonNameString(buttonName) + ", Input Type: " + getButtonTypeString(buttonInput.input_type) +
                ", Index: " + std::to_string(buttonInput.index) +
                ", Value: " + std::to_string(buttonInput.value);
        }
        else {
            return "Button not found: " + getButtonNameString(buttonName);
        }
    }

private:
    
    std::vector<ButtonName> generateButtonList(ButtonType buttonType) const {

            auto compareByName = [this](ButtonName id1, ButtonName id2) {
                return getButtonNameString(id1) < getButtonNameString(id2);
            };

        std::vector<ButtonName> buttonList;
        for (const auto& buttonMap : buttonMaps) {
            const auto& buttonID = buttonMap.first;
            const std::string buttonName = getButtonNameString(buttonID);
            const auto& buttonInput = buttonMap.second;
            const auto bType = getButtonTypeString(buttonType);
            if (buttonName.find(bType) != std::string::npos) {
                buttonList.push_back(buttonID);
            }
        }
        if (buttonType != ButtonType::HAT) {
            // Sort the buttonList vector alphabetically
            std::sort(buttonList.begin(), buttonList.end());
        }
        return buttonList;
    }

    std::vector<ButtonName> generateGenericButtonList() const {
        // Sort the genericButtons vector by length using the custom comparator
        auto compareByNameLength = [this](ButtonName id1, ButtonName id2) {
            std::string name1 = getButtonNameString(id1);
            std::string name2 = getButtonNameString(id2);
            return name1.length() < name2.length();
        };

        std::vector<ButtonName> genericButtons;
        for (const auto& buttonMap : buttonMaps) {
            const auto& buttonID = buttonMap.first;
            const std::string buttonName = getButtonNameString(buttonID);
            const auto& buttonInput = buttonMap.second;
            if (buttonName.find("DPAD") == std::string::npos &&
                buttonName.find("SHOULDER") == std::string::npos &&
                buttonName.find("THUMB") == std::string::npos &&
                buttonName.find("TRIGGER") == std::string::npos &&
                buttonName.find("STICK") == std::string::npos) {
                genericButtons.push_back(buttonID);
            }
        }
        std::sort(genericButtons.begin(), genericButtons.end(), compareByNameLength);
        return genericButtons;
    }
};

struct SDLJoystickData {
    SDL_Joystick* _ptr = nullptr;
    std::string name = "";
    int num_axes = 0;
    int num_buttons = 0;
    int num_hats = 0;
    SDLButtonMapping mapping;
    std::vector<int> avgBaseline;
};

// **********************************
// These should probably be private member functions
// use API functions at bottom
SDLButtonMapping::ButtonType get_input_type(const std::string& s) {
    std::string lowercaseString = s;
    std::transform(lowercaseString.begin(), lowercaseString.end(), lowercaseString.begin(), ::tolower);

    if (lowercaseString.find("dpad") != std::string::npos) {
        return SDLButtonMapping::ButtonType::HAT;
    }
    else if (lowercaseString.find("trigger") != std::string::npos) {
        return SDLButtonMapping::ButtonType::TRIGGER;
    }
    else if (lowercaseString.find("stick") != std::string::npos) {
        return SDLButtonMapping::ButtonType::STICK;
    }
    else if (lowercaseString.find("button") != std::string::npos) {
        return SDLButtonMapping::ButtonType::BUTTON;
    }
    else {
        return SDLButtonMapping::ButtonType::UNSET;
    }
}

bool haveSameSign(int num1, int num2) {
    return (num1 >= 0 && num2 >= 0) || (num1 < 0 && num2 < 0);
}

BYTE ShortToByte(SHORT value)
{
    // Scale the absolute value to fit within the range of a BYTE (0 to 255)
    DOUBLE scaleFactor = 255.0 / SHRT_MAX;
    DOUBLE scaledValue = scaleFactor * value;

    // Round the scaled value to the nearest integer
    BYTE byteValue = static_cast<BYTE>(std::round(scaledValue));

    return byteValue;
}

int calculateMode(const std::vector<int>& data) {
    std::unordered_map<int, int> freqDict;
    int mode = 0;
    int maxFreq = 0;
    for (int x : data) {
        freqDict[x]++;
        if (freqDict[x] > maxFreq) {
            mode = x;
            maxFreq = freqDict[x];
        }
    }
    return mode;
}

std::string encodeStringToHex(const std::string& s) {
    std::stringstream encoded;
    for (char c : s) {
        encoded << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(c));
    }
    return encoded.str();
}

std::string input_verb(SDLButtonMapping::ButtonType type) {
    if (type == SDLButtonMapping::ButtonType::TRIGGER) {
        return "squeeze";
    }
    else if (type == SDLButtonMapping::ButtonType::STICK) {
        return "move";
    }
    else {
        return "press";
    }
}

std::string format_input_name(const std::string& name) {
    std::string formattedName = name;
    std::replace(formattedName.begin(), formattedName.end(), '_', ' ');
    if (formattedName.find(' ') == std::string::npos) {
        formattedName += " Button";
    }
    std::string finalName;
    std::istringstream iss(formattedName);
    std::string word;
    while (iss >> word) {
        if (!finalName.empty()) {
            finalName += " ";
        }
        finalName += std::string(1, std::toupper(word[0])) + word.substr(1);
    }
    return finalName;
}

std::pair<bool, std::filesystem::path> check_for_saved_mapping(const std::string& _filename) {
    std::string appdataFolder = g_getenv("APPDATA");
    appdataFolder += "\\" + std::string(APP_NAME);

    std::filesystem::path saveFolder = std::filesystem::path(appdataFolder);
    std::filesystem::path filename = saveFolder / (_filename + ".map");

    if (!std::filesystem::exists(saveFolder)) {
        if (!std::filesystem::create_directories(saveFolder)) {
            return { false, filename };
        }
        return { false, filename };
    }

    if (std::filesystem::is_regular_file(filename)) {
        return { true, filename };
    }
    else {
        return { false, filename };
    }
}

std::pair< std::tuple<int, int, int>, std::tuple<std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>>>
get_sdl_joystick_baseline(SDL_Joystick* joystick, int numSamples = 512) {
    int numAxes = SDL_JoystickNumAxes(joystick);
    int numButtons = SDL_JoystickNumButtons(joystick);
    int numHats = SDL_JoystickNumHats(joystick);

    // Setup sample buffer for all inputs
    std::vector<std::vector<int>> sampleBuf(numSamples, std::vector<int>(numHats + numButtons + numAxes));
    std::vector<int> avgBuffer(numHats + numButtons + numAxes, 0);
    std::vector<int> avgReport(numHats + numButtons + numAxes, 0);
    std::vector<int> medianReport(numHats + numButtons + numAxes, 0);
    std::vector<int> modeReport(numHats + numButtons + numAxes, 0);
    std::vector<int> rangeReport(numHats + numButtons + numAxes, 0);

    // Collect data from samples
    for (int i = 0; i < numSamples; i++) {
        SDL_JoystickUpdate();
        for (int j = 0; j < numHats + numButtons + numAxes; j++) {
            int value = 0;
            if (j < numAxes) {
                value = SDL_JoystickGetAxis(joystick, j);
            }
            else if (j < numAxes + numButtons) {
                value = SDL_JoystickGetButton(joystick, j - numAxes);
            }
            else {
                value = SDL_JoystickGetHat(joystick, j - numAxes - numButtons);
            }
            avgBuffer[j] += value;
            sampleBuf[i][j] = value;
        }
    }

    // Calculate median, mode, range, and average for each input
    for (int j = 0; j < numHats + numButtons + numAxes; j++) {
        std::vector<int> data;
        for (int i = 0; i < numSamples; i++) {
            data.push_back(sampleBuf[i][j]);
        }
        std::sort(data.begin(), data.end());
        medianReport[j] = data[numSamples / 2];
        modeReport[j] = calculateMode(data);
        rangeReport[j] = data[numSamples - 1] - data[0];
        avgReport[j] = avgBuffer[j] / numSamples;
    }

    return std::make_pair(std::make_tuple(numAxes, numButtons, numHats), std::make_tuple(avgReport, medianReport, modeReport, rangeReport));
}

SDLButtonMapping::ButtonMapInput get_sdljoystick_input(const SDLJoystickData& joystick) {
    SDLButtonMapping::ButtonMapInput input;

    while (!APP_KILLED) {
        // Get the joystick state
        SDL_JoystickUpdate();

        if (getKeyState(VK_ESCAPE)) {
            while (getKeyState(VK_ESCAPE)) {
                // Esc key pressed, wait for release
                Sleep(10);
            }
            // Esc key released, return empty input
            return input;
        }

        // Iterate over all joystick axes
        for (int i = 0; i < joystick.num_axes; i++) {
            int axis_value = SDL_JoystickGetAxis(joystick._ptr, i); // / 32767.0f;
            if (std::abs(axis_value - joystick.avgBaseline[i]) > 16000) {
                input.set(SDLButtonMapping::ButtonType::STICK, i, axis_value < 0 ? -1 : 1);
                return input;
            }
        }

        // Iterate over all joystick buttons
        for (int i = 0; i < joystick.num_buttons; i++) {
            if (SDL_JoystickGetButton(joystick._ptr, i)) {
                input.set(SDLButtonMapping::ButtonType::BUTTON, i, 1);
                return input;
            }
        }

        // Iterate over DPad hats and record their value
        for (int i = 0; i < joystick.num_hats; i++) {
            int hat_direction = SDL_JoystickGetHat(joystick._ptr, i);
            if (hat_direction != 0) {
                input.set(SDLButtonMapping::ButtonType::HAT, i, hat_direction);
                return input;
            }
        }
    }
    return input;
}

void wait_for_no_sdljoystick_input(SDLJoystickData& joystick) {
    bool awaiting_silence = true;
    while (awaiting_silence && !APP_KILLED) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle events if necessary
        }

        int axis_value = 0;
        int button_value = 0;
        int hat_value = 0;
        
        // Iterate over all joystick axes and record their value
        for (int i = 0; i < SDL_JoystickNumAxes(joystick._ptr); i++) {
            int read_val = SDL_JoystickGetAxis(joystick._ptr, i); 
            axis_value += (abs(read_val-joystick.avgBaseline[i]) > 6000) ? abs(read_val) : 0;
        }

        // Iterate over all joystick buttons and record their value
        for (int i = 0; i < SDL_JoystickNumButtons(joystick._ptr); i++) {
            button_value += SDL_JoystickGetButton(joystick._ptr, i);
        }

        // Iterate over DPad hats and record their value
        for (int i = 0; i < SDL_JoystickNumHats(joystick._ptr); i++) {
            hat_value += SDL_JoystickGetHat(joystick._ptr, i);
        }

        // If no values are detected, the function may exit
        if (axis_value + hat_value + button_value == 0) {
            awaiting_silence = false;
        }
    }
}

bool there_is_sdljoystick_input(SDLJoystickData& joystick) {

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Handle events if necessary
        }

        // Iterate over all joystick axes
        for (int i = 0; i < SDL_JoystickNumAxes(joystick._ptr); i++) {
            int read_val = SDL_JoystickGetAxis(joystick._ptr, i);
            if (abs(read_val - joystick.avgBaseline[i]) > 10000)
                return true;
        }

        // Iterate over all joystick buttons
        for (int i = 0; i < SDL_JoystickNumButtons(joystick._ptr); i++) {
            if (SDL_JoystickGetButton(joystick._ptr, i))
                return true;
        }

        // Iterate over DPad hats
        for (int i = 0; i < SDL_JoystickNumHats(joystick._ptr); i++) {
            if (SDL_JoystickGetHat(joystick._ptr, i))
                return true;
        }

        // If no values are detected return false       
        return false;
}

void setSDLMapping(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>& inputList) {
    // Default Inputs (all)
    if (inputList.empty()) {
        inputList.insert(inputList.end(), joystick.mapping.stickButtonNames.begin(), joystick.mapping.stickButtonNames.end());
        inputList.insert(inputList.end(), joystick.mapping.shoulderButtonNames.begin(), joystick.mapping.shoulderButtonNames.end());
        inputList.insert(inputList.end(), joystick.mapping.triggerButtonNames.begin(), joystick.mapping.triggerButtonNames.end());
        inputList.insert(inputList.end(), joystick.mapping.thumbButtonNames.begin(), joystick.mapping.thumbButtonNames.end());
        inputList.insert(inputList.end(), joystick.mapping.dpadButtonNames.begin(), joystick.mapping.dpadButtonNames.end());
        inputList.insert(inputList.end(), joystick.mapping.genericButtonNames.begin(), joystick.mapping.genericButtonNames.end());
    }

    // Create a map to ensure no input is used twice
    std::unordered_map<std::string, bool> receivedInput;

    // Clear the output text buffer and announce purpose
    g_outputText = "Create Mapping for " + joystick.name + " \r\n\r\n";

    // Map all inputs in inputList
    for (const auto& inputID : inputList) {
        if (APP_KILLED) return;
        std::string inputName = SDLButtonMapping::getButtonNameString(inputID);
        SDLButtonMapping::ButtonType inputType = get_input_type(inputName);

        

        // Receive updates from the device and map inputs
        SDLButtonMapping::ButtonMapInput received_input;
        bool settingInput = true;
        while (settingInput && !APP_KILLED) {

            // Wait for no input to be detected
            if (there_is_sdljoystick_input(joystick)) { 
                std::cout << "  <<  Input Detected, Please Release To Continue >> ";
                while (there_is_sdljoystick_input(joystick) && !APP_KILLED) {
                    Sleep(20);
                }
            }
            
            // Prompt the user for input
            g_outputText = input_verb(inputType) + " " + format_input_name(inputName) + " ...\r\n";
            displayOutputText();

            // Receive an input signature
            received_input = get_sdljoystick_input(joystick);
            if (APP_KILLED) return;

            // Create a key from the input
            std::string inputKey = std::to_string(static_cast<int>(received_input.input_type)) + "_" + std::to_string(received_input.index) + "_" + std::to_string(received_input.value);

            if (received_input.input_type == SDLButtonMapping::ButtonType::UNSET) {
                settingInput = false;
            }
            else if (receivedInput.find(inputKey) == receivedInput.end()) {
                receivedInput[inputKey] = true;
                settingInput = false;
            }
            else {
                std::cout << " << That input ( " <<
                    SDLButtonMapping::getButtonTypeString(received_input.input_type) <<
                    ' ' << std::to_string(received_input.index) <<
                    SDLButtonMapping::getInputValueString(received_input.input_type, received_input.value) <<
                    " ) has already been assigned! >> \r\n";

                while (there_is_sdljoystick_input(joystick) && !APP_KILLED) {
                    Sleep(20);
                }
            }
        }

        // Map received input to inputID
        joystick.mapping.buttonMaps[inputID].set(received_input.input_type, received_input.index, received_input.value);

        if (received_input.input_type == SDLButtonMapping::ButtonType::UNSET) {
            g_outputText = "<< Input " + format_input_name(inputName) + " has been skipped! >>\r\n\r\n";
        }
        /*else {
            g_outputText = joystick.mapping.displayInput(inputID) + "\r\n\r\n";
        }*/

    }
    g_outputText += "<< All Done! >>\r\n\r\n";
    displayOutputText();

    return;
}


//************************
// VIGEM DATA STRUCTURES
//
// Possible XUSB report buttons. 
typedef enum _XUSB_BUTTON
{
    XUSB_GAMEPAD_DPAD_UP = 0x0001,
    XUSB_GAMEPAD_DPAD_DOWN = 0x0002,
    XUSB_GAMEPAD_DPAD_LEFT = 0x0004,
    XUSB_GAMEPAD_DPAD_RIGHT = 0x0008,
    XUSB_GAMEPAD_START = 0x0010,
    XUSB_GAMEPAD_BACK = 0x0020,
    XUSB_GAMEPAD_LEFT_THUMB = 0x0040,
    XUSB_GAMEPAD_RIGHT_THUMB = 0x0080,
    XUSB_GAMEPAD_LEFT_SHOULDER = 0x0100,
    XUSB_GAMEPAD_RIGHT_SHOULDER = 0x0200,
    XUSB_GAMEPAD_GUIDE = 0x0400,
    XUSB_GAMEPAD_A = 0x1000,
    XUSB_GAMEPAD_B = 0x2000,
    XUSB_GAMEPAD_X = 0x4000,
    XUSB_GAMEPAD_Y = 0x8000

} XUSB_BUTTON, * PXUSB_BUTTON;

// Represents an XINPUT_GAMEPAD-compatible report structure.
typedef struct _XUSB_REPORT
{
    USHORT wButtons;
    BYTE bLeftTrigger;
    BYTE bRightTrigger;
    SHORT sThumbLX;
    SHORT sThumbLY;
    SHORT sThumbRX;
    SHORT sThumbRY;

} XUSB_REPORT, * PXUSB_REPORT;


//***********************
//    SDL VIGEM HELPERS API
// 
// Map to translate SDLButtonMapping::ButtonName to _XUSB_BUTTON
std::map<SDLButtonMapping::ButtonName, _XUSB_BUTTON> toXUSB = {
        {SDLButtonMapping::ButtonName::DPAD_UP, XUSB_GAMEPAD_DPAD_UP},
        {SDLButtonMapping::ButtonName::DPAD_DOWN, XUSB_GAMEPAD_DPAD_DOWN},
        {SDLButtonMapping::ButtonName::DPAD_LEFT, XUSB_GAMEPAD_DPAD_LEFT},
        {SDLButtonMapping::ButtonName::DPAD_RIGHT, XUSB_GAMEPAD_DPAD_RIGHT},
        {SDLButtonMapping::ButtonName::START, XUSB_GAMEPAD_START},
        {SDLButtonMapping::ButtonName::BACK, XUSB_GAMEPAD_BACK},
        {SDLButtonMapping::ButtonName::LEFT_THUMB, XUSB_GAMEPAD_LEFT_THUMB},
        {SDLButtonMapping::ButtonName::RIGHT_THUMB, XUSB_GAMEPAD_RIGHT_THUMB},
        {SDLButtonMapping::ButtonName::LEFT_SHOULDER, XUSB_GAMEPAD_LEFT_SHOULDER},
        {SDLButtonMapping::ButtonName::RIGHT_SHOULDER, XUSB_GAMEPAD_RIGHT_SHOULDER},
        {SDLButtonMapping::ButtonName::GUIDE, XUSB_GAMEPAD_GUIDE},
        {SDLButtonMapping::ButtonName::A, XUSB_GAMEPAD_A},
        {SDLButtonMapping::ButtonName::B, XUSB_GAMEPAD_B},
        {SDLButtonMapping::ButtonName::X, XUSB_GAMEPAD_X},
        {SDLButtonMapping::ButtonName::Y, XUSB_GAMEPAD_Y}
};

// Function will populate an XUSB_REPORT with data from an SDLButtonMapping and a list of inputs to read from
void get_xbox_report_from_SDLmap(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName>const& inputList, XUSB_REPORT& xboxReport) {
    // Reset report values
    xboxReport.wButtons = 0;
    xboxReport.sThumbLX = 0;
    xboxReport.sThumbLY = 0;
    xboxReport.sThumbRX = 0;
    xboxReport.sThumbRY = 0;
    xboxReport.bLeftTrigger = 0;
    xboxReport.bRightTrigger = 0;

    // Receive new input events
    SDL_JoystickUpdate();

    // make names shorter
    using bName = SDLButtonMapping::ButtonName;
    using bType = SDLButtonMapping::ButtonType;
    using inputMap = decltype(joystick.mapping.buttonMaps);
    inputMap button = joystick.mapping.buttonMaps;

    for (const auto& inputID : inputList) {
        if (APP_KILLED) return;

         //button[inputID].input_type;  // the type of SDL input used to represent bName output
         //button[inputID].index;       // the index of the SDL input
         //button[inputID].value;       // a set value to compare to current input value

    // Hat Value
        if (button[inputID].input_type == bType::HAT) {
            int buttonValue = SDL_JoystickGetHat(joystick._ptr, button[inputID].index);
            bool isPressed = (button[inputID].value & buttonValue) != 0;  // bitwise AND compare
            // Assign value to the proper XBOX report field
            switch (inputID) {
            case bName::LEFT_STICK_LEFT: {
                if (isPressed)
                    xboxReport.sThumbLX = - INT16_MAX;
            }
                                          break;
            case bName::LEFT_STICK_RIGHT: {
                if (isPressed)
                    xboxReport.sThumbLX = INT16_MAX;
            }
                                        break;
            case bName::LEFT_STICK_UP: {
                if (isPressed)
                    xboxReport.sThumbLY = INT16_MAX;
            }
                                     break;
            case bName::LEFT_STICK_DOWN: {
                if (isPressed)
                    xboxReport.sThumbLY = -INT16_MAX;
            }
                                       break;
            case bName::RIGHT_STICK_LEFT: {
                if (isPressed)
                    xboxReport.sThumbRX = -INT16_MAX;
            }
                                        break;
            case bName::RIGHT_STICK_RIGHT: {
                if (isPressed)
                    xboxReport.sThumbRX = INT16_MAX;
            }
                                         break;
            case bName::RIGHT_STICK_UP: {
                if (isPressed)
                    xboxReport.sThumbRY = INT16_MAX;
            }
                                      break;
            case bName::RIGHT_STICK_DOWN: {
                if (isPressed)
                    xboxReport.sThumbRY = -INT16_MAX;
            }
                                        break;
            case bName::LEFT_TRIGGER: {
                xboxReport.bLeftTrigger = 255 * isPressed;
            }
                                    break;
            case bName::RIGHT_TRIGGER: {
                xboxReport.bRightTrigger = 255 * isPressed;
            }
                                     break;
            default: {
                if (isPressed)
                    //Return corresponding XBOX_BUTTON value based on input ID
                    xboxReport.wButtons += toXUSB[inputID];
            }
            }

        }

    // Axis Value
        else if (button[inputID].input_type == bType::STICK) {
            int inputValue = SDL_JoystickGetAxis(joystick._ptr, button[inputID].index);
            bool sameSign = haveSameSign(inputValue, button[inputID].value);
            int absVal = max((abs(inputValue) - 1), 0);
            if (absVal) {
                // Assign value to the proper XBOX report field
                switch (inputID) {
                case bName::LEFT_STICK_LEFT: {
                    if (sameSign)
                        // left stick left is negative value
                        xboxReport.sThumbLX = -absVal;
                }
                                           break;
                case bName::LEFT_STICK_RIGHT: {
                    if (sameSign)
                        // left stick right is positive value
                        xboxReport.sThumbLX = absVal;
                }
                                            break;
                case bName::LEFT_STICK_UP: {
                    if (sameSign)
                        // left stick up is positive value
                        xboxReport.sThumbLY = absVal;
                }
                                         break;
                case bName::LEFT_STICK_DOWN: {
                    if (sameSign)
                        // left stick down is negative value
                        xboxReport.sThumbLY = -absVal;
                }
                                           break;
                case bName::RIGHT_STICK_LEFT: {
                    if (sameSign)
                        // right stick left is negative value
                        xboxReport.sThumbRX = -absVal;
                }
                                            break;
                case bName::RIGHT_STICK_RIGHT: {
                    if (sameSign)
                        // right stick right is positive value
                        xboxReport.sThumbRX = absVal;
                }
                                             break;
                case bName::RIGHT_STICK_UP: {
                    if (sameSign)
                        // right stick up is positive value
                        xboxReport.sThumbRY = absVal;
                }
                                          break;
                case bName::RIGHT_STICK_DOWN: {
                    if (sameSign)
                        // right stick down is negative value
                        xboxReport.sThumbRY = -absVal;
                }
                                            break;
                case bName::LEFT_TRIGGER: {
                    if (sameSign)
                        // left trigger is positive byte value
                        xboxReport.bLeftTrigger = ShortToByte(absVal);
                }
                                        break;
                case bName::RIGHT_TRIGGER: {
                    if (sameSign)
                        // right trigger is positive byte value
                        xboxReport.bRightTrigger = ShortToByte(absVal);
                }
                                         break;
                default: {
                    if (absVal > 6000 && sameSign)
                        //Return corresponding XBOX_BUTTON value based on input ID
                        xboxReport.wButtons += toXUSB[inputID];
                }
                }
            }
        }

    // Standard Button
        else if (button[inputID].input_type == bType::BUTTON) {
            int buttonValue = SDL_JoystickGetButton(joystick._ptr, button[inputID].index);
            // Assign value to the proper XBOX report field
            switch (inputID) {
            case bName::LEFT_STICK_LEFT: {
                if (buttonValue)
                    xboxReport.sThumbLX = buttonValue * -INT16_MAX;
            }
                                       break;
            case bName::LEFT_STICK_RIGHT: {
                if (buttonValue)
                    xboxReport.sThumbLX = buttonValue * INT16_MAX;
            }
                                        break;
            case bName::LEFT_STICK_UP: {
                if (buttonValue)
                    xboxReport.sThumbLY = buttonValue * INT16_MAX;
            }
                                     break;
            case bName::LEFT_STICK_DOWN: {
                if (buttonValue)
                    xboxReport.sThumbLY = buttonValue * -INT16_MAX;
            }
                                       break;
            case bName::RIGHT_STICK_LEFT: {
                if (buttonValue)
                    xboxReport.sThumbRX = buttonValue * -INT16_MAX;
            }
                                        break;
            case bName::RIGHT_STICK_RIGHT: {
                if (buttonValue)
                    xboxReport.sThumbRX = buttonValue * INT16_MAX;
            }
                                         break;
            case bName::RIGHT_STICK_UP: {
                if (buttonValue)
                    xboxReport.sThumbRY = buttonValue * INT16_MAX;
            }
                                      break;
            case bName::RIGHT_STICK_DOWN: {
                if (buttonValue)
                    xboxReport.sThumbRY = buttonValue * -INT16_MAX;
            }
                                          break;
            case bName::LEFT_TRIGGER: {
                xboxReport.bLeftTrigger = buttonValue * 255;
            }
                                    break;
            case bName::RIGHT_TRIGGER: {
                xboxReport.bRightTrigger = buttonValue * 255;
            }
                                     break;
            default: {
                //Return corresponding XBOX_BUTTON value based on input ID
                if (buttonValue)
                    xboxReport.wButtons += toXUSB[inputID];
            }
            }
        }
    }
}

// Will output XUSB_REPORT values to g_outputText
void printXusbReport(const XUSB_REPORT& report) {
    g_outputText += "wButtons: " + std::to_string(report.wButtons) + "\r\n";
    g_outputText += "bLeftTrigger: " + std::to_string(static_cast<int>(report.bLeftTrigger)) + "\r\n";
    g_outputText += "bRightTrigger: " + std::to_string(static_cast<int>(report.bRightTrigger)) + "\r\n";
    g_outputText += "sThumbLX: " + std::to_string(report.sThumbLX) + "\r\n";
    g_outputText += "sThumbLY: " + std::to_string(report.sThumbLY) + "\r\n";
    g_outputText += "sThumbRX: " + std::to_string(report.sThumbRX) + "\r\n";
    g_outputText += "sThumbRY: " + std::to_string(report.sThumbRY) + "\r\n";
}


//***********************
//    SDL JOY HELPERS API
//
int InitJoystickInput()
{
    // Enable Xinput devices with this hint
    SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
    // Initialize SDL
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    return 0;
}

std::unordered_map<std::string, int> getJoystickList() {
    // Check for available joysticks
    int numJoysticks = SDL_NumJoysticks();
     if (numJoysticks <= 0)
    {
        std::cout << "No joysticks connected." << std::endl;
        //return -2;
    }
    // Create and Populate list
    std::unordered_map<std::string, int> joystickList;
    for (int i = 0; i < numJoysticks; ++i)
    {
        std::string joystickName = SDL_JoystickNameForIndex(i);
        joystickList[joystickName] = i;
    }
    return joystickList;
}

int ConsoleSelectJoystickDialog(int numJoysticks, SDLJoystickData& joystick) {
    // Print the available joysticks
       std::cout << "Connected Joysticks:" << std::endl;
       for (int i = 0; i < numJoysticks; ++i)
       {
           std::cout << 1+i << ": " << SDL_JoystickNameForIndex(i) << std::endl;
       }

       // Prompt the user to select a joystick
       int selectedJoystickIndex;
       std::cout << "Select a joystick (enter the index): ";
       std::cin >> selectedJoystickIndex;
       --selectedJoystickIndex;

       // Check if the selected index is valid
       if (selectedJoystickIndex < 0 || selectedJoystickIndex >= numJoysticks)
       {
           std::cout << "Invalid joystick index." << std::endl;
           SDL_Quit();
           return 0;
       }

       // Open the selected joystick
       joystick._ptr = SDL_JoystickOpen(selectedJoystickIndex);
       if (joystick._ptr == nullptr)
       {
           std::cout << "Failed to open joystick: " << SDL_GetError() << std::endl;
           SDL_Quit();
           return 0;
       } 
       return 1;
}

void OpenSelectJoystickDialog(std::unordered_map<std::string, int>& joystickList) {
    //
    // Prompt the user to select a joystick
    //
        // Reset global selected index
    g_joystickSelected = -1;
    // Create the "Select Joystick" dialog
    //DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SELECT_JOYSTICK_DIALOG), g_hWnd, SelectJoystickDialogProc, reinterpret_cast<LPARAM>(&joystickList));
    // Show the dialog
    //ShowWindow(g_hSelectJoystickDialog, SW_SHOW);
    // Await the selection
    while (g_joystickSelected < 0) {
        Sleep(10);
    }

}

int ConnectToJoystick(int joyIndex, SDLJoystickData& joystick) {
    // Open the specified joystick
    joystick._ptr = SDL_JoystickOpen(joyIndex);
    if (joystick._ptr == nullptr)
    {
        std::cout << "Failed to open joystick: " << SDL_GetError() << std::endl;
        return 0;
    }
    return 1;
}

void BuildJoystickInputData(SDLJoystickData& joystick) {
    // Get Baseline Reading for inputs
    std::tuple<int,int,int> joyInputInfo;
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> baselineReports;
    std::tie(joyInputInfo, baselineReports) = get_sdl_joystick_baseline(joystick._ptr);

    joystick.num_axes = std::get<0>(joyInputInfo);
    joystick.num_buttons = std::get<1>(joyInputInfo);
    joystick.num_hats = std::get<2>(joyInputInfo);
    joystick.name = std::string(SDL_JoystickName(joystick._ptr));
    joystick.avgBaseline = std::get<0>(baselineReports);
}

void OpenOrCreateMapping(SDLJoystickData& joystick, std::string& mapName) {
    // Check for a saved Map for selected joystick
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;
    if (result.first) {
        // File exists, try and load data
        joystick.mapping.loadMapping(filePath.string());
    }
    else {
        // File does not exist

        // Visual notification to user
        g_outputText = "No Button Map Exists for:  " + joystick.name + "\r\n\r\n\t Press Any Button To Continue\r\n\r\n";
        displayOutputText();

        // Wait for an input/button press
        get_sdljoystick_input(joystick);

        std::vector<SDLButtonMapping::ButtonName> inputList;
        // Set a Button Map for joystick
        setSDLMapping(joystick, inputList);
        //Save new mapping
        int didSave = joystick.mapping.saveMapping(filePath.string());
    }
}

int RemapInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName> inputList = std::vector<SDLButtonMapping::ButtonName>()){
    // Convert joystick name to hex mapfile name   
    std::string mapName = encodeStringToHex(joystick.name);
    // Get file path for a mapfile
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;

    // Visual notification to user
   // appendWindowTitle(g_hWnd, "Remap Buttons!"));
    g_outputText = "Remapping Inputs For:  " + joystick.name + "\r\n\r\n\t Press Any Button To Continue\r\n\r\n";
    displayOutputText();

    // Wait for an input/button press
    get_sdljoystick_input(joystick);

    // Set a Button Map for joystick
    setSDLMapping(joystick, inputList);

    //Save new mapping
    int didSave = joystick.mapping.saveMapping(filePath.string());
    //appendWindowTitle(g_hWnd, " Saved map : " + std::to_string(didSave)));
    return didSave;
}

int SDLRumble(SDLJoystickData& joystick, Uint8 leftMotor, Uint8 rightMotor, Uint32 duration_ms = 200) {
    return SDL_JoystickRumble(joystick._ptr, leftMotor*128, rightMotor*128, duration_ms);
}
//***************************************
