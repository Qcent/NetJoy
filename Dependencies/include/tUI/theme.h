#pragma once

// ********************************
// FullColourScheme

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



// ^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^
// tUI Helper Defines
#define COLOR_EGG_b   0x0001
#define PTRN_EGG_b    0x0002
#define EGGSHOWN_f    0x0004
#define CLUB_EGG_a    0x0008
#define BORDER_EGG_a  0x0010
#define HEART_EGG_a   0x0020
#define tUI_THEME_f   0x0040
#define tUI_THEME_af  0x0080
#define RECOL_tUI_f   0x0100
#define REDRAW_tUI_f  0x0200
#define REFLAG_tUI_f  0x0400
#define PTRN_HELD_f   0x0800
#define tUI_LOADED_f  0x1000
#define tUI_RESTART_f 0x2000

// ********************************
// tUI THEME

void printDiagonalPattern(int top, int bottom, int start_x, int start_y, int width, int step, wchar_t block = L'░', int dir = 0) {
    int ob = 0;
    int x = start_x, y = start_y;
    while (ob < 2) {
        ob = 0;
        if (dir == 0) {
            // top-left to bottom-right
            if (x >= width) ob++;
            else if (x >= start_x) {
                ob = 0;
                setCursorPosition(x, y);
                std::wcout << block;
            }
            // down and right
            y++;
            x++;
            if (y >= bottom) {
                y = top;
                x += step;
                if (x >= width) ob++;
            }
        }
        else if (dir == -1) {
            // bottom-left to top-right
            if (x >= width) ob++;
            else if (x >= start_x) {
                ob = 0;
                setCursorPosition(x, y);
                std::wcout << block;
            }
            // up and right
            y--;
            x++;
            if (y < top) {
                y = bottom - 1;
                x += step;
                if (x >= width) ob++;
            }
        }
        else {
            return;
        }
    }
}

void no_whitespace_Draw(const wchar_t* _text, int x, int y, int _width, int _length) {
    int line = 0;
    setCursorPosition(x, y);
    for (int i = 1; i - 1 < _length; i++) {
        if (_text[i - 1] == L' ') {
            // advance cursor
            setCursorPosition(x + i % _width, y + line);
        }
        else
            std::wcout << _text[i - 1];
        if (i % _width == 0 && i < _length) {
            line++;
            setCursorPosition(x, y + line);
        }
    }
}

wchar_t getWcharFromByte(byte b) {
    switch (b) {
    case 0: return L' ';
    case 1: return L'░';
    case 2: return L'▒';
    case 3: return L'▓';
    case 4: return L'█';
    default: return L'?';  // Handle invalid input
    }
}

byte getByteFromWchar(wchar_t wc) {
    switch (wc) {
    case L' ': return 0;
    case L'░': return 1;
    case L'▒': return 2;
    case L'▓': return 3;
    case L'█': return 4;
    default: return 0;
    }
}

wchar_t getCharAtPosition(int x, int y) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    wchar_t ch = L' ';
    DWORD charsRead = 0;
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    ReadConsoleOutputCharacter(hConsole, &ch, 1, coord, &charsRead);
    return ch;
}

void clearPtrn(WORD color) {
    setTextColor(color);
    for (int block = 9; block < 19; block++) {
        printDiagonalPattern(2, 19, 5, block, 68, -7, L' ');
    }
}

class tUITheme {
private:
    byte _ptrnBlocks[11] = { 0 };
    FullColorScheme _colors;
    DWORD _state = 0x0000;

public:
    tUITheme() {};

    void recordTheme(FullColorScheme& scheme, DWORD state) {
        recordBlocks();
        _colors = scheme;
        _state = state;
    }

    void recordBlocks() {
        int block;
        for (block = 9; block < 18; block++) {
            _ptrnBlocks[block - 9] = getByteFromWchar(getCharAtPosition(5, block));
        }
        _ptrnBlocks[9] = getByteFromWchar(getCharAtPosition(5, 8));
        block = 0;
        wchar_t b1 = getWcharFromByte(_ptrnBlocks[0]), b2 = getWcharFromByte(_ptrnBlocks[1]);
        while (b1 == b2) {
            block++;
            b1 = getWcharFromByte(_ptrnBlocks[block]);
            b2 = getWcharFromByte(_ptrnBlocks[block + 1]);
        }
        if (getCharAtPosition(6, block + 9) == b2)  // pattern moves up and right
            _ptrnBlocks[10] = 1;
        else
            _ptrnBlocks[10] = 0;
    }

