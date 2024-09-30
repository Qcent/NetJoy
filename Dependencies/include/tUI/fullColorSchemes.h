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

// Package up all the colors needed for maximum color expression
struct FullColorScheme {
    const wchar_t* name;
    tUIColorPkg menuColors;
    tUIColorPkg controllerColors;
    WORD menuBg;
    WORD controllerBg;
    FullColorScheme()
        : name(L"Default"),
        menuColors({ 0, 0, 0, 0 }),
        controllerColors({ 0, 0, 0, 0 }),
        menuBg(0),
        controllerBg(0) {}

    FullColorScheme(const wchar_t* n, tUIColorPkg mColors, tUIColorPkg cColors, WORD mBg, WORD cBg)
        : name(n), menuColors(mColors), controllerColors(cColors), menuBg(mBg), controllerBg(cBg) {}

    bool operator==(const FullColorScheme& other) const {
        if (!(this->menuColors == other.menuColors)) {
            return false;
        }
        if (!(this->controllerColors == other.controllerColors)) {
            return false;
        }

        if (this->menuBg != other.menuBg) {
            return false;
        }
        if (this->controllerBg != other.controllerBg) {
            return false;
        }

        return true;
    }
};

#define NUM_COLOR_SCHEMES   5   // number of defined color schemes + a place for a random scheme
#define RANDOMSCHEME        0   // index to store a randomly generated color scheme

FullColorScheme fullColorSchemes[NUM_COLOR_SCHEMES + 1] = {
    { L"Random",
        {
        // Menu Colors
        WHITE | (MAGENTA AS_BG),			// Header
        WHITE | (RED AS_BG),		        // Error
        WHITE | (CYAN AS_BG),				// message 1
        WHITE | (BRIGHT_RED AS_BG)			// message 2
    },
    {
        // Controller Colors
        BLACK | (BRIGHT_MAGENTA AS_BG),	    // outline | faceColor 
        WHITE | (BRIGHT_MAGENTA AS_BG),		// buttonColor
        BLUE | (BRIGHT_MAGENTA AS_BG),	    // highlightColor
        WHITE | (CYAN AS_BG)	            // selectColor
    },
    // Menu background
    WHITE | (BRIGHT_RED AS_BG),
    // Controller background
    BLACK | (MAGENTA AS_BG),
},


{ L"Plum",
    {
        // Menu Colors
        WHITE | (MAGENTA AS_BG),			// Header
        WHITE | (RED AS_BG),		        // Error
        WHITE | (CYAN AS_BG),				// message 1
        WHITE | (BRIGHT_RED AS_BG)			// message 2
    },
        {
            // Controller Colors
            BLACK | (BRIGHT_MAGENTA AS_BG),	    // outline | faceColor 
            WHITE | (BRIGHT_MAGENTA AS_BG),		// buttonColor
            BLUE | (BRIGHT_MAGENTA AS_BG),	    // highlightColor
            WHITE | (CYAN AS_BG)	            // selectColor
        },
    // Menu background
    WHITE | (BRIGHT_RED AS_BG),
    // Controller background
    BLACK | (MAGENTA AS_BG),
},


 { L"Watermelon",
    {
        // Menu Colors
        BLACK | (GREEN AS_BG),			// Header
        WHITE | (RED AS_BG),		    // Error
        BLACK | (BRIGHT_CYAN AS_BG),	// message 1
        BLUE | (BRIGHT_GREEN AS_BG)		// message 2
    },
        {
            // Controller Colors
            BRIGHT_GREEN | (BRIGHT_RED AS_BG),	// outline | faceColor 
            BLACK | (BRIGHT_RED AS_BG),		    // buttonColor
            BRIGHT_GREEN | (BRIGHT_RED AS_BG),  // highlightColor
            BLACK | (BRIGHT_GREEN AS_BG)        // selectColor
        },
    // Menu background
    BLACK | (CYAN AS_BG),
    // Controller background
    BRIGHT_GREEN | (BRIGHT_BLUE AS_BG),
},


 { L"Blueberry",
    {
        // Menu Colors
        WHITE | (BRIGHT_MAGENTA AS_BG),		// Header
        WHITE | (RED AS_BG),		        // Error
        BLUE | (BRIGHT_CYAN AS_BG),			// message 1
        BRIGHT_CYAN | (MAGENTA AS_BG)		// message 2
    },
        {
            // Controller Colors
            CYAN | (CYAN AS_BG),	        // outline | faceColor 
            WHITE | (CYAN AS_BG),		    // buttonColor
            BLUE | (CYAN AS_BG),            // highlightColor
            BRIGHT_CYAN | (BRIGHT_MAGENTA AS_BG)  // selectColor
        },
    // Menu background
    CYAN | (MAGENTA AS_BG),
    // Controller background
    CYAN | (BLUE AS_BG),
},


{ L"Citrus",
   {
        // Menu Colors
        BRIGHT_BLUE | (BRIGHT_GREEN AS_BG),	// Header
        WHITE | (RED AS_BG),		        // Error
        BLACK | (BRIGHT_CYAN AS_BG),	// message 1
        WHITE | (CYAN AS_BG)			    // message 2
    },
    {
        // Controller Colors
        BRIGHT_GREEN | (BRIGHT_GREEN AS_BG),	// outline | faceColor 
        BLUE | (BRIGHT_GREEN AS_BG),		    // buttonColor
        WHITE | (BRIGHT_GREEN AS_BG),           // highlightColor
        WHITE | (YELLOW AS_BG)      // selectColor
    },
    // Menu background
    BRIGHT_BLUE | (BRIGHT_GREEN AS_BG),
    // Controller background
    BRIGHT_GREEN | (BRIGHT_RED AS_BG),
},


{ L"Emily's Bougie Baby",
    {
        // Menu Colors
        BLACK | (YELLOW AS_BG),			// Header
        WHITE | (RED AS_BG),		    // Error
        WHITE | (BLUE AS_BG),			// message 1
        BLUE | (BLACK AS_BG)			// message 2
    },
        {
            // Controller Colors
            YELLOW | (BLACK AS_BG),	        // outline | faceColor 
            YELLOW | (BLACK AS_BG),		    // buttonColor
            BRIGHT_YELLOW | (BLACK AS_BG),  // highlightColor
            WHITE | (YELLOW AS_BG)  // selectColor
        },
    // Menu background
    RED | (BLACK AS_BG),
    // Controller background
    YELLOW | (BLACK AS_BG),
}

};

