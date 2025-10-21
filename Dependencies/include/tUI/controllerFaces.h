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

#include "ColorSchemes.h"

constexpr int XBOX_FACE_LINES = 17;
constexpr int DS4_FACE_LINES = 17;

// defines the area to be colored in for the controller face
textBox ControllerFace[XBOX_FACE_LINES];

// Sets the ControllerFace array to the Xbox controller configuration
void BuildXboxFace() {
	ControllerFace[0] = textBox(17, 5, 40, 1, ALIGN_LEFT, L"                                       ", GREY AS_BG);
	ControllerFace[1] = textBox(16, 6, 42, 1, ALIGN_LEFT, L"                                         ", GREY AS_BG);
	ControllerFace[2] = textBox(15, 7, 44, 1, ALIGN_LEFT, L"                                           ", GREY AS_BG);
	ControllerFace[3] = textBox(14, 8, 46, 1, ALIGN_LEFT, L"                                             ", GREY AS_BG);
	ControllerFace[4] = textBox(13, 9, 48, 1, ALIGN_LEFT, L"                                               ", GREY AS_BG);
	ControllerFace[5] = textBox(12, 10, 50, 1, ALIGN_LEFT, L"                                                 ", GREY AS_BG);
	ControllerFace[6] = textBox(11, 11, 52, 1, ALIGN_LEFT, L"                                                   ", GREY AS_BG);
	ControllerFace[7] = textBox(10, 12, 54, 1, ALIGN_LEFT, L"                                                     ", GREY AS_BG);
	ControllerFace[8] = textBox(9, 13, 56, 1, ALIGN_LEFT, L"                                                       ", GREY AS_BG);

	ControllerFace[9] = textBox(9, 14, 13, 1, ALIGN_LEFT, L"            ", GREY AS_BG);
	ControllerFace[10] = textBox(52, 14, 13, 1, ALIGN_LEFT, L"            ", GREY AS_BG);

	ControllerFace[11] = textBox(9, 15, 10, 1, ALIGN_LEFT, L"          ", GREY AS_BG);
	ControllerFace[12] = textBox(54, 15, 10, 1, ALIGN_LEFT, L"          ", GREY AS_BG);

	ControllerFace[13] = textBox(9, 16, 8, 1, ALIGN_LEFT, L"        ", GREY AS_BG);
	ControllerFace[14] = textBox(56, 16, 8, 1, ALIGN_LEFT, L"        ", GREY AS_BG);

	ControllerFace[15] = textBox(9, 17, 7, 1, ALIGN_LEFT, L".     ,", GREY AS_BG);
	ControllerFace[16] = textBox(57, 17, 7, 1, ALIGN_LEFT, L".     ,", GREY AS_BG);
}

// Sets the ControllerFace array to the DS4 controller configuration
void BuildDS4Face() {
	ControllerFace[0] = textBox(14, 5, 45, 1, ALIGN_LEFT, L"                                             ", GREY AS_BG);
	ControllerFace[1] = textBox(13, 6, 47, 1, ALIGN_LEFT, L"                                               ", GREY AS_BG);
	ControllerFace[2] = textBox(12, 7, 49, 1, ALIGN_LEFT, L"                                                 ", GREY AS_BG);
	ControllerFace[3] = textBox(11, 8, 51, 1, ALIGN_LEFT, L"/                                                 \\", GREY AS_BG);
	ControllerFace[4] = textBox(11, 9, 51, 1, ALIGN_LEFT, L"                                                   ", GREY AS_BG);
	ControllerFace[5] = textBox(10, 10, 53, 1, ALIGN_LEFT, L"'                                                   '", GREY AS_BG);
	ControllerFace[6] = textBox(9, 11, 55, 1, ALIGN_LEFT, L"/                                                     \\", GREY AS_BG);

	ControllerFace[7] = textBox(9, 12, 35, 1, ALIGN_LEFT, L"                       /^^^^^^^", GREY AS_BG);
	ControllerFace[8] = textBox(40, 12, 25, 1, ALIGN_LEFT, L"\\                       ", GREY AS_BG);

	ControllerFace[9] = textBox(9, 13, 14, 1, ALIGN_LEFT, L"              ", GREY AS_BG);
	ControllerFace[10] = textBox(50, 13, 14, 1, ALIGN_LEFT, L"              ", GREY AS_BG);

	ControllerFace[11] = textBox(9, 14, 13, 1, ALIGN_LEFT, L"             ", GREY AS_BG);
	ControllerFace[12] = textBox(51, 14, 13, 1, ALIGN_LEFT, L"             ", GREY AS_BG);

	ControllerFace[13] = textBox(9, 15, 12, 1, ALIGN_LEFT, L"            ", GREY AS_BG);
	ControllerFace[14] = textBox(52, 15, 12, 1, ALIGN_LEFT, L"            ", GREY AS_BG);

	ControllerFace[15] = textBox(10, 16, 10, 1, ALIGN_LEFT, L".        .", GREY AS_BG);
	ControllerFace[16] = textBox(53, 16, 10, 1, ALIGN_LEFT, L".        ,", GREY AS_BG);
}

