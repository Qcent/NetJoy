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
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <SDL3/SDL.h>

#define AXIS_INPUT_THRESHOLD 16000  // set high to prevent false positives on noisy input during mapping
#define AXIS_INPUT_DEADZONE 3000    // like threshold but used for main loop joystick reading

#define ANALOG_RANGE_NEG_TO_POS 3
#define ANALOG_RANGE_POS_TO_NEG 4

class SDLButtonMapping {
public:
    enum class ButtonType {
        UNSET,
        HAT,
        STICK,
        THUMB,
        TRIGGER,
        SHOULDER,
        BUTTON
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
        int special;

        ButtonMapInput(ButtonType input_type = ButtonType::UNSET, int index = -1, int value = 0, int spec = -1)
            : input_type(input_type), index(index), value(value), special(spec) {}

        void set(ButtonType input_type, int index, int value) {
            this->input_type = input_type;
            this->index = index;
            this->value = value;
        }

        void clear() {
            input_type = ButtonType::UNSET;
            index = -1;
            value = 0;
            special = -1;
        }

        bool operator==(const ButtonMapInput& other) const {
            return (this->input_type == other.input_type) &&
                (this->index == other.index) &&
                (this->value == other.value);
        }

        // Custom hash function for ButtonMapInput to be used as a key in unordered_map
        struct HashFunction {
            std::size_t operator()(const ButtonMapInput& input) const {
                return std::hash<int>()(static_cast<int>(input.input_type)) ^
                    std::hash<int>()(input.index) ^
                    std::hash<int>()(input.value);
            }
        };
    };

    using ButtonMap = std::unordered_map<ButtonName, ButtonMapInput>;
    using InverseMap = std::unordered_map<ButtonMapInput, ButtonName, ButtonMapInput::HashFunction>;

    ButtonMap buttonMaps;
    InverseMap inverseMap;
    std::vector<ButtonName> dpadInputList;
    std::vector<ButtonName> extRangeInputList;
    std::vector<ButtonName> stickButtonNames;
    std::vector<ButtonName> triggerButtonNames;
    std::vector<ButtonName> thumbButtonNames;
    std::vector<ButtonName> shoulderButtonNames;
    std::vector<ButtonName> dpadButtonNames;
    std::vector<ButtonName> genericButtonNames;

    SDLButtonMapping()
        : buttonMaps({ {ButtonName::DPAD_UP, ButtonMapInput()},
                      {ButtonName::DPAD_DOWN, ButtonMapInput()},
                      {ButtonName::DPAD_LEFT, ButtonMapInput()},
                      {ButtonName::DPAD_RIGHT, ButtonMapInput()},
                      {ButtonName::START, ButtonMapInput()},
                      {ButtonName::BACK, ButtonMapInput()},
                      {ButtonName::LEFT_THUMB, ButtonMapInput()},
                      {ButtonName::RIGHT_THUMB, ButtonMapInput()},
                      {ButtonName::LEFT_SHOULDER, ButtonMapInput()},
                      {ButtonName::RIGHT_SHOULDER, ButtonMapInput()},
                      {ButtonName::GUIDE, ButtonMapInput()},
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
                      {ButtonName::RIGHT_STICK_DOWN, ButtonMapInput()} }),
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

    void populateExtraMaps() {
        // Clear all maps/lists
        extRangeInputList.clear();
        dpadInputList.clear();
        inverseMap.clear();  
        std::vector<ButtonName> setButtons = getSetButtonNames();
        // Re-populate maps/lists
        for (const auto& buttonName : setButtons) {
            const auto& buttonInput = buttonMaps.at(buttonName);
            inverseMap[buttonInput] = buttonName;

            if (buttonInput.input_type == SDLButtonMapping::ButtonType::HAT) {
                dpadInputList.push_back(buttonName);
            }

            if (buttonInput.input_type == SDLButtonMapping::ButtonType::STICK && buttonInput.value >= ANALOG_RANGE_NEG_TO_POS) {
                extRangeInputList.push_back(buttonName);
            }
        }
    }

    int saveMapping(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (file.is_open()) {
            std::vector<std::tuple<ButtonName, ButtonType, int, int, int>> buttonList;
            for (const auto& buttonMap : buttonMaps) {
                const auto& buttonName = buttonMap.first;
                const auto& buttonInput = buttonMap.second;
                buttonList.emplace_back(buttonName, buttonInput.input_type, buttonInput.index, buttonInput.value, buttonInput.special);
            }

            const std::size_t tupleSize = sizeof(std::tuple<ButtonName, ButtonType, int, int, int>);
            const std::size_t dataSize = buttonList.size() * tupleSize;

            file.write(reinterpret_cast<const char*>(buttonList.data()), static_cast<std::streamsize>(dataSize));
            file.close();
            return 1; // Successfully saved button maps
        }
        else {
            return 0; // Error: Failed to open file
        }
    }

