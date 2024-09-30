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
#include <random>

// Foreground (Text) Colors
enum ConsoleColor {
	BLACK,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	YELLOW,
	BRIGHT_GREY,
	GREY,
	BRIGHT_BLUE,
	BRIGHT_GREEN,
	BRIGHT_CYAN,
	BRIGHT_RED,
	BRIGHT_MAGENTA,
	BRIGHT_YELLOW,
	WHITE
};

// bitwise color operations
#define FG_ONLY         & 0x000F
#define BG_ONLY         & 0x00F0
#define DROP_BG         & 0xFF0F
#define AS_BG			<< 4
#define AS_FG			>> 4

// Package up all the colors needed for bare minimum color expression
struct ColorScheme {
	WORD outlineColor;
	WORD faceColor;
	WORD buttonColor;
	WORD highlightColor;
	WORD selectColor;
};

// Generate a random integer from r1 - r2 inclusive
int generateRandomInt(int r1, int r2) {
	// Create a random number generator engine
	std::random_device rd;
	std::mt19937 gen(rd());
	// Create a distribution to generate integers between r1 and r2
	std::uniform_int_distribution<> dist(r1, r2);
	// Generate and return a random number
	return dist(gen);
}

// Function Compares fg to bg, returns true if there is bad contrast
bool CheckContrastMismatch(WORD fg, WORD bg) {
	static std::map<byte, std::set<byte>> conflictingColors = {
		// Define Conflicting color pairs
		{BLACK,			{ BLACK } },
		{BLUE, 			{ BLUE,		BLACK,	BRIGHT_GREEN }},
		{GREEN, 		{ GREEN,	CYAN,	YELLOW,		BRIGHT_CYAN,	BRIGHT_RED }},
		{CYAN, 			{ CYAN,		GREEN,	BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_RED,		BRIGHT_GREEN,	BRIGHT_CYAN }},
		{RED, 			{ RED,		GREY,	MAGENTA,	 BRIGHT_RED,	BRIGHT_BLUE,	BRIGHT_MAGENTA }},
		{MAGENTA, 		{ MAGENTA,	RED,	BRIGHT_BLUE, BRIGHT_MAGENTA }},
		{YELLOW, 		{ YELLOW,	GREEN,	BRIGHT_GREEN }},
		{BRIGHT_GREY, 	{ BRIGHT_GREY,	CYAN, BRIGHT_BLUE,	BRIGHT_GREEN,	BRIGHT_CYAN,	WHITE,	YELLOW, GREY }},
		{GREY, 			{ GREY,		CYAN,	RED,	BRIGHT_BLUE,	BRIGHT_GREY,	BRIGHT_RED,		BRIGHT_MAGENTA }},
		{BRIGHT_BLUE, 	{ BRIGHT_BLUE,	 CYAN,	GREY,	 BRIGHT_GREY,	 BRIGHT_RED,	 BRIGHT_MAGENTA,	 BRIGHT_YELLOW }},
		{BRIGHT_GREEN, 	{ BRIGHT_GREEN,	 BRIGHT_GREY,	 YELLOW,	CYAN,	BRIGHT_CYAN }},
		{BRIGHT_CYAN, 	{ BRIGHT_CYAN, YELLOW,  BRIGHT_BLUE,	BRIGHT_GREEN,	CYAN,	WHITE,	BRIGHT_GREY }},
		{BRIGHT_RED, 	{ BRIGHT_RED,	BRIGHT_BLUE,  BRIGHT_MAGENTA,	 BRIGHT_GREY,	BRIGHT_RED,	 CYAN,	 GREY,	YELLOW }},
		{BRIGHT_MAGENTA,{ BRIGHT_MAGENTA,	 BRIGHT_BLUE,	BRIGHT_RED,		CYAN,	RED,	MAGENTA }},
		{BRIGHT_YELLOW, { BRIGHT_YELLOW,	 WHITE }},
		{WHITE, 		{ WHITE,	BRIGHT_YELLOW,	BRIGHT_GREEN,	 BRIGHT_GREY,	BRIGHT_CYAN }},
	};

	for (byte conflictingColor : conflictingColors[fg FG_ONLY]) {
		if (conflictingColor == (bg >> 4))
			return 1;
	}

	return 0;
}

// Function checks if FG is same as BG
bool sameFG_BG(WORD fg, WORD bg) {
	return (fg & 0x000f) == ((bg & 0x00f0) >> 4);
}