// Handles all details of color setting coupled buttons and contrast correcting shoulder buttons
void SetCoupledButtonColors(WORD bgcol, WORD fcol, WORD dcol, WORD hcol, WORD scol) {
	// A list of replacement colors to select from
	std::vector<WORD> colorList = {			// new color selection is as follows:
	static_cast<WORD>(generateRandomInt(0,15)),			// random color
	static_cast<WORD>((fcol FG_ONLY)),			// controller outline
	static_cast<WORD>((fcol BG_ONLY) >> 4),		// controller face
	WHITE,									// white
	inverseFGColor(bgcol >> 4),			// inverse of bg
	inverseFGColor((fcol BG_ONLY) >> 4),			// inverse of controller face
	BLACK									// black
	};

	// FG color won't match bg color
	auto findSafeColor = [&](WORD color) -> WORD {
		if (sameFG_BG(color, bgcol)) {
			return inverseFGColor(color);
		}
		return color;
	};

	// safe colors for shoulders
	WORD shoulder_dcol, shoulder_hcol, shoulder_scol;

	//color conversion if bg matches fg on shoulder buttons
	shoulder_dcol = findSafeColor(dcol);
	shoulder_hcol = findSafeColor(hcol);

	shoulder_scol = (findSafeColor(scol) FG_ONLY) | (findSafeColor((scol BG_ONLY) >> 4) << 4);

	// recursive implementation default
	if (CheckContrastMismatch(shoulder_dcol, bgcol)) {
		shoulder_dcol = findSafeFGColor(bgcol, colorList, colorList.begin()) | (shoulder_dcol BG_ONLY);
	}

	// recursive implementation highlight
	if (CheckContrastMismatch(shoulder_hcol, bgcol)) {
		shoulder_hcol = findSafeFGColor(bgcol, colorList, colorList.begin()) | (shoulder_hcol BG_ONLY);
	}

	// recursive implementation select
	if (CheckContrastMismatch(shoulder_scol, bgcol)) {
		shoulder_scol = findSafeFGColor(bgcol, colorList, colorList.begin()) | (shoulder_scol BG_ONLY);
	}


	button_L1_highlight.SetDefaultColor(shoulder_dcol | bgcol);
	button_L1_highlight.SetHighlightColor(shoulder_hcol | bgcol);
	button_L1_highlight.SetSelectColor(shoulder_scol);
	button_L2_highlight.SetDefaultColor(shoulder_dcol | bgcol);
	button_L2_highlight.SetHighlightColor(shoulder_hcol | bgcol);
	button_L2_highlight.SetSelectColor(shoulder_scol);

	button_L1_outline.SetSelectColor((shoulder_scol FG_ONLY) | bgcol);
	button_L2_outline.SetSelectColor((shoulder_scol FG_ONLY) | bgcol);

	button_R1_highlight.SetDefaultColor(shoulder_dcol | bgcol);
	button_R1_highlight.SetHighlightColor(shoulder_hcol | bgcol);
	button_R1_highlight.SetSelectColor(shoulder_scol);
	button_R2_highlight.SetDefaultColor(shoulder_dcol | bgcol);
	button_R2_highlight.SetHighlightColor(shoulder_hcol | bgcol);
	button_R2_highlight.SetSelectColor(shoulder_scol);

	button_R1_outline.SetSelectColor((shoulder_scol FG_ONLY) | bgcol);
	button_R2_outline.SetSelectColor((shoulder_scol FG_ONLY) | bgcol);
	//
	button_L3_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_R3_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));

	//
	button_A_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_A_highlight.SetSelectColor(scol);

	button_B_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_B_highlight.SetSelectColor(scol);

	button_X_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_X_highlight.SetSelectColor(scol);

	button_Y_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_Y_highlight.SetSelectColor(scol);

	//
	button_DpadUp_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_DpadUp_highlight.SetSelectColor(hcol | (scol BG_ONLY));

	button_DpadLeft_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_DpadLeft_highlight.SetSelectColor(hcol | (scol BG_ONLY));

	button_DpadRight_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_DpadRight_highlight.SetSelectColor(hcol | (scol BG_ONLY));

	button_DpadDown_outline.SetSelectColor((scol FG_ONLY) | (fcol BG_ONLY));
	button_DpadDown_highlight.SetSelectColor(hcol | (scol BG_ONLY));

}

