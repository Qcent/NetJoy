#pragma once
#include "textUI.hpp"

#define XBOX_QUIT_LINE  18
#define DS4_QUIT_LINE   17

// 'Permanent' / memory persistent, on screen buttons
mouseButton quitButton(CONSOLE_WIDTH / 2 - 5, XBOX_QUIT_LINE, 11, L" (Q) Quit  ");
mouseButton mappingButton(CONSOLE_WIDTH / 2 - 9, XBOX_QUIT_LINE - 2, 18, L" (M) Map Buttons  ");
mouseButton restartButton[4] = {
    mouseButton(CONSOLE_WIDTH / 2 - 12, XBOX_QUIT_LINE - 1, 25, L" (R[\t,\t]) Restart "),
    mouseButton(CONSOLE_WIDTH / 2 - 8, XBOX_QUIT_LINE - 1, 1, L"1"),
    mouseButton(CONSOLE_WIDTH / 2 - 6, XBOX_QUIT_LINE - 1, 1, L"2"),
    mouseButton(CONSOLE_WIDTH / 2 + 6, XBOX_QUIT_LINE - 1, 9, L"[mode] "),
};


// *********************************
// Declare all necessary controller buttons: outline and highlight areas

// Controller button highlight areas
mouseButton button_A_highlight(51, 11, 2, L"  ", UNHOVERABLE);
mouseButton button_B_highlight(54, 9, 2, L"  ", UNHOVERABLE);
mouseButton button_X_highlight(47, 9, 2, L"  ", UNHOVERABLE);
mouseButton button_Y_highlight(51, 7, 2, L"  ", UNHOVERABLE);

mouseButton button_Back_highlight(29, 8, 3, L"( )");
mouseButton button_Start_highlight(41, 8, 3, L"( )");
mouseButton button_Guide_highlight(35, 7, 3, L"[_]");

mouseButton button_DpadUp_highlight(27, 10, 2, L"  ", UNHOVERABLE);
mouseButton button_DpadLeft_highlight(24, 11, 2, L"__", UNHOVERABLE);
mouseButton button_DpadRight_highlight(30, 11, 2, L"__", UNHOVERABLE);
mouseButton button_DpadDown_highlight(27, 12, 2, L"__", UNHOVERABLE);

mouseButton button_L1_highlight(18, 4, 8, L"________");
mouseButton button_L2_highlight(19, 3, 6, L"______");
mouseButton button_L3_highlight(21, 8, 2, L"  ");

mouseButton button_LStickUp_highlight(21, 7, 2, L"--");
mouseButton button_LStickLeft_highlight(18, 8, 2, L" {");
mouseButton button_LStickRight_highlight(24, 8, 2, L"} ");
mouseButton button_LStickDown_highlight(21, 9, 2, L"--");

mouseButton button_R1_highlight(47, 4, 8, L"________");
mouseButton button_R2_highlight(48, 3, 6, L"______");
mouseButton button_R3_highlight(41, 11, 2, L"  ");

mouseButton button_RStickUp_highlight(41, 10, 2, L"--");
mouseButton button_RStickLeft_highlight(38, 11, 2, L" {");
mouseButton button_RStickRight_highlight(44, 11, 2, L"} ");
mouseButton button_RStickDown_highlight(41, 12, 2, L"--");

