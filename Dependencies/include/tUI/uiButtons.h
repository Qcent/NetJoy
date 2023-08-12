#pragma once
#include "textUI.hpp"

// lets make a quit button
mouseButton quitButton(consoleWidth / 2 - 5, 17, 11, L" (Q) Quit  ");


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
mouseButton button_A_outline(50, 10, 4, L" .. |\t\t| ``", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_B_outline(53, 8, 4, L" .. |\t\t| ``", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_X_outline(46, 8, 4, L" .. |\t\t| ``", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_Y_outline(50, 6, 4, L" .. |\t\t| ``", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);

//mouseButton button_Back_outline(29, 8, 3, L"( )", UNCLICKABLE);
//mouseButton button_Start_outline(41, 8, 3, L"( )", UNCLICKABLE);
//mouseButton button_Guide_outline(35, 7, 3, L"[_]", UNCLICKABLE);
// Same as highlight

mouseButton button_DpadUp_outline(26, 9, 4, L"\t__\t|\t\t|", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadLeft_outline(23, 10, 3, L"\t__|\t\t", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadRight_outline(30, 10, 3, L"__\t\t\t|", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);
mouseButton button_DpadDown_outline(26, 12, 4, L"|\t\t|", HOVERED_BUTTON, HOVERED_BUTTON, DEFAULT_TEXT);

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


// Callback function to link buttons status
void ShareButtonStatus(mouseButton& button) {
    auto other = button.GetPartner();
    other->SetStatus(button.Status());
    other->Update();
}

// Sets position properties for controller buttons according to type
void SetButtonPositions(byte controlerType) {
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

// Loads controller button Highlight areas into screen
void LoadButtonHighlightAreas(textUI& screen) {

    //screen.AddButton(&button_A_highlight);
    //screen.AddButton(&button_B_highlight);
    //screen.AddButton(&button_X_highlight);
    //screen.AddButton(&button_Y_highlight);
    
    //screen.AddButton(&button_DpadUp_highlight);
    //screen.AddButton(&button_DpadLeft_highlight);
    //screen.AddButton(&button_DpadRight_highlight);
    //screen.AddButton(&button_DpadDown_highlight);
     
    // Above is 'Coupled' to share status with outline

    screen.AddButton(&button_Back_highlight);
    screen.AddButton(&button_Start_highlight);
    screen.AddButton(&button_Guide_highlight);

    screen.AddButton(&button_L1_highlight);
    screen.AddButton(&button_L2_highlight);
    screen.AddButton(&button_L3_highlight);

    screen.AddButton(&button_LStickUp_highlight);
    screen.AddButton(&button_LStickLeft_highlight);
    screen.AddButton(&button_LStickRight_highlight);
    screen.AddButton(&button_LStickDown_highlight);

    screen.AddButton(&button_R1_highlight);
    screen.AddButton(&button_R2_highlight);
    screen.AddButton(&button_R3_highlight);

    screen.AddButton(&button_RStickUp_highlight);
    screen.AddButton(&button_RStickLeft_highlight);
    screen.AddButton(&button_RStickRight_highlight);
    screen.AddButton(&button_RStickDown_highlight);

 }

// Loads controller button outline areas into screen
void LoadButtonOutlines(textUI& screen) {

    screen.AddButton(&button_A_outline);
    button_A_outline.Couple(&button_A_highlight);
    button_A_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_B_outline);
    button_B_outline.Couple(&button_B_highlight);
    button_B_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_X_outline);
    button_X_outline.Couple(&button_X_highlight);
    button_X_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_Y_outline);
    button_Y_outline.Couple(&button_Y_highlight);
    button_Y_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_DpadUp_outline);
    button_DpadUp_outline.Couple(&button_DpadUp_highlight);
    button_DpadUp_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_DpadLeft_outline);
    button_DpadLeft_outline.Couple(&button_DpadLeft_highlight);
    button_DpadLeft_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_DpadRight_outline);
    button_DpadRight_outline.Couple(&button_DpadRight_highlight);
    button_DpadRight_outline.setCallback(ShareButtonStatus);

    screen.AddButton(&button_DpadDown_outline);
    button_DpadDown_outline.Couple(&button_DpadDown_highlight);
    button_DpadDown_outline.setCallback(ShareButtonStatus);


    //screen.AddButton(&button_L1_outline);
    button_L1_highlight.Couple(&button_L1_outline);
    button_L1_highlight.setCallback(ShareButtonStatus);
    button_L1_outline.SetSelectColor(HOVERED_BUTTON);

    //screen.AddButton(&button_L2_outline);
    button_L2_highlight.Couple(&button_L2_outline);
    button_L2_highlight.setCallback(ShareButtonStatus);
    button_L2_outline.SetSelectColor(HOVERED_BUTTON);

    //screen.AddButton(&button_L3_outline);
    button_L3_highlight.Couple(&button_L3_outline);
    button_L3_highlight.setCallback(ShareButtonStatus);
    button_L3_outline.SetSelectColor(HOVERED_BUTTON);

    //screen.AddButton(&button_R1_outline);
    button_R1_highlight.Couple(&button_R1_outline);
    button_R1_highlight.setCallback(ShareButtonStatus);
    button_R1_outline.SetSelectColor(HOVERED_BUTTON);

    //screen.AddButton(&button_R2_outline);
    button_R2_highlight.Couple(&button_R2_outline);
    button_R2_highlight.setCallback(ShareButtonStatus);
    button_R2_outline.SetSelectColor(HOVERED_BUTTON);

    //screen.AddButton(&button_R3_outline);
    button_R3_highlight.Couple(&button_R3_outline);
    button_R3_highlight.setCallback(ShareButtonStatus);
    button_R3_outline.SetSelectColor(HOVERED_BUTTON);
}

// Initializes controller buttons into screen UI
void LoadButtons(textUI& screen, byte controlerType=0) {

    LoadButtonHighlightAreas(screen);
    LoadButtonOutlines(screen); 

    SetButtonPositions(controlerType);
}