// Function can switch color of FG 
WORD inverseFGColor(WORD color) {
	byte fg_color = color FG_ONLY;
	WORD inverse = 0xF ^ fg_color;
	color &= ~fg_color;

	inverse |= color BG_ONLY;

	return inverse;
}

// Returns a contrast safe foreground color based on input color
WORD findSafeFGColor(WORD color, const std::vector<WORD>& colorList, std::vector<WORD>::const_iterator it) {
	if (it != colorList.end()) {
		WORD safe_color = *it;
		if (CheckContrastMismatch(safe_color, color)) {
			return findSafeFGColor(color, colorList, ++it);
		}
		else {
			return safe_color;
		}
	}
	return BLACK; // Default fallback
}

// take a console text color and make the FG and BG distinct
WORD makeSafeColors(WORD inputColor) {
	WORD fg = inputColor FG_ONLY;
	WORD bg = inputColor BG_ONLY;

	// A list of replacement colors to select from
	std::vector<WORD> colorList;
	// populate the colors list
	{
		int colorseed = generateRandomInt(1100000000, 2147483647);

		// Loop through each byte of the integer
		for (int byteIndex = 0; byteIndex < sizeof(int); ++byteIndex) {
			unsigned char byteValue = (colorseed >> (byteIndex * 8)) & 0xFF; // Extract a byte

			// Reading four bits at a time within each byte
			for (int nibbleIndex = 0; nibbleIndex < 2; ++nibbleIndex) {
				unsigned char nibble = (byteValue >> (nibbleIndex * 4)) & 0x0F; // Extract four bits
				colorList.push_back(nibble);	// use 4 bits as color value
			}
		}
		colorList.push_back(WHITE);  // If both white and black fail to pass
		colorList.push_back(BLACK);  // CheckContrastMismatch , sumpin' broken
	}

	WORD safe_col = inputColor;
	if (sameFG_BG(fg, bg)) {
		safe_col = inverseFGColor(inputColor);
	}

	if (CheckContrastMismatch(safe_col, bg)) {
		safe_col = findSafeFGColor(bg, colorList, colorList.begin()) | bg;
	}

	return safe_col;
}

// Generates a contrast safe color scheme for a controller face
ColorScheme createRandomScheme() {
	// A list of replacement colors to select from
	std::vector<WORD> colorList;

	// populate the colors list
	{
		int colorseed = generateRandomInt(1100000000, 2147483647);

		// Loop through each byte of the integer
		for (int byteIndex = 0; byteIndex < sizeof(int); ++byteIndex) {
			unsigned char byteValue = (colorseed >> (byteIndex * 8)) & 0xFF; // Extract a byte

			// Reading four bits at a time within each byte
			for (int nibbleIndex = 0; nibbleIndex < 2; ++nibbleIndex) {
				unsigned char nibble = (byteValue >> (nibbleIndex * 4)) & 0x0F; // Extract four bits
				colorList.push_back(nibble);
			}
		}
	}

	ColorScheme randScheme = { 0 };
	randScheme.outlineColor = static_cast<WORD>(generateRandomInt(0, 15));

	randScheme.faceColor = static_cast<WORD>(generateRandomInt(0, 15)) << 4;

	randScheme.buttonColor = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin());
	colorList.erase(colorList.begin());

	randScheme.highlightColor = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin());
	colorList.erase(colorList.begin());

	for (int i = 0; i < 15; ++i) {
		if (!sameFG_BG(randScheme.buttonColor, randScheme.highlightColor << 4))
			break;
		randScheme.highlightColor = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin());
		colorList.push_back(static_cast<WORD>(generateRandomInt(0, 15)));
	}
	colorList.erase(colorList.begin());

	// select bg
	WORD sBG = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin()) << 4;
	colorList.erase(colorList.begin());

	for (int i = 0; i < 15; ++i) {
		if (!sameFG_BG(randScheme.faceColor >> 4, sBG))
			break;
		sBG = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin()) << 4;
		colorList.push_back(static_cast<WORD>(generateRandomInt(0, 15)));
	}

	// select fg
	WORD sFG= findSafeFGColor(randScheme.faceColor, colorList, colorList.begin());
	colorList.erase(colorList.begin());

	for (int i = 0; i < 15; ++i) {
		if (!sameFG_BG(sFG, randScheme.faceColor))
			break;
		sFG = findSafeFGColor(randScheme.faceColor, colorList, colorList.begin());
		colorList.push_back(static_cast<WORD>(generateRandomInt(0, 15)));
	}


	randScheme.selectColor = sFG | sBG;

	return randScheme;
}