// Controller button outlines
mouseButton button_A_outline(50, 10, 4, L" .. |\t\t| ``", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_B_outline(53, 8, 4, L" .. |\t\t| ``", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_X_outline(46, 8, 4, L" .. |\t\t| ``", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_Y_outline(50, 6, 4, L" .. |\t\t| ``", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);

//mouseButton button_Back_outline(29, 8, 3, L"( )", UNCLICKABLE);
//mouseButton button_Start_outline(41, 8, 3, L"( )", UNCLICKABLE);
//mouseButton button_Guide_outline(35, 7, 3, L"[_]", UNCLICKABLE);
// Same as highlight

mouseButton button_DpadUp_outline(26, 9, 4, L"\t__\t|\t\t|", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadLeft_outline(23, 10, 3, L"\t__|\t\t", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadRight_outline(30, 10, 3, L"__\t\t\t|", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadDown_outline(26, 12, 4, L"|\t\t|", DEFAULT_TEXT, HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);

mouseButton button_L1_outline(17, 3, 10, L",\t______\t_\\________/", UNCLICKABLE);
mouseButton button_L2_outline(18, 2, 8, L" ______ /______\\", UNCLICKABLE);
mouseButton button_L3_outline(20, 7, 4, L".\t\t.\t\t\t\t'\t\t'", UNCLICKABLE);

//mouseButton button_LStickUp_outline(21, 7, 2, L"--", UNCLICKABLE);
//mouseButton button_LStickLeft_outline(18, 8, 2, L" {", UNCLICKABLE);
//mouseButton button_LStickRight_outline(24, 8, 2, L"} ", UNCLICKABLE);
//mouseButton button_LStickDown_outline(21, 9, 2, L"--", UNCLICKABLE);
// Same as highlight

mouseButton button_R1_outline(46, 3, 10, L"_\t______\t,\\________/", UNCLICKABLE);
mouseButton button_R2_outline(47, 2, 8, L" ______ /______\\", UNCLICKABLE);
mouseButton button_R3_outline(40, 10, 4, L".\t\t.\t\t\t\t'\t\t'", UNCLICKABLE);

//mouseButton button_RStickUp_outline(41, 10, 2, L"--", UNCLICKABLE);
//mouseButton button_RStickLeft_outline(38, 11, 2, L" {", UNCLICKABLE);
//mouseButton button_RStickRight_outline(44, 11, 2, L"} ", UNCLICKABLE);
//mouseButton button_RStickDown_outline(41, 12, 2, L"--", UNCLICKABLE);
// Same as highlight


// A map of controller ButtonNames to their associated mouseButtons
std::map<SDLButtonMapping::ButtonName, mouseButton*> buttonIdMap = {
    {SDLButtonMapping::ButtonName::DPAD_UP, &button_DpadUp_outline },
    {SDLButtonMapping::ButtonName::DPAD_DOWN, &button_DpadDown_outline},
    {SDLButtonMapping::ButtonName::DPAD_LEFT, &button_DpadLeft_outline},
    {SDLButtonMapping::ButtonName::DPAD_RIGHT, &button_DpadRight_outline},
    {SDLButtonMapping::ButtonName::START, &button_Start_highlight},
    {SDLButtonMapping::ButtonName::BACK, &button_Back_highlight},
    {SDLButtonMapping::ButtonName::LEFT_THUMB, &button_L3_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_THUMB, &button_R3_highlight},
    {SDLButtonMapping::ButtonName::LEFT_SHOULDER, &button_L1_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_SHOULDER, &button_R1_highlight},
    {SDLButtonMapping::ButtonName::GUIDE, &button_Guide_highlight},
    {SDLButtonMapping::ButtonName::A, &button_A_outline},
    {SDLButtonMapping::ButtonName::B, &button_B_outline},
    {SDLButtonMapping::ButtonName::X, &button_X_outline},
    {SDLButtonMapping::ButtonName::Y, &button_Y_outline},
    {SDLButtonMapping::ButtonName::LEFT_TRIGGER, &button_L2_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_TRIGGER, &button_R2_highlight},
    {SDLButtonMapping::ButtonName::LEFT_STICK_LEFT, &button_LStickLeft_highlight},
    {SDLButtonMapping::ButtonName::LEFT_STICK_UP, &button_LStickUp_highlight},
    {SDLButtonMapping::ButtonName::LEFT_STICK_RIGHT, &button_LStickRight_highlight},
    {SDLButtonMapping::ButtonName::LEFT_STICK_DOWN, &button_LStickDown_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_STICK_LEFT, &button_RStickLeft_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_STICK_UP, &button_RStickUp_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_STICK_RIGHT, &button_RStickRight_highlight},
    {SDLButtonMapping::ButtonName::RIGHT_STICK_DOWN, &button_RStickDown_highlight}
};


// ********************************
//  For managing controller face and shoulder buttons

// callback links buttons status with _partner // defined in joySender++.h, **FOR NOW**
void ShareButtonStatusCallback(mouseButton& button);

// Sets ids associated with SDLButtonMapping::ButtonName values for controller buttons
void SetControllerButtonIds() {
    // Highlight areas
    button_Back_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::BACK));
    button_Start_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::START));
    button_Guide_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::GUIDE));

    button_LStickUp_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_STICK_UP));
    button_LStickLeft_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_STICK_LEFT));
    button_LStickRight_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_STICK_RIGHT));
    button_LStickDown_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_STICK_DOWN));

    button_L1_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_SHOULDER));
    button_L2_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_TRIGGER));
    button_L3_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::LEFT_THUMB));

    button_RStickUp_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_STICK_UP));
    button_RStickLeft_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_STICK_LEFT));
    button_RStickRight_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_STICK_RIGHT));
    button_RStickDown_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_STICK_DOWN));

    button_R1_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_SHOULDER));
    button_R2_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_TRIGGER));
    button_R3_highlight.SetId(static_cast<int>(SDLButtonMapping::ButtonName::RIGHT_THUMB));

    // Outline areas
    button_A_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::A));
    button_B_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::B));
    button_X_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::X));
    button_Y_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::Y));
    button_DpadUp_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::DPAD_UP));
    button_DpadLeft_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::DPAD_LEFT));
    button_DpadRight_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::DPAD_RIGHT));
    button_DpadDown_outline.SetId(static_cast<int>(SDLButtonMapping::ButtonName::DPAD_DOWN));
}