    int loadMapping(const std::string& filename) {
        bool V1_Flag = false;
        std::ifstream file(filename, std::ios::binary);
        if (file.is_open()) {
            file.seekg(0, std::ios::end);
            std::size_t fileSize = static_cast<std::size_t>(file.tellg());
            file.seekg(0, std::ios::beg);

            if (fileSize % sizeof(std::tuple<ButtonName, ButtonType, int, int, int>) != 0) {
                // Error: Invalid file size for mapping version 2
                // Check if size matches version 1
                if(fileSize % sizeof(std::tuple<ButtonName, ButtonType, int, int>) == 0) {
                    V1_Flag = true;
                    g_outputText += OLDMAP_WARNING_MSG;
                    g_outputText += "\r\n";
                }
                else {
                    file.close();
                    return 0;
                }
            }

            if (V1_Flag) {
                // Old format
                std::vector<std::tuple<ButtonName, ButtonType, int, int>> buttonList(fileSize / sizeof(std::tuple<ButtonName, ButtonType, int, int>));
                file.read(reinterpret_cast<char*>(buttonList.data()), static_cast<std::streamsize>(fileSize));

                buttonMaps.clear();
                for (const auto& buttonTuple : buttonList) {
                    const auto& buttonName = std::get<0>(buttonTuple);
                    const auto& buttonInputType = std::get<1>(buttonTuple);
                    const auto& buttonIndex = std::get<2>(buttonTuple);
                    const auto& buttonValue = std::get<3>(buttonTuple);

                    // Add int with a value of -1
                    ButtonMapInput buttonInput(buttonInputType, buttonIndex, buttonValue, -1);
                    buttonMaps[buttonName] = buttonInput;
                }
            }
            else {
                std::vector<std::tuple<ButtonName, ButtonType, int, int, int>> buttonList(fileSize / sizeof(std::tuple<ButtonName, ButtonType, int, int, int>));
                file.read(reinterpret_cast<char*>(buttonList.data()), static_cast<std::streamsize>(fileSize));

                buttonMaps.clear();
                for (const auto& buttonTuple : buttonList) {
                    const auto& buttonName = std::get<0>(buttonTuple);
                    const auto& buttonInputType = std::get<1>(buttonTuple);
                    const auto& buttonIndex = std::get<2>(buttonTuple);
                    const auto& buttonValue = std::get<3>(buttonTuple);
                    const auto& buttonSpecial = std::get<4>(buttonTuple);

                    ButtonMapInput buttonInput(buttonInputType, buttonIndex, buttonValue, buttonSpecial);
                    buttonMaps[buttonName] = buttonInput;
                }
            }

            file.close();
            // update SDL3 maps
            populateExtraMaps();
            return 1; // Successfully loaded button maps
        }
        else {
            return 0; // Error: Failed to open file
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

    static std::string displayInput(ButtonMapInput input) {
        if (input.input_type == SDLButtonMapping::ButtonType::UNSET)
            return " (UNSET) ";
        std::string out = getButtonTypeString(input.input_type) + " " +
            std::to_string(input.index) + ": " +
            std::to_string(input.value);

        return out;
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
    int joyID = -1;
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
    else if (lowercaseString.find("shoulder") != std::string::npos) {
        return SDLButtonMapping::ButtonType::BUTTON;
    }
    else {
        return SDLButtonMapping::ButtonType::UNSET;
    }
}

bool haveSameSign(int num1, int num2) {
    return (num1 >= 0 && num2 >= 0) || (num1 < 0 && num2 < 0);
}

BYTE ShortToByte(SHORT value){
    // Scale the absolute value to fit within the range of a BYTE (0 to 255)
    DOUBLE scaleFactor = 255.0 / SHRT_MAX;
    DOUBLE scaledValue = scaleFactor * value;

    // Round the scaled value to the nearest integer
    BYTE byteValue = static_cast<BYTE>(std::round(scaledValue));

    return byteValue;
}

BYTE SignedShortToUnsignedByte(int16_t signedValue) {
    // Map signed short range [-32768, 32767] to unsigned byte range [0, 255]
    BYTE scaledValue = static_cast<BYTE>(
        ((signedValue - INT16_MIN) * 255) / (INT16_MAX - INT16_MIN)
        );

    return scaledValue;
}

BYTE SignedShortToUnsignedByteReversed(int16_t signedValue) {
    // Map signed short range [-32768, 32767] to unsigned byte range [255, 0]
    BYTE scaledValue = static_cast<BYTE>(
        255 - ((signedValue - INT16_MIN) * 255) / (INT16_MAX - INT16_MIN)
        );

    return scaledValue;
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
get_sdl_joystick_baseline(SDL_Joystick* joystick, int numSamples = 64) {
    int numAxes = SDL_GetNumJoystickAxes(joystick);
    int numButtons = SDL_GetNumJoystickButtons(joystick);
    int numHats = SDL_GetNumJoystickHats(joystick);

    // Setup sample buffer for all inputs
    std::vector<std::vector<int>> sampleBuf(numSamples, std::vector<int>(numHats + numButtons + numAxes));
    std::vector<int> avgBuffer(numHats + numButtons + numAxes, 0);
    std::vector<int> avgReport(numHats + numButtons + numAxes, 0);
    std::vector<int> medianReport(numHats + numButtons + numAxes, 0);
    std::vector<int> modeReport(numHats + numButtons + numAxes, 0);
    std::vector<int> rangeReport(numHats + numButtons + numAxes, 0);

    // Collect data from samples
    for (int i = 0; i < numSamples; i++) {
        SDL_UpdateJoysticks();
        for (int j = 0; j < numHats + numButtons + numAxes; j++) {
            int value = 0;
            if (j < numAxes) {
                value = SDL_GetJoystickAxis(joystick, j);
            }
            else if (j < numAxes + numButtons) {
                value = SDL_GetJoystickButton(joystick, j - numAxes);
            }
            else {
                value = SDL_GetJoystickHat(joystick, j - numAxes - numButtons);
            }
            avgBuffer[j] += value;
            sampleBuf[i][j] = value;
        }

        // sleep for more accurate polling
        Sleep(5);
    }
    // clear all joystick events generated during scan
    SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);

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

void get_sdljoystick_mapping_input(const SDLJoystickData& joystick, SDLButtonMapping::ButtonMapInput& input) {
    SDL_UpdateJoysticks();
    // Clear all joystick events to prevent memory leaks
    SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);
    // Iterate over all Axes
    for (int i = 0; i < joystick.num_axes; i++) {
        int axis_value = SDL_GetJoystickAxis(joystick._ptr, i); // / 32767.0f;
        if (std::abs(axis_value - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD) {

            // Watch axis to see how far it moves in ~1/5 second
            constexpr double totaltime_s = 1 / 5.0f;
            constexpr int numsamples = 12;
            constexpr int delay_ms = (totaltime_s / numsamples) * 1000;
            int inital_reading, low_reading, high_reading;
            inital_reading = low_reading = high_reading = axis_value;
            for (int watching = 0; watching < numsamples; watching++) {
                Sleep(delay_ms);
                SDL_UpdateJoysticks();
                axis_value = SDL_GetJoystickAxis(joystick._ptr, i);
                if (std::abs(axis_value - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD) {
                    if (axis_value > high_reading) high_reading = axis_value;
                    else if (axis_value < low_reading) low_reading = axis_value;
                }
            }
            SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);

            // Analyze readings
            if (inital_reading <= 0 && low_reading <= 0 && high_reading <= 0) {
                // All readings were negative
                input.set(SDLButtonMapping::ButtonType::STICK, i, -1);
            }
            else if (inital_reading >= 0 && low_reading >= 0 && high_reading >= 0) {
                // All readings were positive
                input.set(SDLButtonMapping::ButtonType::STICK, i, 1);
            }
            else if (inital_reading < 0 && low_reading < 0 && high_reading > 0) {
                // Values went from - to +
                input.set(SDLButtonMapping::ButtonType::STICK, i, ANALOG_RANGE_NEG_TO_POS);
            }
            else if (inital_reading > 0 && high_reading > 0 && low_reading < 0) {
                // Values went from + to -
                input.set(SDLButtonMapping::ButtonType::STICK, i, ANALOG_RANGE_POS_TO_NEG);
            }

            return;
        }
    }

    // Iterate over all Buttons
    for (int i = 0; i < joystick.num_buttons; i++) {
        if (SDL_GetJoystickButton(joystick._ptr, i)) {
            input.set(SDLButtonMapping::ButtonType::BUTTON, i, 1);
            return;
        }
    }

    // Iterate over DPad/hats
    for (int i = 0; i < joystick.num_hats; i++) {
        int hat_direction = SDL_GetJoystickHat(joystick._ptr, i);
        if (hat_direction != 0) {
            input.set(SDLButtonMapping::ButtonType::HAT, i, hat_direction);
            return;
        }
    }

}

SDLButtonMapping::ButtonMapInput get_sdljoystick_input(const SDLJoystickData& joystick) {
    SDLButtonMapping::ButtonMapInput input;
    while (!APP_KILLED) {

        if (getKeyState(VK_ESCAPE)) {
            while (getKeyState(VK_ESCAPE)) {
                // Esc key pressed, wait for release
                Sleep(10);
            }
            // Esc key released, return empty input
            return input;
        }

        get_sdljoystick_mapping_input(joystick, input);
        if (input.input_type != SDLButtonMapping::ButtonType::UNSET) {
            return input;
        }

        Sleep(20);
    }
    return input;
}

void wait_for_no_sdljoystick_input(SDLJoystickData& joystick) {
    bool awaiting_silence = true;
    while (awaiting_silence && !APP_KILLED) {
        Sleep(20);
        SDL_UpdateJoysticks();
        SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);

        int axis_value = 0;
        int button_value = 0;
        int hat_value = 0;

        // Iterate over all joystick axes and record their value
        for (int i = 0; i < joystick.num_axes; i++) {
            int read_val = SDL_GetJoystickAxis(joystick._ptr, i);
            axis_value += (abs(read_val - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD) ? abs(read_val) : 0;
        }

        // Iterate over all joystick buttons and record their value
        for (int i = 0; i < joystick.num_buttons; i++) {
            button_value += SDL_GetJoystickButton(joystick._ptr, i);
        }

        // Iterate over DPad hats and record their value
        for (int i = 0; i < joystick.num_hats; i++) {
            hat_value += SDL_GetJoystickHat(joystick._ptr, i);
        }

        // If no values are detected, the function may exit
        if (axis_value + hat_value + button_value == 0) {
            awaiting_silence = false;
        }
    }
}

bool there_is_sdljoystick_input(SDLJoystickData& joystick) {
    SDL_UpdateJoysticks();
    SDL_FlushEvents(SDL_EVENT_JOYSTICK_AXIS_MOTION, SDL_EVENT_JOYSTICK_UPDATE_COMPLETE);

    // Iterate over all joystick axes
    for (int i = 0; i < joystick.num_axes; i++) {
        int read_val = SDL_GetJoystickAxis(joystick._ptr, i);
        if (abs(read_val - joystick.avgBaseline[i]) > AXIS_INPUT_THRESHOLD)
            return true;
    }

    // Iterate over all joystick buttons
    for (int i = 0; i < joystick.num_buttons; i++) {
        if (SDL_GetJoystickButton(joystick._ptr, i))
            return true;
    }

    // Iterate over DPad hats
    for (int i = 0; i < joystick.num_hats; i++) {
        if (SDL_GetJoystickHat(joystick._ptr, i))
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
                std::cout << " <<  Input Detected, Please Release To Continue  >> ";
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
            if ((inputType == SDLButtonMapping::ButtonType::BUTTON ||
                inputType == SDLButtonMapping::ButtonType::SHOULDER ||
                inputType == SDLButtonMapping::ButtonType::HAT ||
                inputType == SDLButtonMapping::ButtonType::THUMB)
                && received_input.input_type == SDLButtonMapping::ButtonType::STICK) {
                if ((received_input.value == ANALOG_RANGE_NEG_TO_POS)
                    || (received_input.value == ANALOG_RANGE_POS_TO_NEG)) {
                    received_input.special = received_input.value;
                    received_input.value = (received_input.special == ANALOG_RANGE_NEG_TO_POS) ? -1 : 1;
                }
            }

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

    }

    joystick.mapping.populateExtraMaps();

    g_outputText += "<< All Done! >>\r\n\r\n";
    displayOutputText();
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

// Used for looping through all Dpad directions
BYTE DPAD_DIRECTIONS[] = { XUSB_GAMEPAD_DPAD_UP, XUSB_GAMEPAD_DPAD_DOWN, XUSB_GAMEPAD_DPAD_LEFT, XUSB_GAMEPAD_DPAD_RIGHT };

// Clears the value for a specific emulatedInput in an XUSB_REPORT
void clear_XBOX_REPORT_value(const SDLButtonMapping::ButtonName emulatedInput, XUSB_REPORT& xboxReport) {
    using inputName = SDLButtonMapping::ButtonName;
    switch (emulatedInput) {
    case inputName::LEFT_STICK_LEFT:
        xboxReport.sThumbLX = 0;
        break;
    case inputName::LEFT_STICK_RIGHT:
        xboxReport.sThumbLX = 0;
        break;
    case inputName::LEFT_STICK_UP:
        xboxReport.sThumbLY = 0;
        break;
    case inputName::LEFT_STICK_DOWN:
        xboxReport.sThumbLY = 0;
        break;
    case inputName::RIGHT_STICK_LEFT:
        xboxReport.sThumbRX = 0;
        break;
    case inputName::RIGHT_STICK_RIGHT:
        xboxReport.sThumbRX = 0;
        break;
    case inputName::RIGHT_STICK_UP:
        xboxReport.sThumbRY = 0;
        break;
    case inputName::RIGHT_STICK_DOWN:
        xboxReport.sThumbRY = 0;
        break;
    case inputName::LEFT_TRIGGER:
        xboxReport.bLeftTrigger = 0;
        break;
    case inputName::RIGHT_TRIGGER:
        xboxReport.bRightTrigger = 0;
        break;
    default:
        xboxReport.wButtons &= ~toXUSB[emulatedInput];
        break;
    }
}

// Function will update an XUSB_REPORT from a SDLButtonMapping::ButtonMapInput targeting emulatedInput
void SDL_event_to_xbox_report(const SDLButtonMapping::ButtonMapInput sdlEvent, const SDLButtonMapping::ButtonName emulatedInput, XUSB_REPORT& xboxReport, SDLJoystickData& joystick) {
    using inputName = SDLButtonMapping::ButtonName;
    using inputType = SDLButtonMapping::ButtonType;
    int absVal = 0;

    switch (sdlEvent.input_type) {
    case inputType::HAT:
        switch (emulatedInput) {
        case inputName::LEFT_STICK_LEFT:
            xboxReport.sThumbLX = INT16_MIN;
            break;
        case inputName::LEFT_STICK_RIGHT:
            xboxReport.sThumbLX = INT16_MAX;
            break;
        case inputName::LEFT_STICK_UP:
            xboxReport.sThumbLY = INT16_MAX;
            break;
        case inputName::LEFT_STICK_DOWN:
            xboxReport.sThumbLY = INT16_MIN;
            break;
        case inputName::RIGHT_STICK_LEFT:
            xboxReport.sThumbRX = INT16_MIN;
            break;
        case inputName::RIGHT_STICK_RIGHT:
            xboxReport.sThumbRX = INT16_MAX;
            break;
        case inputName::RIGHT_STICK_UP:
            xboxReport.sThumbRY = INT16_MAX;
            break;
        case inputName::RIGHT_STICK_DOWN:
            xboxReport.sThumbRY = INT16_MIN;
            break;
        case inputName::LEFT_TRIGGER:
            xboxReport.bLeftTrigger = UINT8_MAX;
            break;
        case inputName::RIGHT_TRIGGER:
            xboxReport.bRightTrigger = UINT8_MAX;
            break;
        default:
            // Return corresponding XBOX_BUTTON value based on emulatedInput
            xboxReport.wButtons += toXUSB[emulatedInput];
            break;
        }
        break;

    case inputType::STICK:
        absVal = max((abs(sdlEvent.value) - 1), 0); // prevents off by 1 errors when swapping INT16_MAX-INT16_MIN
        switch (emulatedInput) {
        case inputName::LEFT_STICK_LEFT:
            xboxReport.sThumbLX = -absVal;
            break;
        case inputName::LEFT_STICK_RIGHT:
            xboxReport.sThumbLX = absVal;
            break;
        case inputName::LEFT_STICK_UP:
            xboxReport.sThumbLY = absVal;
            break;
        case inputName::LEFT_STICK_DOWN:
            xboxReport.sThumbLY = -absVal;
            break;
        case inputName::RIGHT_STICK_LEFT:
            xboxReport.sThumbRX = -absVal;
            break;
        case inputName::RIGHT_STICK_RIGHT:
            xboxReport.sThumbRX = absVal;
            break;
        case inputName::RIGHT_STICK_UP:
            xboxReport.sThumbRY = absVal;
            break;
        case inputName::RIGHT_STICK_DOWN:
            xboxReport.sThumbRY = -absVal;
            break;
        case inputName::LEFT_TRIGGER:
            if (sdlEvent.index == ANALOG_RANGE_NEG_TO_POS) {
                xboxReport.bLeftTrigger = SignedShortToUnsignedByte(sdlEvent.value);
            }
            else if (sdlEvent.index == ANALOG_RANGE_POS_TO_NEG) {
                xboxReport.bLeftTrigger = SignedShortToUnsignedByteReversed(sdlEvent.value);
            }
            else {
                xboxReport.bLeftTrigger = ShortToByte(absVal);
            }
            break;
        case inputName::RIGHT_TRIGGER:
            if (sdlEvent.index == ANALOG_RANGE_NEG_TO_POS) {
                xboxReport.bRightTrigger = SignedShortToUnsignedByte(sdlEvent.value);
            }
            else if (sdlEvent.index == ANALOG_RANGE_POS_TO_NEG) {
                xboxReport.bRightTrigger = SignedShortToUnsignedByteReversed(sdlEvent.value);
            }
            else {
                xboxReport.bRightTrigger = ShortToByte(absVal);
            }
            break;
        default:
            // remove value from buttons
            xboxReport.wButtons &= ~toXUSB[emulatedInput];

            // for sticks that use *special* full range (INT16_MIN - INT16_MAX)
            if ((sdlEvent.special == ANALOG_RANGE_NEG_TO_POS
                && sdlEvent.value < (INT16_MIN + AXIS_INPUT_DEADZONE))
                || (sdlEvent.special == ANALOG_RANGE_POS_TO_NEG
                    && sdlEvent.value > (INT16_MAX - AXIS_INPUT_DEADZONE)))
                break;
            // for sticks that use signed axis            
            else if (sdlEvent.special == -1 && (std::abs(sdlEvent.value - joystick.avgBaseline[sdlEvent.index]) < AXIS_INPUT_DEADZONE))
                break;

            // Return corresponding XBOX_BUTTON value based on emulatedInput
            xboxReport.wButtons += toXUSB[emulatedInput];

            break;
        }

        break;

    case inputType::BUTTON:
        switch (emulatedInput) {
        case inputName::LEFT_STICK_LEFT:
            xboxReport.sThumbLX = sdlEvent.value * INT16_MIN;
            break;
        case inputName::LEFT_STICK_RIGHT:
            xboxReport.sThumbLX = sdlEvent.value * INT16_MAX;
            break;
        case inputName::LEFT_STICK_UP:
            xboxReport.sThumbLY = sdlEvent.value * INT16_MAX;
            break;
        case inputName::LEFT_STICK_DOWN:
            xboxReport.sThumbLY = sdlEvent.value * INT16_MIN;
            break;
        case inputName::RIGHT_STICK_LEFT:
            xboxReport.sThumbRX = sdlEvent.value * INT16_MIN;
            break;
        case inputName::RIGHT_STICK_RIGHT:
            xboxReport.sThumbRX = sdlEvent.value * INT16_MAX;
            break;
        case inputName::RIGHT_STICK_UP:
            xboxReport.sThumbRY = sdlEvent.value * INT16_MAX;
            break;
        case inputName::RIGHT_STICK_DOWN:
            xboxReport.sThumbRY = sdlEvent.value * INT16_MIN;
            break;
        case inputName::LEFT_TRIGGER:
            xboxReport.bLeftTrigger = sdlEvent.value * UINT8_MAX;
            break;
        case inputName::RIGHT_TRIGGER:
            xboxReport.bRightTrigger = sdlEvent.value * UINT8_MAX;
            break;
        default:
            xboxReport.wButtons += (sdlEvent.value ? 1 : -1) * toXUSB[emulatedInput];
            break;
        }
        break;

    default:
        break;
    }
}

// Builds a new XUSB_REPORT from current SDL_Events returns false if joystick is removed, else true
bool get_xbox_report_from_SDL_events(SDLJoystickData& joystick, XUSB_REPORT& xbox_report) {
    SDL_Event event;
    SDLButtonMapping::ButtonMapInput eventMap;
    while (SDL_PollEvent(&event) != 0) {
        int input_is_set = false;
        if (event.jdevice.which != joystick.joyID) {
            continue; // bypass any events not from our joystick
        }
        switch (event.type) {
        case SDL_EVENT_JOYSTICK_REMOVED:
            return false;
            break;

        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            eventMap.set(SDLButtonMapping::ButtonType::BUTTON, (int)event.jbutton.button, true);

            // Check if the eventMap exists in the inverseMap
            if (joystick.mapping.inverseMap.find(eventMap) != joystick.mapping.inverseMap.end()) {
                SDL_event_to_xbox_report(eventMap, joystick.mapping.inverseMap[eventMap], xbox_report, joystick);
            }
            break;

        case SDL_EVENT_JOYSTICK_BUTTON_UP:
            eventMap.set(SDLButtonMapping::ButtonType::BUTTON, (int)event.jbutton.button, true);

            // Check if the eventMap exists in the inverseMap
            if (joystick.mapping.inverseMap.find(eventMap) != joystick.mapping.inverseMap.end()) {
                auto emulatedInput = joystick.mapping.inverseMap[eventMap];
                eventMap.value = false;
                SDL_event_to_xbox_report(eventMap, emulatedInput, xbox_report, joystick);
            }
            break;

        case SDL_EVENT_JOYSTICK_HAT_MOTION:
            // All values set by DPAD must be reset on DPAD value change
            for (auto const& input : joystick.mapping.dpadInputList) {
                    clear_XBOX_REPORT_value(input, xbox_report);
            }

            // All dpad/hat directions must be assessed, as a single value is given for entire dpad state
            for (int dpad_dir : DPAD_DIRECTIONS) {
                // bitwise AND comparison of dpad directions (extracts single direction from possible multi direction)
                if (event.jhat.value & dpad_dir) {
                    eventMap.set(SDLButtonMapping::ButtonType::HAT, (int)event.jhat.hat, dpad_dir);

                    // If the eventMap exists in the inverseMap: set xbox_report accordingly
                    if (joystick.mapping.inverseMap.find(eventMap) != joystick.mapping.inverseMap.end()) {
                        SDL_event_to_xbox_report(eventMap, joystick.mapping.inverseMap[eventMap], xbox_report, joystick);
                    }
                }
            }
            break;

        case SDL_EVENT_JOYSTICK_AXIS_MOTION:
            // Ensure extended range mode by inserting mapped axis range value into the input signature for known extended range inputs
            for (auto extRangeInput : joystick.mapping.extRangeInputList) {
                eventMap.set(SDLButtonMapping::ButtonType::STICK, event.jaxis.axis, joystick.mapping.buttonMaps[extRangeInput].value); // insert axis range value

                // Check if eventMap == stored button mapping, ensuring that extended range mode will be used if it is set
                if (joystick.mapping.buttonMaps[extRangeInput] == eventMap) {
                    // set index to mapped range value / and value to axis value (in special)
                    eventMap.index = eventMap.value;
                    eventMap.value = event.jaxis.value;
                    SDL_event_to_xbox_report(eventMap, extRangeInput, xbox_report, joystick);
                    input_is_set = true;
                    break;
                }
            }
            if (input_is_set) break;

            // spoof mapped input range value based on axis value
            eventMap.set(SDLButtonMapping::ButtonType::STICK, event.jaxis.axis, event.jaxis.value > 0 ? 1 : -1);

            // Check if the spoofed eventMap exists in the inverseMap
            if (joystick.mapping.inverseMap.find(eventMap) != joystick.mapping.inverseMap.end()) {
                auto emulatedInput = joystick.mapping.inverseMap[eventMap];
                // set value from stick input
                eventMap.value = event.jaxis.value;
                SDL_event_to_xbox_report(eventMap, emulatedInput, xbox_report, joystick);
            }
            break;

        case SDL_EVENT_QUIT:
            APP_KILLED = true;
            break;
        }
    }
    return 1;
}

// Will output XUSB_REPORT values to g_outputText
void printXusbReport(const XUSB_REPORT& report) {
    g_outputText += "wButtons: " + std::to_string(report.wButtons) + "      \r\n";
    g_outputText += "bLeftTrigger: " + std::to_string(static_cast<int>(report.bLeftTrigger)) + "      \r\n";
    g_outputText += "bRightTrigger: " + std::to_string(static_cast<int>(report.bRightTrigger)) + "      \r\n";
    g_outputText += "sThumbLX: " + std::to_string(report.sThumbLX) + "      \r\n";
    g_outputText += "sThumbLY: " + std::to_string(report.sThumbLY) + "      \r\n";
    g_outputText += "sThumbRX: " + std::to_string(report.sThumbRX) + "      \r\n";
    g_outputText += "sThumbRY: " + std::to_string(report.sThumbRY) + "      \r\n";
}


//***********************
//    SDL JOY HELPERS API
//
int InitJoystickInput()
{
    // Enable Xinput devices with this hint
    SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
    // Disable SDL handling SIGINT
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
    // Initialize SDL
    if (SDL_Init(SDL_INIT_JOYSTICK) < 0)
    {
        std::cout << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }
    return 0;
}

int ConnectToJoystick(int joyIndex, SDLJoystickData& joystick) {
    int numJoysticks = 0;
    SDL_JoystickID* joystick_list = nullptr;
    joystick_list = SDL_GetJoysticks(&numJoysticks);

    // Open the specified joystick
    joystick._ptr = SDL_OpenJoystick(joystick_list[joyIndex]);
    if (joystick._ptr == nullptr)
    {
        //std::cout << "Failed to open joystick: " << SDL_GetError() << std::endl;
        SDL_free(joystick_list);
        return 0;
    }
    joystick.joyID = joystick_list[joyIndex];

    SDL_free(joystick_list);
    return 1;
}

int ConsoleSelectJoystickDialog(SDLJoystickData& joystick) {
    int numJoysticks = 0;
    SDL_JoystickID* joystick_list = nullptr;
    joystick_list = SDL_GetJoysticks(&numJoysticks);
    // Print the available joysticks
    std::cout << "Connected Joysticks:" << std::endl;

    if (numJoysticks < 1) {
        SDL_free(joystick_list);
        std::cout << "\t (None) \n  Connect a joystick to continue...";

        bool needJoystick = true;
        SDL_Event event;
        while (!APP_KILLED && needJoystick) {
            while (SDL_PollEvent(&event) != 0) {
                switch (event.type) {
                case SDL_EVENT_JOYSTICK_ADDED:
                    needJoystick = false;
                    break;
                }
            }
            Sleep(100);
        }

        clearConsoleScreen();
        joystick_list = SDL_GetJoysticks(&numJoysticks);
        std::cout << "Connected Joysticks:" << std::endl;
    }

    for (int i = 0; i < numJoysticks; ++i)
    {
        std::cout << 1 + i << ": " << SDL_GetJoystickNameForID(joystick_list[i]) << std::endl;
    }
    SDL_free(joystick_list);

    // Prompt the user to select a joystick
    int selectedJoystickIndex;
    std::cout << "Select a joystick (1";
    if (numJoysticks > 1) std::cout << "-" << numJoysticks;
    std::cout << "): ";
    std::cin >> selectedJoystickIndex;
    --selectedJoystickIndex;
    while ((getchar()) != '\n');

    // Check if the selected index is valid
    if (selectedJoystickIndex < 0 || selectedJoystickIndex >= numJoysticks)
    {
        clearConsoleScreen();
        std::cout << "Invalid joystick index." << std::endl;
        
        // allows for rescan of devices
        return ConsoleSelectJoystickDialog(joystick);
    }

    // Open the selected joystick
    return ConnectToJoystick(selectedJoystickIndex, joystick);
}

void BuildJoystickInputData(SDLJoystickData& joystick) {
    // Get Baseline Reading for inputs
    std::tuple<int, int, int> joyInputInfo;
    std::tuple<std::vector<int>, std::vector<int>, std::vector<int>, std::vector<int>> baselineReports;
    std::tie(joyInputInfo, baselineReports) = get_sdl_joystick_baseline(joystick._ptr);

    joystick.num_axes = std::get<0>(joyInputInfo);
    joystick.num_buttons = std::get<1>(joyInputInfo);
    joystick.num_hats = std::get<2>(joyInputInfo);
    joystick.name = std::string(SDL_GetJoystickNameForID(joystick.joyID));
    joystick.avgBaseline = std::get<0>(baselineReports);
}

void OpenOrCreateMapping(SDLJoystickData& joystick) {
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
            return OpenOrCreateMapping(joystick);
        }
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

int RemapInputs(SDLJoystickData& joystick, std::vector<SDLButtonMapping::ButtonName> inputList = std::vector<SDLButtonMapping::ButtonName>()) {
    // Convert joystick name to hex mapfile name   
    std::string mapName = encodeStringToHex(joystick.name);
    // Get file path for a mapfile
    auto result = check_for_saved_mapping(mapName);
    std::filesystem::path filePath = result.second;

    // Visual notification to user
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
    return SDL_RumbleJoystick(joystick._ptr, leftMotor * 128 + 127 * (leftMotor > 0), rightMotor * 128 + 127 * (rightMotor > 0), duration_ms);
}
//***************************************
