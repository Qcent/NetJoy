#include <io.h>
#include <fcntl.h>
#include <map>
#include <set>
#include <vector>

#include "textUI.hpp"
#include "backdrops.h"
#include "uiButtons.h"
#include "controllerFaces.h"
#include "animations.h"

//----------------------------------------------------------------------------
// lifted from : https://cplusplus.com/forum/windows/10731/
struct console
{
    console(unsigned width, unsigned height)
    {
        SMALL_RECT r;
        COORD      c;
        hConOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(hConOut, &csbi);

        r.Left = r.Top = 0;
        r.Right = width;
        r.Bottom = height + 1;
        SetConsoleWindowInfo(hConOut, TRUE, &r);

        c.X = width;
        c.Y = height;
        SetConsoleScreenBufferSize(hConOut, c);
    }

    ~console()
    {
        SetConsoleTextAttribute(hConOut, csbi.wAttributes);
        SetConsoleScreenBufferSize(hConOut, csbi.dwSize);
        SetConsoleWindowInfo(hConOut, TRUE, &csbi.srWindow);
    }

    HANDLE                     hConOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
};

// ********************************
// FullColourScheme

// Package up all the colors needed for maximum color expression
struct FullColorScheme {
    const wchar_t* name;
    tUIColorPkg menuColors;
    tUIColorPkg controllerColors;
    WORD menuBg;
    WORD controllerBg;
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
FullColorScheme fullSchemeFromSimpleScheme(ColorScheme& simp, WORD bg) {
    FullColorScheme ret = {
        L"Random",
        {
            // Menu Colors
            BLACK | (GREY AS_BG),			// Header
            BLACK | (RED AS_BG),		        // Error
            BLACK | (BRIGHT_GREY AS_BG),		// message 1
            BLACK | (WHITE AS_BG)			// message 2
        },
        {
            // Controller Colors
            static_cast<WORD>(simp.outlineColor | simp.faceColor),	    // outline | faceColor 
            static_cast<WORD>(simp.buttonColor | simp.faceColor),		// buttonColor
            static_cast<WORD>(simp.highlightColor | simp.faceColor),	    // highlightColor
            simp.selectColor                         // selectColor 
        },
        // Menu background
        BLACK | (WHITE AS_BG),
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
        static_cast<WORD>(full.controllerColors.col4),
        L" "
    };
}