    bool const saveToFile(const char* filename) {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            std::wcerr << L"Error opening file for saving!" << std::endl;
            return 0;
        }

        outFile.write(reinterpret_cast<const char*>(_ptrnBlocks), 11);
        outFile.write(reinterpret_cast<const char*>(&_colors), sizeof(_colors));
        outFile.write(reinterpret_cast<const char*>(&_state), sizeof(_state));

        outFile.close();
        return 1;
    }

    bool const loadFromFile(const char* filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            return false;
        }
        inFile.read(reinterpret_cast<char*>(_ptrnBlocks), 11);
        inFile.read(reinterpret_cast<char*>(&_colors), sizeof(FullColorScheme));
        _colors.name = L"SavedTheme";
        inFile.read(reinterpret_cast<char*>(&_state), sizeof(_state));
        inFile.close();

        return true;
    }

    void setColors(FullColorScheme& scheme) {
        _colors = scheme;
    }

    void const restoreColors(FullColorScheme& scheme) {
        scheme = _colors;
    }

    void const restoreState(DWORD& state) {
        state = _state;
    }

    bool const drawPtrn() {
        int block = 0, dir = _ptrnBlocks[10] * -1;
        if (dir > 0 || dir < -1) return 0; // corrupt map data leads to infinite loop
        setTextColor(_colors.menuBg);
        for (block = 9; block < 19; block++) {
            printDiagonalPattern(2, 19, 5, block + dir * 7, 68, -7, getWcharFromByte(_ptrnBlocks[block - 9]), dir);
        }
        return 1;
    }

    void drawPtrnDiag(int block) {
        int dir = _ptrnBlocks[10] * -1;
        setTextColor(_colors.menuBg);
        printDiagonalPattern(2, 19, 5, (block+9) + dir * 7, 68, -7, getWcharFromByte(_ptrnBlocks[block]), dir);
    }

    void shiftPtrnLeft() {
        byte temp = _ptrnBlocks[0];

        for (int i = 1; i < 10; ++i) {
            _ptrnBlocks[i - 1] = _ptrnBlocks[i];
        }
        _ptrnBlocks[9] = temp;
    }


};

byte g_currentColorScheme;
ColorScheme g_simpleScheme;
tUITheme g_theme;
DWORD g_status = 0;


void POPULATE_COLOR_LIST(std::vector<WORD>& colorList) {
    int colorseed = generateRandomInt(1100000000, 2147483647);
    for (int byteIndex = 0; byteIndex < sizeof(int); ++byteIndex) {
        unsigned char byteValue = (colorseed >> (byteIndex * 8)) & 0xFF;
        for (int nibbleIndex = 0; nibbleIndex < 2; ++nibbleIndex) {
            unsigned char nibble = (byteValue >> (nibbleIndex * 4)) & 0x0F;
            colorList.push_back(nibble);
        }
    }
}

tUIColorPkg GET_EGG_COLOR() {
    std::vector<WORD> colorList; \
        POPULATE_COLOR_LIST(colorList); \
        WORD col3 = CheckContrastMismatch(fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY, fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) ? \
        (findSafeFGColor(fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY, colorList, colorList.begin()) | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY) : \
        (fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY | fullColorSchemes[g_currentColorScheme].menuBg BG_ONLY); \
        return tUIColorPkg({ fullColorSchemes[g_currentColorScheme].menuBg, fullColorSchemes[g_currentColorScheme].menuBg, col3, col3 });
}

void COLOR_EGGS() {
    tUIColorPkg colors = GET_EGG_COLOR();
    g_screen.SetButtonsColorsById(colors, { 42,43,44,45,46,47 });
}

void THE_HEARTENING() {
#ifdef JOYSENDER_TUI
    g_status |= HEART_EGG_a | REDRAW_tUI_f | REFLAG_tUI_f;
    g_status &= ~tUI_THEME_af;
    replaceXEveryNth(&SendFooterAnimation[4][8], 31, L"+", L"♥", 1);
    replaceXEveryNth(&SendFooterAnimation[12][9], 31, L"+", L"♥", 1);
    replaceXEveryNth(&SendFooterAnimation[8][29], 31, L"-", L"♥", 1);
    replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * (consoleHeight - 1)], sizeof(JoyRecvMain_Backdrop), L"\\", L"♥", 6);
    for (int i = 0; i < FOOTER_ANI_FRAME_COUNT; i++) {
        replaceXEveryNth(&SendFooterAnimation[i][10], 31, L"\\", L"♥", 1);
    }