// Run once to couple button outline <-> highlight areas // make color changes and set button id's
void CoupleControllerButtons() {
    button_A_outline.Couple(&button_A_highlight);
    button_A_outline.setCallback(ShareButtonStatusCallback);

    button_B_outline.Couple(&button_B_highlight);
    button_B_outline.setCallback(ShareButtonStatusCallback);

    button_X_outline.Couple(&button_X_highlight);
    button_X_outline.setCallback(ShareButtonStatusCallback);

    button_Y_outline.Couple(&button_Y_highlight);
    button_Y_outline.setCallback(ShareButtonStatusCallback);

    button_DpadUp_outline.Couple(&button_DpadUp_highlight);
    button_DpadUp_outline.setCallback(ShareButtonStatusCallback);

    button_DpadLeft_outline.Couple(&button_DpadLeft_highlight);
    button_DpadLeft_outline.setCallback(ShareButtonStatusCallback);

    button_DpadRight_outline.Couple(&button_DpadRight_highlight);
    button_DpadRight_outline.setCallback(ShareButtonStatusCallback);

    button_DpadDown_outline.Couple(&button_DpadDown_highlight);
    button_DpadDown_outline.setCallback(ShareButtonStatusCallback);

    button_L1_highlight.Couple(&button_L1_outline);
    button_L1_highlight.setCallback(ShareButtonStatusCallback);

    button_L2_highlight.Couple(&button_L2_outline);
    button_L2_highlight.setCallback(ShareButtonStatusCallback);

    button_L3_highlight.Couple(&button_L3_outline);
    button_L3_highlight.setCallback(ShareButtonStatusCallback);

    button_R1_highlight.Couple(&button_R1_outline);
    button_R1_highlight.setCallback(ShareButtonStatusCallback);

    button_R2_highlight.Couple(&button_R2_outline);
    button_R2_highlight.setCallback(ShareButtonStatusCallback);

    button_R3_highlight.Couple(&button_R3_outline);
    button_R3_highlight.setCallback(ShareButtonStatusCallback);

    // Some color changes are also necessary
    button_L1_outline.SetSelectColor(HOVERED_BUTTON);
    button_L2_outline.SetSelectColor(HOVERED_BUTTON);
    button_L3_outline.SetSelectColor(HOVERED_BUTTON);
    button_R1_outline.SetSelectColor(HOVERED_BUTTON);
    button_R2_outline.SetSelectColor(HOVERED_BUTTON);
    button_R3_outline.SetSelectColor(HOVERED_BUTTON);

    // might as well run this here it won't hurt anything :)
    SetControllerButtonIds();
}