void SetControllerFace(textUI& screen, ColorScheme& colorsScheme, WORD BG_COLOR, int mode, bool noBG = false) {
	if (mode == 1) {
		screen.SetBackdrop(XBOX_Backdrop);
	}
	else {
		screen.SetBackdrop(DS4_Backdrop);
	}

	// Set the background colors
	screen.SetBackdropColor(colorsScheme.outlineColor | BG_COLOR);

	// do customized button effect colors & // Shoulder button contrast correction
	SetCoupledButtonColors(
		BG_COLOR,
		(colorsScheme.outlineColor | colorsScheme.faceColor),
		colorsScheme.buttonColor,
		colorsScheme.highlightColor,
		colorsScheme.selectColor);
}

void ReDrawControllerFace(textUI& screen, ColorScheme& colorsScheme, WORD BG_COLOR, int mode, bool noBG = false) {
	int faceLines;
	if (mode == 1) {
		faceLines = XBOX_FACE_LINES;
	}
	else {
		faceLines = DS4_FACE_LINES;
	}

	// make sure detail on controller face is visible
	WORD safe_col = colorsScheme.outlineColor;
	if (CheckContrastMismatch(colorsScheme.outlineColor, colorsScheme.faceColor)) {
		safe_col = BG_COLOR >> 4;
	}
	while (CheckContrastMismatch(safe_col, colorsScheme.faceColor)) {
		safe_col = (safe_col + 1) % 15;
	}

	// Set controller buttons to color scheme
	textUI controllerButtons;		// for setting button colors
	AddControllerButtons(controllerButtons);	// without them being click able on screen

	if (mode == 2) controllerButtons.AddButton(&button_DS4_TouchPad_highlight);
	controllerButtons.SetButtonsColors({ static_cast<WORD>(colorsScheme.buttonColor | colorsScheme.faceColor),
								static_cast<WORD>(colorsScheme.highlightColor | colorsScheme.faceColor),
								colorsScheme.selectColor,
								0x0000  // unused
		});
	
	// do customized button effect colors & // Shoulder button contrast correction
	SetCoupledButtonColors(
		BG_COLOR,
		(colorsScheme.outlineColor | colorsScheme.faceColor),
		colorsScheme.buttonColor,
		colorsScheme.highlightColor,
		colorsScheme.selectColor);

	if (noBG)
		screen.DrawBackdropClearWhitespace();
	else
		screen.DrawBackdrop();

	// Draw face
	setTextColor(safe_col | colorsScheme.faceColor);
	for (int i = 0; i < faceLines; i++) {
		ControllerFace[i].Draw_noColor();
	}

	controllerButtons.DrawButtons();
}