#else
    g_status |= HEART_EGG_a | REDRAW_tUI_f | REFLAG_tUI_f;
    g_status &= ~tUI_THEME_af;
    replaceXEveryNth(&FooterAnimation[1][2], 31, L"+", L"♥", 1);
    replaceXEveryNth(FooterAnimation[2], 31, L"┄", L"+", 1);
    replaceXEveryNth(FooterAnimation[8], 31, L"+", L"♥", 1);
    replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * (consoleHeight - 1)], sizeof(JoyRecvMain_Backdrop), L"\\", L"♥", 6);
    for (int i = 0; i < FOOTER_ANI_FRAME_COUNT; i++) {
        replaceXEveryNth(&FooterAnimation[i][10], 31, L"\\", L"♥", 1);
    }
#endif
}
void DE_HEARTENING() {
#ifdef JOYSENDER_TUI
    g_status |= HEART_EGG_a | REDRAW_tUI_f | REFLAG_tUI_f;
    g_status &= ~tUI_THEME_af;
    replaceXEveryNth(&SendFooterAnimation[4][8], 31, L"♥", L"+", 1);
    replaceXEveryNth(&SendFooterAnimation[12][9], 31, L"♥", L"+", 1);
    replaceXEveryNth(&SendFooterAnimation[8][29], 31, L"♥", L"-", 1);
    replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * (consoleHeight - 1)], sizeof(JoyRecvMain_Backdrop), L"♥", L"\\", 6);
    for (int i = 0; i < FOOTER_ANI_FRAME_COUNT; i++) {
        replaceXEveryNth(&SendFooterAnimation[i][10], 31, L"♥", L"\\", 1);
    }
#else
    g_status &= ~(HEART_EGG_a | tUI_THEME_af);
    g_status |= REDRAW_tUI_f | REFLAG_tUI_f;
    replaceXEveryNth(&FooterAnimation[1][2], 31, L"♥", L"+", 1);
    replaceXEveryNth(FooterAnimation[2], 31, L"+", L"┄", 1);
    replaceXEveryNth(FooterAnimation[8], 31, L"♥", L"+", 1);
    replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * (consoleHeight - 1)], sizeof(JoyRecvMain_Backdrop), L"♥", L"\\", 6);
    for (int i = 0; i < FOOTER_ANI_FRAME_COUNT; i++) {
        replaceXEveryNth(&FooterAnimation[i][10], 31, L"♥", L"\\", 1);
    }
#endif
}


void THE_CLUBENING() {
    g_status |= CLUB_EGG_a;
#ifdef JOYSENDER_TUI
    replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * 4], sizeof(JoySendMain_Backdrop), L"++", L"+╠", 25, 3);
    replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth - 1) * 5], sizeof(JoySendMain_Backdrop), L"++", L"╣+", 19);
    int side, row, col, top;
    top = generateRandomInt(-1, 6);
    side = generateRandomInt(0, 1);
    row = generateRandomInt(0, 1);
    col = generateRandomInt(3, 16);
    if (top) {
        col = generateRandomInt(6, 62);
        replaceXEveryNth(&JoySendMain_Backdrop[col], sizeof(JoySendMain_Backdrop), (side ? L"►" : L"◄"), L"♣", 1);
    }
    replaceXEveryNth(&JoySendMain_Backdrop[((consoleWidth + 2) * col) + (side * 66)], sizeof(JoySendMain_Backdrop), (row ? L"+" : (side ? L"╣" : L"╠")), L"♣", 1);
#else
    replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 4], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╠", 25, 3);
    replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth - 1) * 5], sizeof(JoyRecvMain_Backdrop), L"||", L"╣┋", 19);    
    int side, row, col;
    side = generateRandomInt(0, 1);
    row = generateRandomInt(0, 1);
    col = generateRandomInt(3, 16);
    replaceXEveryNth(&JoyRecvMain_Backdrop[((consoleWidth + 2) * col) + (side * 66)], sizeof(JoyRecvMain_Backdrop), (row ? L"┋" : (side ? L"╣" : L"╠")), L"♣", 1);
#endif
}

void PRINT_EGG_X() {
    if (g_status & BORDER_EGG_a) {
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        setCursorPosition(consoleWidth - 3, consoleHeight - 3);
        std::wcout << L"╳";
    }
}