// Sets position properties for controller buttons according to type
void SetControllerButtonPositions(byte controlerType) {
    if (controlerType == 1) {
        // Set positions for Xbox Layout 
        button_A_highlight.SetPosition(51, 11);
        button_B_highlight.SetPosition(54, 9);
        button_X_highlight.SetPosition(47, 9);
        button_Y_highlight.SetPosition(51, 7);

        button_Back_highlight.SetPosition(29, 8);
        button_Start_highlight.SetPosition(41, 8);
        button_Guide_highlight.SetPosition(35, 7);

        button_DpadUp_highlight.SetPosition(27, 10);
        button_DpadLeft_highlight.SetPosition(24, 11);
        button_DpadRight_highlight.SetPosition(30, 11);
        button_DpadDown_highlight.SetPosition(27, 12);

        button_L1_highlight.SetPosition(18, 4);
        button_L2_highlight.SetPosition(19, 3);
        button_L3_highlight.SetPosition(21, 8);

        button_LStickUp_highlight.SetPosition(21, 7);
        button_LStickLeft_highlight.SetPosition(18, 8);
        button_LStickRight_highlight.SetPosition(24, 8);
        button_LStickDown_highlight.SetPosition(21, 9);

        button_R1_highlight.SetPosition(47, 4);
        button_R2_highlight.SetPosition(48, 3);
        button_R3_highlight.SetPosition(41, 11);

        button_RStickUp_highlight.SetPosition(41, 10);
        button_RStickLeft_highlight.SetPosition(38, 11);
        button_RStickRight_highlight.SetPosition(44, 11);
        button_RStickDown_highlight.SetPosition(41, 12);

        button_A_outline.SetPosition(50, 10);
        button_B_outline.SetPosition(53, 8);
        button_X_outline.SetPosition(46, 8);
        button_Y_outline.SetPosition(50, 6);

        button_DpadUp_outline.SetPosition(26, 9);
        button_DpadLeft_outline.SetPosition(23, 10);
        button_DpadRight_outline.SetPosition(30, 10);
        button_DpadDown_outline.SetPosition(26, 12);

        button_L1_outline.SetPosition(17, 3);
        button_L2_outline.SetPosition(18, 2);
        button_L3_outline.SetPosition(20, 7);

        button_R1_outline.SetPosition(46, 3);
        button_R2_outline.SetPosition(47, 2);
        button_R3_outline.SetPosition(40, 10);

    }

    else if (controlerType == 2) {
        // Load DS4 positions
        button_A_highlight.SetPosition(53, 11);
        button_B_highlight.SetPosition(56, 9);
        button_X_highlight.SetPosition(49, 9);
        button_Y_highlight.SetPosition(53, 7);

        button_Back_highlight.SetPosition(27, 7);
        button_Start_highlight.SetPosition(43, 7);
        button_Guide_highlight.SetPosition(35, 11);

        button_DpadUp_highlight.SetPosition(19, 7);
        button_DpadLeft_highlight.SetPosition(16, 8);
        button_DpadRight_highlight.SetPosition(22, 8);
        button_DpadDown_highlight.SetPosition(19, 9);

        button_L1_highlight.SetPosition(16, 4);
        button_L2_highlight.SetPosition(17, 3);
        button_L3_highlight.SetPosition(27, 11);

        button_LStickUp_highlight.SetPosition(27, 10);
        button_LStickLeft_highlight.SetPosition(24, 11);
        button_LStickRight_highlight.SetPosition(30, 11);
        button_LStickDown_highlight.SetPosition(27, 12);

        button_R1_highlight.SetPosition(49, 4);
        button_R2_highlight.SetPosition(50, 3);
        button_R3_highlight.SetPosition(44, 11);

        button_RStickUp_highlight.SetPosition(44, 10);
        button_RStickLeft_highlight.SetPosition(41, 11);
        button_RStickRight_highlight.SetPosition(47, 11);
        button_RStickDown_highlight.SetPosition(44, 12);

        button_A_outline.SetPosition(52, 10);
        button_B_outline.SetPosition(55, 8);
        button_X_outline.SetPosition(48, 8);
        button_Y_outline.SetPosition(52, 6);

        button_DpadUp_outline.SetPosition(18, 6);
        button_DpadLeft_outline.SetPosition(15, 7);
        button_DpadRight_outline.SetPosition(22, 7);
        button_DpadDown_outline.SetPosition(18, 9);

        button_L1_outline.SetPosition(15, 3);
        button_L2_outline.SetPosition(16, 2);
        button_L3_outline.SetPosition(26, 10);

        button_R1_outline.SetPosition(48, 3);
        button_R2_outline.SetPosition(49, 2);
        button_R3_outline.SetPosition(43, 10);
    }
}

// Adds Controller buttons to the textUI object // highlights and outlines
void AddControllerButtons(textUI& screen) {

    // Highlight areas
    screen.AddButton(&button_Back_highlight);
    screen.AddButton(&button_Start_highlight);
    screen.AddButton(&button_Guide_highlight);

    screen.AddButton(&button_LStickUp_highlight);
    screen.AddButton(&button_LStickLeft_highlight);
    screen.AddButton(&button_LStickRight_highlight);
    screen.AddButton(&button_LStickDown_highlight);

    screen.AddButton(&button_L1_highlight);
    screen.AddButton(&button_L2_highlight);
    screen.AddButton(&button_L3_highlight);

    screen.AddButton(&button_RStickUp_highlight);
    screen.AddButton(&button_RStickLeft_highlight);
    screen.AddButton(&button_RStickRight_highlight);
    screen.AddButton(&button_RStickDown_highlight);

    screen.AddButton(&button_R1_highlight);
    screen.AddButton(&button_R2_highlight);
    screen.AddButton(&button_R3_highlight);

    // Outline areas
    screen.AddButton(&button_A_outline);
    screen.AddButton(&button_B_outline);
    screen.AddButton(&button_X_outline);
    screen.AddButton(&button_Y_outline);
    screen.AddButton(&button_DpadUp_outline);
    screen.AddButton(&button_DpadLeft_outline);
    screen.AddButton(&button_DpadRight_outline);
    screen.AddButton(&button_DpadDown_outline);

}