// takes a color package representing controller button colors and converts to screen button colors for mouse hovering
tUIColorPkg controllerButtonsToScreenButtons(tUIColorPkg& inButs) {
    return tUIColorPkg(
        inButs.col2,    // default
        inButs.col3,    // highlight
        inButs.col4,    // select
        inButs.col3     // active // used for ip input
    );
}

// converts a simple color scheme to a full color scheme *random color scheme generation can be converted to a full scheme
FullColorScheme fullSchemeFromSimpleScheme(ColorScheme& simp, ColorScheme& menu, WORD bg) {
    FullColorScheme ret = {
        L"Random",
        {
            // Menu Colors
            makeSafeColors(menu.outlineColor | menu.selectColor),	    // Header // LAN
            static_cast<WORD>(WHITE | RED AS_BG),		                // Error
            makeSafeColors(simp.faceColor AS_FG | menu.outlineColor AS_BG),	 // message 1 // WAN
            makeSafeColors(menu.faceColor AS_FG | menu.buttonColor AS_BG)	 // message 2 // PORT
        },
        {
            // Controller Colors
            static_cast<WORD>(simp.outlineColor | simp.faceColor),	    // outline | faceColor 
            static_cast<WORD>(simp.buttonColor | simp.faceColor),		// buttonColor
            static_cast<WORD>(simp.highlightColor | simp.faceColor),	    // highlightColor
            simp.selectColor                         // selectColor 
        },
        // Menu background
        static_cast<WORD>(menu.faceColor | menu.buttonColor),
        // Controller background
        static_cast<WORD>(simp.outlineColor | bg)
    };

    return ret;
}

// converts a a full color scheme to a simple scheme *needed for draw and redraw controller face functions
ColorScheme simpleSchemeFromFullScheme(FullColorScheme& full) {
    return ColorScheme{
        static_cast<WORD>(full.controllerColors.col1 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col1 BG_ONLY),
        static_cast<WORD>(full.controllerColors.col2 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col3 FG_ONLY),
        static_cast<WORD>(full.controllerColors.col4)
    };
}