void SPEC_EGGS() {
    if (g_status & EGGSHOWN_f) return;
    g_status |= EGGSHOWN_f;
    mouseButton* heartEgg = new mouseButton(3, consoleHeight - 3, 3, L"♥");
    g_screen.AddButton(heartEgg);
    heartEgg->SetId(42);
    heartEgg->setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            if (!(g_status & HEART_EGG_a)) {
                THE_HEARTENING();
            }
            else if (g_status & HEART_EGG_a) {
                DE_HEARTENING();
            }
        }
        });
    mouseButton* patrnEgg = new mouseButton(consoleWidth - 3, consoleHeight - 3, 3, L"♠");
    patrnEgg->SetId(45);
    g_screen.AddButton(patrnEgg);
    patrnEgg->setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            if (g_status & PTRN_EGG_b) {
                g_status &= ~(PTRN_EGG_b | tUI_THEME_af);
                g_status |= REDRAW_tUI_f | REFLAG_tUI_f;
            }
            else {
                g_status |= PTRN_EGG_b | REDRAW_tUI_f | REFLAG_tUI_f;
                g_status &= ~tUI_THEME_af;
            }
        }
        });
}

void CLEAN_EGGS() {
    g_screen.DeleteButton(42);
    g_screen.DeleteButton(45);
    g_screen.DeleteButton(46);
    g_screen.DeleteButton(47);
    g_status &= ~EGGSHOWN_f;
}

void GET_NEW_COLOR_SCHEME() {
    g_currentColorScheme = generateRandomInt(0, NUM_COLOR_SCHEMES);
    if (g_currentColorScheme == RANDOMSCHEME)
    {
        ColorScheme menuScheme = createRandomScheme();
        WORD newRandomBG = generateRandomInt(0, 15) AS_BG;
        ColorScheme randomScheme = createRandomScheme();
        fullColorSchemes[RANDOMSCHEME] = fullSchemeFromSimpleScheme(randomScheme, menuScheme, newRandomBG);
    }
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]); /* Set for compatibility with DrawControllerFace */
}




void MAKE_PATTERNS() {
    if ((g_status & PTRN_EGG_b) && (g_status & tUI_THEME_af)) {
        g_theme.drawPtrn();
        return;
    }
    srand(static_cast<unsigned int>(time(0)));
    std::vector<wchar_t> blocks = { L'▓', L'▒', L'█', L'▓', L'░', L'▒' };
    std::vector<int> numbers = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(numbers.begin(), numbers.end(), rng);
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    std::shuffle(blocks.begin(), blocks.end(), rng);
    int dir = (numbers[2] > 13) ? 0 : -1;
    printDiagonalPattern(2, 19, 5, numbers[0] + dir * 7, 68, -7, L'▒', dir);
    printDiagonalPattern(2, 19, 5, numbers[1] + dir * 7, 68, -7, L'▒', dir);
    printDiagonalPattern(2, 19, 5, numbers[2] + dir * 7, 68, -7, L' ', dir);
    printDiagonalPattern(2, 19, 5, numbers[3] + dir * 7, 68, -7, L'▓', dir);
    printDiagonalPattern(2, 19, 5, numbers[4] + dir * 7, 68, -7, L'▒', dir);
    printDiagonalPattern(2, 19, 5, numbers[5] + dir * 7, 68, -7, L'▒', dir);
    printDiagonalPattern(2, 19, 5, numbers[6] + dir * 7, 68, -7, L'▒', dir);
    printDiagonalPattern(2, 19, 5, numbers[7] + dir * 7, 68, -7, blocks[3], dir);
    printDiagonalPattern(2, 19, 5, numbers[8] + dir * 7, 68, -7, L'░', dir);
    printDiagonalPattern(2, 19, 5, numbers[9] + dir * 7, 68, -7, L'▒', dir);
}

// dont really like this function
void RANDOM_DIAGONAL(int col = -1) {
    std::vector<wchar_t> blocks = { L'▓', L'▒', L'█', L'▓', L'░', L'▒' };
    std::vector<int> startpoints = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(startpoints.begin(), startpoints.end(), rng);

    int block = generateRandomInt(0, 5);
    int dir = generateRandomInt(-1, 0);
    int start = generateRandomInt(0, 9);

    if(col > -1)
        setTextColor(col);
    else
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);

    printDiagonalPattern(2, 19, 5, start + dir * 7, 68, -7,blocks[block], dir);
}

void MORPH_BORDER() {
#ifdef JOYSENDER_TUI
    if (!(g_status & BORDER_EGG_a)) {
        g_status |= BORDER_EGG_a;
        replaceXEveryNth(JoySendMain_Backdrop, sizeof(JoySendMain_Backdrop), L"____", L"+◄+►", 15);
        replaceXEveryNth(&JoySendMain_Backdrop[consoleWidth - 6], sizeof(JoySendMain_Backdrop), L"_", L"+", 1);
        replaceXEveryNth(JoySendMain_Backdrop, sizeof(JoySendMain_Backdrop), L"___", L"═══", 21);
        

        replaceXEveryNth(JoySendMain_Backdrop, sizeof(JoySendMain_Backdrop), L"=", L"┉", 1);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * 2], sizeof(JoySendMain_Backdrop), L"||", L"╣┋", 25, 3);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth - 1) * 3], sizeof(JoySendMain_Backdrop), L"||", L"┋╠", 8, 2);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * 17], sizeof(JoySendMain_Backdrop), L"||", L"♥┋", 1);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth) * 18], sizeof(JoySendMain_Backdrop), L"||", L"┋╳", 1);
        if (!(g_status |= CLUB_EGG_a) && generateRandomInt(7, 18) < 11) {
            g_status |= CLUB_EGG_a;
            THE_CLUBENING();
        }
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * 19], sizeof(JoySendMain_Backdrop), L"-{", L"◄∑", 1);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 5) * 19], sizeof(JoySendMain_Backdrop), L"}-", L"∫►", 1);
        replaceXEveryNth(rsideFooter, 20, L"}-", L"∫►", 1);
}
#else
    if (!(g_status & BORDER_EGG_a)) {
        g_status |= BORDER_EGG_a;
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"= ", L"══", 30);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"=", L"═", 1);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"= ", L"┉┉", 31);
        replaceXEveryNth(JoyRecvMain_Backdrop, sizeof(JoyRecvMain_Backdrop), L"=", L"┉", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 2], sizeof(JoyRecvMain_Backdrop), L"||", L"◊┋", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth) * 3], sizeof(JoyRecvMain_Backdrop), L"||", L"┋◊", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 3], sizeof(JoyRecvMain_Backdrop), L"||", L"╣┋", 25, 5);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth - 1) * 4], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╠", 19, 4);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 17], sizeof(JoyRecvMain_Backdrop), L"||", L"♥┋", 1);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth) * 18], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╳", 1);
        if (!(g_status |= CLUB_EGG_a) && generateRandomInt(7, 18) < 11) {
            g_status |= CLUB_EGG_a;
            THE_CLUBENING();
        }
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 19], sizeof(JoyRecvMain_Backdrop), L"-{", L"◄∑", 1);
        replaceXEveryNth(&FooterAnimation[1][2], 31, L".", L"+", 1);
        replaceXEveryNth(FooterAnimation[2], 31, L"*", L"┄", 1);
        replaceXEveryNth(FooterAnimation[8], 31, L"*", L"+", 1);
        replaceXEveryNth(rsideFooter, 20, L"}-", L"∫►", 1);
    }
#endif
}

void RESTORE_THEME() {
    g_currentColorScheme = RANDOMSCHEME;
    g_theme.restoreColors(fullColorSchemes[RANDOMSCHEME]);
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    g_screen.DrawBackdrop();
    DWORD savedStatus = 0;
    g_theme.restoreState(savedStatus);
    if (savedStatus & BORDER_EGG_a) {
        g_status &= ~BORDER_EGG_a; // need to be off for morph border
        MORPH_BORDER();
    }
    if (savedStatus & HEART_EGG_a) {
        g_status |= HEART_EGG_a;
        THE_HEARTENING();
    }
    if (savedStatus & PTRN_EGG_b) {
        g_status |= PTRN_EGG_b;
        g_theme.drawPtrn();
    }
    else {
        g_status &= ~PTRN_EGG_b;
        clearPtrn(fullColorSchemes[g_currentColorScheme].menuBg);
    }
    g_status |= tUI_LOADED_f | REDRAW_tUI_f;
}

void RESTORE_THEME2() {
    g_currentColorScheme = RANDOMSCHEME;
    g_theme.restoreColors(fullColorSchemes[RANDOMSCHEME]);
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
    g_status |= tUI_LOADED_f | REDRAW_tUI_f | RECOL_tUI_f;
}
