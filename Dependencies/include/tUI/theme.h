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
#include <functional>
#include <mutex>

extern void tUI_THEME_SELECTOR_SCREEN();
extern void printX_Ntimes(const wchar_t, const int);
extern void drawPatternedArea(const RECT, byte*);
extern void printSubsetOfBuffer(const COORD, const RECT&, const RECT&, const wchar_t*, bool);
extern void restartModeCallback(mouseButton&);
extern void restartStatusCallback(mouseButton&);
extern void mappingButtonCallback(mouseButton&);
void tUI_CLEAR_SCREEN(WORD col);
void tUI_DRAW_TITLE_AND_VERSION();
mouseButton* tUI_GET_HOLIDAY_IMAGE(uint8_t size);

constexpr int HOLIDAY_BASE_ID = 69400;
textUI* holidayImages = nullptr;

// ^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^*%*^*&^
// tUI Helper Defines
#define COLOR_EGG_b    0x0001
#define PTRN_EGG_b     0x0002
#define DIAMONDS_a     0x0004
#define CLUB_EGG_a     0x0008
#define BORDER_EGG_a   0x0010
#define HEART_EGG_a    0x0020
#define tUI_THEME_f    0x0040
#define tUI_THEME_af   0x0080
#define RECOL_tUI_f    0x0100
#define REDRAW_tUI_f   0x0200
#define REFLAG_tUI_f   0x0400
#define MISC_FLAG_f    0x0800
#define tUI_LOADED_f   0x1000
#define tUI_RESTART_f  0x2000
#define CTRLR_SCREEN_f 0x4000
#define EDIT_THEME_f   0x8000

#define EGG_IDs { 41,42,43,44,45,46,47 }
#define HEAP_BTN_IDs { 41,42,45,46,47 }

#ifdef JOYSENDER_TUI
const char* THEME_FILE = "send.theme";
#else
const char* THEME_FILE = "recv.theme";
#endif

// ********************************
// tUI THEME

class tUITheme {
private:
    byte _ptrnBlocks[11] = { 0 };
    DWORD _state = 0x0000;

public:
    FullColorScheme _colors;

    tUITheme() {};

    // Custom comparator (equality operator)
    bool operator==(const tUITheme& other) const {
        if (!std::equal(std::begin(_ptrnBlocks), std::end(_ptrnBlocks), std::begin(other._ptrnBlocks))) {
            return false;
        }
        if (!(this->_colors == other._colors)) {
            return false;
        }
        return this->_state == other._state;
    }

    // Inequality operator 
    bool operator!=(const tUITheme& other) const {
        return !(*this == other);
    }

    void recordTheme(FullColorScheme& scheme, DWORD state) {
        recordBlocks();
        _colors = scheme;
        _state = state;
    }
    
    void updateTheme(FullColorScheme& scheme, DWORD state) {
        _colors = scheme;
        _state = state;
    }

    // modified to maintain original functionality while also dynamic. requires 2 columns of pattern 9 charaters high
    void recordBlocks(int x = 5, int y = 8) {
        for (int i = 0; i < 9; i++) {
            _ptrnBlocks[i] = getByteFromWchar(getCharAtPosition(x, y + 1+i));
        }
        _ptrnBlocks[9] = getByteFromWchar(getCharAtPosition(x, y));
        int block = 0;
        wchar_t b1 = getWcharFromByte(_ptrnBlocks[0]), b2 = getWcharFromByte(_ptrnBlocks[1]);
        while (b1 == b2) {
            block++;
            b1 = getWcharFromByte(_ptrnBlocks[block]);
            b2 = getWcharFromByte(_ptrnBlocks[block + 1]);
        }
        if (getCharAtPosition(x + 1, block + y + 1) == b2)  // pattern moves up and right
            _ptrnBlocks[10] = 1;
        else
            _ptrnBlocks[10] = 0;
    }
    bool const saveToFile(const char* filename) {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            std::wcerr << L"Error opening file for saving!" << std::endl;
            return false;
        }
        outFile.write(reinterpret_cast<const char*>(_ptrnBlocks), 11);
        uint64_t namePtr = reinterpret_cast<uint64_t>(nullptr);
        outFile.write(reinterpret_cast<const char*>(&namePtr), sizeof(namePtr));
        outFile.write(reinterpret_cast<const char*>(&_colors.menuColors), sizeof(_colors.menuColors));
        outFile.write(reinterpret_cast<const char*>(&_colors.controllerColors), sizeof(_colors.controllerColors));
        outFile.write(reinterpret_cast<const char*>(&_colors.menuBg), sizeof(_colors.menuBg));
        outFile.write(reinterpret_cast<const char*>(&_colors.controllerBg), sizeof(_colors.controllerBg));
        outFile.write(reinterpret_cast<const char*>(&_state), sizeof(_state));
        outFile.close();
        return true;
    }

    bool const loadFromFile(const char* filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            return false;
        }
        inFile.read(reinterpret_cast<char*>(_ptrnBlocks), 11);
        uint64_t namePtr;
        inFile.read(reinterpret_cast<char*>(&namePtr), sizeof(namePtr));
        _colors.name = L"SavedTheme";
        inFile.read(reinterpret_cast<char*>(&_colors.menuColors), sizeof(_colors.menuColors));
        inFile.read(reinterpret_cast<char*>(&_colors.controllerColors), sizeof(_colors.controllerColors));
        inFile.read(reinterpret_cast<char*>(&_colors.menuBg), sizeof(_colors.menuBg));
        inFile.read(reinterpret_cast<char*>(&_colors.controllerBg), sizeof(_colors.controllerBg));
        inFile.read(reinterpret_cast<char*>(&_state), sizeof(_state));
        inFile.close();
        return true;
    }

    void setColors(const FullColorScheme& scheme) {
        _colors = scheme;
    }

    void const restoreColors(FullColorScheme& scheme) {
        scheme = _colors;
    }

    DWORD& getState() {
        return _state;
    }

    byte* getBlocks() {
        return &_ptrnBlocks[0];
    }

    void setState(const DWORD state) {
        _state = state;
    }

    void const restoreState(DWORD& state) {
        state = _state;
    }

    bool const drawPtrn(WORD col = 0xFF00) {
        if (col == 0xff00) col = _colors.menuBg;
        int block = 0, dir = _ptrnBlocks[10] * -1;
        setTextColor(col);
        if (_ptrnBlocks[0] == 255) {
            wchar_t ch;
            switch (_ptrnBlocks[1]) {
            case 1:
                ch = L'░';
                break;
            case 2:
                ch = L'▒';
                break;
            case 3:
                ch = L'▓';
                break;
            default:
                ch = L'░';
            }
            for (int i = 0; i < 17; ++i) {
                setCursorPosition(5, 2 + i);
                printX_Ntimes(ch, 63);
            }
        }
        else {            
            for (block = 9; block < 19; block++) {
                printDiagonalPattern(2, 19, 5, block + dir * 7, 68, -7, getWcharFromByte(_ptrnBlocks[block - 9]), dir);
            }
        }
        return 1;
    }

    void drawPtrnDiag(int block, RECT area = {5,2,68,19}) {
        int dir = _ptrnBlocks[10] * -1;
        int step = 1 - ((area.bottom - area.top) - 9);
        setTextColor(_colors.menuBg);
        printDiagonalPattern(area.top, area.bottom, area.left, (block+9) + dir * 7, area.right-area.left, step, getWcharFromByte(_ptrnBlocks[block]), dir);
    }

    void shiftPtrnLeft() {
        byte temp = _ptrnBlocks[0];

        for (int i = 1; i < 10; ++i) {
            _ptrnBlocks[i - 1] = _ptrnBlocks[i];
        }
        _ptrnBlocks[9] = temp;
    }
};


std::mutex theme_mtx;
byte g_currentColorScheme;
ColorScheme g_simpleScheme;
tUITheme g_theme, loadedTheme;
DWORD g_status = 0;
int g_clubPos = 0;

#define tUI_DRAW_HOLIDAY_IMAGES() {\
    mouseButton* tmp = tUI_GET_HOLIDAY_IMAGE(3); \
    tmp->Draw(); \
    tmp = tUI_GET_HOLIDAY_IMAGE(2); \
    tmp->Draw(); \
    cheer = true; \
    tUI_DRAW_TITLE_AND_VERSION(); \
}


WORD tUI_FLAGS() {
    return g_status & (DIAMONDS_a | BORDER_EGG_a | HEART_EGG_a | PTRN_EGG_b);
}

WORD DIAMOND_COLOR() {
    if (g_status & CTRLR_SCREEN_f) {
        if (g_status & DIAMONDS_a) {
            // except when P and not B
            if ((tUI_FLAGS() & (PTRN_EGG_b | BORDER_EGG_a | HEART_EGG_a)) == PTRN_EGG_b) {
                return fullColorSchemes[g_currentColorScheme].controllerBg;
            }
            return fullColorSchemes[g_currentColorScheme].menuBg;
        }
        else {
            return fullColorSchemes[g_currentColorScheme].controllerBg;
        }
    }
    else {
        return fullColorSchemes[g_currentColorScheme].menuBg;
    }
}

const std::vector<COORD> SUIT_POSITIONS_SCATTERED() {
    const std::vector<COORD> pos = {
        { static_cast<SHORT>(g_clubPos % 74), static_cast<SHORT>(g_clubPos / 74) }, // 41 - clubs
        { 3, consoleHeight - 3 }, // 42 - hearts
        { consoleWidth - 3, consoleHeight - 3 },  // 45 - spades
        { 3, 2 },  // 46 - apply
        { consoleWidth - 3, 2 }  // 47 - save
    };
    return pos;
}
const std::vector<COORD> SUIT_POSITIONS_COLLECTED() {
    const short y = (output3.GetPosition().X == 46 ? 0 : 1);
    const std::vector<COORD> pos = {
        { consoleWidth - 25, y }, // 41 - clubs
        { consoleWidth - 23, y }, // 42 - hearts
        { consoleWidth - 21, y },  // 45 - spades
        { consoleWidth - 27, y },  // 46 - apply
        { consoleWidth - 19, y }  // 47 - save
    };
    return pos;
}
const std::vector<COORD> SUIT_POSITIONS_MAP_SCREEN() {
    constexpr short y = 1;
    constexpr short x = consoleWidth - 12;
    const std::vector<COORD> pos = {
        { x + 2, y },   // 41 - clubs
        { x + 4, y },   // 42 - hearts
        { x + 6, y },   // 45 - spades
        { x, y },       // 46 - apply
        { x + 8, y }    // 47 - save
    };
    return pos;
}
void tUI_SET_SUIT_POSITIONS(std::vector<COORD> pos) {
    constexpr int num = 5;
    constexpr int ids[num] = HEAP_BTN_IDs;
    for (int i = 0; i < num; i++) {
        mouseButton* btn = g_screen.GetButtonById(ids[i]);
        if (btn != nullptr) {
            btn->SetPosition(pos[i]);
        }
    }
}
void tUI_AUTO_SET_SUIT_POSITIONS() {
    tUI_SET_SUIT_POSITIONS((((g_status & BORDER_EGG_a) || !(g_status & CTRLR_SCREEN_f)) ? SUIT_POSITIONS_SCATTERED() : SUIT_POSITIONS_COLLECTED()));
}

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
    WORD bg_color = DIAMOND_COLOR();
    std::vector<WORD> colorList; \
    POPULATE_COLOR_LIST(colorList); \
    WORD col3 = CheckContrastMismatch(fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY, bg_color BG_ONLY) ? \
    (findSafeFGColor(bg_color BG_ONLY, colorList, colorList.begin()) | bg_color BG_ONLY) : \
    (fullColorSchemes[g_currentColorScheme].controllerColors.col3 FG_ONLY | bg_color BG_ONLY); \
    return tUIColorPkg({ bg_color, bg_color, col3, col3 });
}
void COLOR_EGGS() {
    tUIColorPkg colors = GET_EGG_COLOR();
    g_screen.SetButtonsColorsById(colors, EGG_IDs);

    if (holidayImages != nullptr) {
        holidayImages->SetButtonsColors(colors);
    }
}

void RESTORE_BORDER() {
    wchar_t* backdrop = nullptr;
    g_status &= ~BORDER_EGG_a;

#ifdef JOYSENDER_TUI
    backdrop = JoySendMain_Backdrop;
    for (int i = 0; i < (74 * 2) - 1; i++) {
        backdrop[i] = sendTop[i];
    }
    for (int i = 2; i < consoleHeight - 2; i+=2) {
        backdrop[3 + (i * 74)] = L'+';
        backdrop[4 + (i * 74)] = L'+';
        backdrop[(consoleWidth - 4) + (i * 74)] = L'+';
        backdrop[(consoleWidth - 3) + (i * 74)] = L'+';

        backdrop[3 + ((i+1) * 74)] = L'|';
        backdrop[4 + ((i+1) * 74)] = L'|';
        backdrop[(consoleWidth - 4) + ((i+1) * 74)] = L'|';
        backdrop[(consoleWidth - 3) + ((i+1) * 74)] = L'|';
    }
#else
    backdrop = JoyRecvMain_Backdrop;
    for (int i = 0; i < (74 * 2) - 1; i++) {
        backdrop[i] = recvTop[i];
    }
    for (int i = 2; i < consoleHeight - 2; i++) {
        backdrop[3 + (i * 74)] = L'|';
        backdrop[4 + (i * 74)] = L'|';
        backdrop[(consoleWidth - 4) + (i * 74)] = L'|';
        backdrop[(consoleWidth - 3) + (i * 74)] = L'|';
    }
#endif    

    replaceXEveryNth(&backdrop[(consoleWidth + 2) * 19], sizeof(backdrop), L"◄∑", L"-{", 1);
    replaceXEveryNth(&backdrop[((consoleWidth + 5) * 19)+14], sizeof(backdrop), L"∫►", L"}-", 1);
    replaceXEveryNth(rsideFooter, 20, L"∫►", L"}-", 1);

}

void THE_HEARTENING() {
#ifdef JOYSENDER_TUI
    g_status |= HEART_EGG_a | REDRAW_tUI_f | REFLAG_tUI_f;
    g_status &= ~tUI_THEME_af;
    replaceXEveryNth(&SendFooterAnimation[4][8], 31, L"+", L"♥", 1);
    replaceXEveryNth(&SendFooterAnimation[12][9], 31, L"+", L"♥", 1);
    replaceXEveryNth(&SendFooterAnimation[8][29], 31, L"-", L"♥", 1);
    replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * (consoleHeight - 1)], sizeof(JoySendMain_Backdrop), L"\\", L"♥", 6);
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
    g_status &= ~(HEART_EGG_a | tUI_THEME_af);
    g_status |= REDRAW_tUI_f | REFLAG_tUI_f;
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

int THE_CLUBENING() {
    g_status |= CLUB_EGG_a;
    int clubPos = 0;
#ifdef JOYSENDER_TUI
    int side, row, col, top, startPoint;
    top = generateRandomInt(-1, 6);
    side = generateRandomInt(0, 1);
    row = generateRandomInt(0, 1);
    col = generateRandomInt(3, 16);
    startPoint = ((consoleWidth + 2) * col) + (side * 66);

    if (g_status & BORDER_EGG_a) {
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth + 2) * 4], sizeof(JoySendMain_Backdrop), L"++", L"+╠", 25, 3);
        replaceXEveryNth(&JoySendMain_Backdrop[(consoleWidth - 1) * 5], sizeof(JoySendMain_Backdrop), L"++", L"╣+", 19);
        if (top) {
            col = generateRandomInt(6, 62);
            clubPos = replaceXEveryNth(&JoySendMain_Backdrop[col], sizeof(JoySendMain_Backdrop), (side ? L"►" : L"◄"), (side ? L"►" : L"◄"), 1);
            clubPos += col;
        }
        else {
            clubPos = replaceXEveryNth(&JoySendMain_Backdrop[startPoint], sizeof(JoySendMain_Backdrop), (row ? L"+" : (side ? L"╣" : L"╠")), (side ? L"╣" : L"╠"), 1);
            clubPos += startPoint;
        }
    }
    else {
        clubPos = replaceXEveryNth(&JoySendMain_Backdrop[startPoint], sizeof(JoySendMain_Backdrop), (row ? L"|" : (side ? L"|" : L"|")), L"|", 1);
        clubPos += startPoint;
    }
#else   
    int side, row, col, startPoint;
    side = generateRandomInt(0, 1);
    row = generateRandomInt(0, 1);
    col = generateRandomInt(3, 16);
    startPoint = ((consoleWidth + 2) * col) + (side * 66);
    if (g_status & BORDER_EGG_a) {
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 4], sizeof(JoyRecvMain_Backdrop), L"||", L"┋╠", 25, 3);
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth - 1) * 5], sizeof(JoyRecvMain_Backdrop), L"||", L"╣┋", 19);
        clubPos = replaceXEveryNth(&JoyRecvMain_Backdrop[startPoint], sizeof(JoyRecvMain_Backdrop), (row ? L"┋" : (side ? L"╣" : L"╠")), (row ? L"┋" : (side ? L"╣" : L"╠")), 1);
    }
    else {
        clubPos = replaceXEveryNth(&JoyRecvMain_Backdrop[startPoint], sizeof(JoyRecvMain_Backdrop), (row ? L"|" : (side ? L"|" : L"|")), L"|", 1); // just need position
    }
    clubPos += startPoint;
#endif
    return clubPos;
}

void PRINT_EGG_X() {
    if (g_status & BORDER_EGG_a) {
        setTextColor(DIAMOND_COLOR());
        setCursorPosition(consoleWidth - 3, consoleHeight - 3);
        std::wcout << L"╳";
    }
}

// loads hearts(42) and spades(45) on the heap and into g_screen
void HEARTS_N_SPADES_BUTTONS() {
    if (g_screen.GetButtonById(42) != nullptr) return;
    mouseButton* heartEgg = new mouseButton(3, consoleHeight - 3, 1, L"♥");
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
    mouseButton* patrnEgg = new mouseButton(consoleWidth - 3, consoleHeight - 3, 1, L"♠");
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

// deletes { 41,42,45,46,47 }
void CLEAN_EGGS() {
    g_screen.DeleteButton(41);
    g_screen.DeleteButton(42);
    g_screen.DeleteButton(45);
    g_screen.DeleteButton(46);
    g_screen.DeleteButton(47);

    if (holidayImages != nullptr) {
        holidayImages->DeleteButtons({ 69400, 69401, 69402, 69403, 69404, 69405, 69406 });
        delete holidayImages;
        holidayImages = nullptr;
    }

}

// loads clubs(41) on heap and into g_screen
void CLEAN_CLUBBIN() {
    if (g_screen.GetButtonById(41) == nullptr) {
        g_clubPos = THE_CLUBENING();
        int x = 0, y = 0;
        x = g_clubPos % 74;
        y = g_clubPos / 74;
        mouseButton* clubEgg = new mouseButton(x, y, 1, L"♣");
        clubEgg->SetId(41);
        g_screen.AddButton(clubEgg);
        clubEgg->setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);

#ifdef JOYSENDER_TUI
                if (g_status & CTRLR_SCREEN_f) {
                    // Spin up a new thread so connection can continue 
                    std::thread editThread(tUI_THEME_SELECTOR_SCREEN);
                    g_status |= EDIT_THEME_f;
                    editThread.detach();
                }
                else {
                    tUI_THEME_SELECTOR_SCREEN();
                }
#else
                std::thread editThread(tUI_THEME_SELECTOR_SCREEN);
                g_status |= EDIT_THEME_f;
                editThread.detach();
#endif
            }
            });
    }
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

// cretes a patters within the bordered area
void MAKE_PATTERNS() {
    if ((g_status & PTRN_EGG_b) && (g_status & tUI_THEME_af)) {
        g_theme.drawPtrn(DIAMOND_COLOR());
        return;
    }
    srand(static_cast<unsigned int>(time(0)));
    std::vector<wchar_t> blocks = { L'▓', L'▒', L'█', L'▓', L'░', L'▒' };
    std::vector<int> numbers = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(numbers.begin(), numbers.end(), rng);
    setTextColor(DIAMOND_COLOR());
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

    g_theme.recordBlocks();
}

// clears inside border area with color
void tUI_CLEAR_PTRN_AREA(WORD color) {
    setTextColor(color);
    for (int block = 9; block < 19; block++) {
        printDiagonalPattern(2, 19, 5, block, 68, -7, L' ');
    }
}

void tUI_CLEAR_SCREEN(WORD col) {
    setCursorPosition(0, 0);
    setTextColor(col);
    for (int i = 0; i < consoleHeight + 1; i++) {
        printX_Ntimes(L' ', 73);
    }
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
            CLEAN_CLUBBIN();
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
            CLEAN_CLUBBIN();
        }
        replaceXEveryNth(&JoyRecvMain_Backdrop[(consoleWidth + 2) * 19], sizeof(JoyRecvMain_Backdrop), L"-{", L"◄∑", 1);
        replaceXEveryNth(&FooterAnimation[1][2], 31, L".", L"+", 1);
        replaceXEveryNth(FooterAnimation[2], 31, L"*", L"┄", 1);
        replaceXEveryNth(FooterAnimation[8], 31, L"*", L"+", 1);
        replaceXEveryNth(rsideFooter, 20, L"}-", L"∫►", 1);
    }
#endif
}

void RESTORE_UI_MORPHS(tUITheme& sourceTheme) {
    // apply / remove border mod
    if (g_status & BORDER_EGG_a) {
        if (!(sourceTheme.getState() & BORDER_EGG_a)) {
            RESTORE_BORDER();
            g_status &= ~BORDER_EGG_a;
        }}
    else {
        if (sourceTheme.getState() & BORDER_EGG_a) {
            g_status &= ~BORDER_EGG_a; // need to be off for morph border
            MORPH_BORDER();
        }}

    // the heart thing
    if (g_status & HEART_EGG_a) {
        if (!(sourceTheme.getState() & HEART_EGG_a)) {
            DE_HEARTENING();
            g_status &= ~HEART_EGG_a;
        }}
    else {
        if (sourceTheme.getState() & HEART_EGG_a) {
            THE_HEARTENING();
            g_status |= BORDER_EGG_a;
        }}
    g_status |= REDRAW_tUI_f;
    g_status &= ~tUI_THEME_af;
}

void tUI_APPLY_LOADED_THEME() {
    tUITheme& theme = loadedTheme;
    g_currentColorScheme = RANDOMSCHEME;
    theme.restoreColors(fullColorSchemes[RANDOMSCHEME]);
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);
    RESTORE_UI_MORPHS(theme);
    theme.restoreState(g_status);
    if (g_status & CLUB_EGG_a) {
        CLEAN_CLUBBIN();
    }
    //if (g_status & PTRN_EGG_b) {
        //theme.drawPtrn(); // gets drawn with REDRAW flag below in loop
    //}
    if (g_status & BORDER_EGG_a) {
        HEARTS_N_SPADES_BUTTONS();
    }
    //sets edge to border color: because next draw could be a no whitespace draw, making ugly
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);

    g_theme = theme;
}

void tUI_LOAD_THEME() {
    tUITheme& theme = g_theme;
    if (!(g_status & tUI_LOADED_f)) {
        if (theme.loadFromFile(THEME_FILE)) {
            loadedTheme = theme;
            tUI_APPLY_LOADED_THEME();
            g_status |= tUI_LOADED_f | REFLAG_tUI_f | REDRAW_tUI_f | RECOL_tUI_f | tUI_THEME_af | tUI_THEME_f;
        }
    }  
}

// loads apply(46) and save(47) theme buttons on heap and into g_screen
void tUI_THEME_BUTTONS() {
    if (g_screen.GetButtonById(46)) return;
    mouseButton* applyTheme = new mouseButton(3, 2, 1, (g_status & tUI_THEME_af) ? L"♦" : L"◊");
    applyTheme->SetId(46);
    g_screen.AddButton(applyTheme);
    applyTheme->setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            if ((g_status & tUI_THEME_af)) {

                g_status &= ~tUI_THEME_af;
                btn.SetText(L"◊");
            }
            else if (g_status & tUI_LOADED_f) {
                tUI_APPLY_LOADED_THEME();
                g_status |= RECOL_tUI_f | tUI_THEME_af | tUI_THEME_f;
                btn.SetText(L"♦");
            }
        }
        });
    mouseButton* saveTheme = new mouseButton(consoleWidth - 3, 2, 1, (g_status & tUI_THEME_f) ? L"♦" : L"◊");
    saveTheme->SetId(47);
    g_screen.AddButton(saveTheme);
    saveTheme->setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {

            btn.SetStatus(MOUSE_OUT);
            if (!(g_status & tUI_THEME_f)) {
                g_status |= tUI_THEME_f | tUI_THEME_af | tUI_LOADED_f | REFLAG_tUI_f;
                tUITheme& theme = g_theme;
                theme.updateTheme(
                    fullColorSchemes[g_currentColorScheme],
                    (g_status & (tUI_THEME_f | tUI_THEME_af | REFLAG_tUI_f |
                        HEART_EGG_a | BORDER_EGG_a | PTRN_EGG_b | CLUB_EGG_a | DIAMONDS_a))
                );
                theme.saveToFile(THEME_FILE);
                loadedTheme = theme;
                btn.SetText(L"♦");
            }
            else if (g_status & tUI_THEME_f) {

                int result = MessageBox(NULL, L"Do you want to clear the favorite theme?", L"Confirm Delete", MB_ICONQUESTION | MB_YESNO);
                if (result == IDYES) {
                    remove(THEME_FILE);
                    g_status &= ~(tUI_THEME_f | tUI_THEME_af | tUI_LOADED_f);
                    g_status |= REFLAG_tUI_f;
                    btn.SetText(L"◊");
                }
            }
        }
        });
}

void tUI_DRAW_BG_AND_BUTTONS() {
    bool cheer = false;
    if (g_status & PTRN_EGG_b) {
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        g_screen.DrawBackdropClearWhitespace();

        if (!cheer && HOLIDAY_FLAG) {
            tUI_DRAW_HOLIDAY_IMAGES();
        }

        setCursorPosition(50, 17);
        std::wcout << L"©░Quinnco.░2025";
    }

    if (!cheer && HOLIDAY_FLAG) {
        tUI_DRAW_HOLIDAY_IMAGES();
        setCursorPosition(50, 17);
        std::wcout << L"© Quinnco. 2025";
    }

    g_screen.DrawButtons();
}

void roll_new_color() {
    GET_NEW_COLOR_SCHEME();
    errorOut.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col2);
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);
}

void tUI_UPDATE_INTERFACE(std::function<void()>re_color, std::function<void()>draw_screen) {
    if (g_status & COLOR_EGG_b) {
        g_status &= ~COLOR_EGG_b;
        g_status &= ~tUI_THEME_af;
        roll_new_color();
        g_status |= RECOL_tUI_f | REFLAG_tUI_f;
    }
    if (g_status & RECOL_tUI_f) {
        g_status &= ~RECOL_tUI_f;
        re_color();
        g_status |= REDRAW_tUI_f;
    }
    if (g_status & REDRAW_tUI_f) {
        g_status &= ~REDRAW_tUI_f;

        if (!(g_status & CTRLR_SCREEN_f)) {
            g_screen.DrawBackdrop();
            if (g_status & PTRN_EGG_b) {
                MAKE_PATTERNS();
            }
        }
        draw_screen();
    }
    if (g_status & REFLAG_tUI_f) {
        g_status &= ~REFLAG_tUI_f;
        mouseButton* applyTheme = g_screen.GetButtonById(46);
        if (applyTheme == nullptr)
            return;
        if (!(g_status & tUI_THEME_af)) {
            applyTheme->SetText(L"◊");
            applyTheme->Update();
        }
        if (g_status & tUI_THEME_af) {
            applyTheme->SetText(L"♦");
            applyTheme->Update();
        }
    }
}

void tUI_HEARTS_OVER_PTRN() {
    static int block = 2;
    if (g_theme.getBlocks()[0] == 255) {
        // shaded ptrn
        if (block == g_theme.getBlocks()[1]) {
            block = block == 2 ? 1 : 2;
            wchar_t trg[2] = {
                getWcharFromByte(g_theme.getBlocks()[1]),
                L'\0'
            };
            wchar_t replace[2] = {
                getWcharFromByte(block),
                L'\0'
            };

            replaceXEveryNth(H_STAMP_S, heartStamp[0].GetLength(), trg, replace, heartStamp[0].GetLength());
            replaceXEveryNth(H_STAMP_M, heartStamp[1].GetLength(), trg, replace, heartStamp[1].GetLength());
            replaceXEveryNth(H_STAMP_L, heartStamp[2].GetLength(), trg, replace, heartStamp[2].GetLength());
        }
    }
    else {
        // diagonal ptrn
        wchar_t trg[2] = {
            getWcharFromByte(block),
            L'\0'
        };
        wchar_t replace[2] = {
            L' ',
            L'\0'
        };
        replaceXEveryNth(H_STAMP_S, heartStamp[0].GetLength(), trg, replace, heartStamp[0].GetLength());
        replaceXEveryNth(H_STAMP_M, heartStamp[1].GetLength(), trg, replace, heartStamp[1].GetLength());
        replaceXEveryNth(H_STAMP_L, heartStamp[2].GetLength(), trg, replace, heartStamp[2].GetLength());

    }

    int x = 0, y = 0, stamp = 0, w = 0, h = 0;
    COORD cornerPos[4] = { {1,1}, {consoleWidth - 1,0}, {0, consoleHeight}, {consoleWidth - 1, consoleHeight} };

    stamp = generateRandomInt(1, 2); // fisrt one M or L
    for (int i = 0; i < 4; i++) {
        w = heartStamp[stamp].GetWidth();
        h = heartStamp[stamp].GetLines();

        int startx = cornerPos[i].X - ((i % 2 == 1) ? w + 2 : 0);  // Adjust for right corners
        int starty = cornerPos[i].Y - ((i > 1) ? h + 2 : 0);       // Adjust for bottom corners

        x = generateRandomInt(startx, startx + 2);
        y = generateRandomInt(starty, starty + 2);

        heartStamp[stamp].SetPosition(x, y);
        heartStamp[stamp].Draw(DIAMOND_COLOR());
        stamp = generateRandomInt(0, 2);
    }
}

void tUI_SET_BG_AND_SHOULDER_BUTTONS() {
    if (g_status & PTRN_EGG_b) {
        if ((tUI_FLAGS() & (DIAMONDS_a | PTRN_EGG_b | BORDER_EGG_a)) == (DIAMONDS_a | PTRN_EGG_b)) {
            // Fullscreen Ptrn            
            drawPatternedArea({ 0,0,consoleWidth+1, consoleHeight+1 }, g_theme.getBlocks());
            if (tUI_FLAGS() & HEART_EGG_a) {
                tUI_HEARTS_OVER_PTRN();
            }
        }
        else {
            MAKE_PATTERNS();
        }
        button_L1_outline.SetText(button_L1_outline2);
        button_L2_outline.SetText(button_L2_outline2);
        button_R1_outline.SetText(button_L1_outline2);
        button_R2_outline.SetText(button_L2_outline2);
    }
    else {
        if (tUI_FLAGS() == DIAMONDS_a || tUI_FLAGS() == (DIAMONDS_a | HEART_EGG_a))
            tUI_CLEAR_PTRN_AREA(DIAMOND_COLOR());
        else
            tUI_CLEAR_PTRN_AREA(fullColorSchemes[g_currentColorScheme].controllerBg);
        
        button_L1_outline.SetText(button_L1_outline1);
        button_L2_outline.SetText(button_L2_outline1);
        button_R1_outline.SetText(button_R1_outline1);
        button_R2_outline.SetText(button_L2_outline1);
    }
}

// modifys default formatting and spacing, of messages on main screen
void tUI_SET_WIDE_MSG_LAYOUT(const char* ip) {
#ifdef JOYSENDER_TUI
    constexpr int width = 43;
    swprintf(msgPointer1, width, L"<< Connected To: %S >>", ip);
    cxMsg.SetPosition(22, 1, 38, 1, ALIGN_LEFT);
#else
    constexpr int width = 45;
    swprintf(msgPointer1, width, L"<< Connection From: %S >>", ip);
    cxMsg.SetPosition(25, 1, 38, 1, ALIGN_LEFT);
#endif
    output1.SetText(msgPointer1);
    output1.SetPosition(6, 1, width, 1, ALIGN_LEFT);
    output3.SetPosition(55, 1);
    fpsMsg.SetPosition(60, 1);
}

// for ptrn and hearts flag combo
void tUI_HEART_BORDER() {
    for (int i = 0; i < 18; i++) {
        setCursorPosition(4, 2 + i);
        std::wcout << L'♥';
        setCursorPosition(consoleWidth-4, 2 + i);
        std::wcout << L'♥';
    }
    setCursorPosition(4, 19);
    printX_Ntimes(L'♥', consoleWidth-8);
}

void tUI_DRAW_BORDER() {
#ifdef JOYSENDER_TUI
    wchar_t* backdrop = JoySendMain_Backdrop;
#else
    wchar_t* backdrop = JoyRecvMain_Backdrop;
#endif

    // draw top
    printSubsetOfBuffer({ 0,0 }, { 0,0,74,2 }, { 0,0,73,2 }, backdrop, true);
    //draw left 
    printSubsetOfBuffer({ 0,2 }, { 0,0,74,21 }, { 0,2,5,18 }, backdrop, true);
    //draw right 
    printSubsetOfBuffer({ 68,2 }, { 0,0,74,21 }, { 68,2,73,18 }, backdrop, true);
    //draw bottom
    printSubsetOfBuffer({ 0,18 }, { 0,18,74,20 }, { 0,18,73,21 }, backdrop, true);
}

void tUI_DRAW_NO_BORDER() {
    //draw top
    printSubsetOfBuffer({ 0,0 }, { 0,0,74,2 }, { 0,0,73,2 }, XBOX_Backdrop, true);
    //draw left 
    printSubsetOfBuffer({ 0,2 }, { 0,0,74,21 }, { 0,2,5,18 }, XBOX_Backdrop, true);
    //draw right 
    printSubsetOfBuffer({ 68,2 }, { 0,0,74,21 }, { 68,2,73,18 }, XBOX_Backdrop, true);
    //draw bottom
    printSubsetOfBuffer({ 0,18 }, { 0,18,74,20 }, { 0,18,73,21 }, XBOX_Backdrop, true);
}

void tUI_DRAW_CONTROLLER_FACE() {
    WORD controllerOverBG = (((tUI_FLAGS() == DIAMONDS_a) || (tUI_FLAGS() == (DIAMONDS_a | HEART_EGG_a))) ? fullColorSchemes[g_currentColorScheme].menuBg : fullColorSchemes[g_currentColorScheme].controllerBg);
    ReDrawControllerFace(g_screen, g_simpleScheme, controllerOverBG, g_mode, ((g_status & BORDER_EGG_a) || (g_status & PTRN_EGG_b) || (tUI_FLAGS() == (DIAMONDS_a | HEART_EGG_a)) || tUI_FLAGS() == DIAMONDS_a));
}

void tUI_RECOLOR_MAIN_LOOP() {
    g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].controllerBg);
    output1.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); //"FPS:"
    //output2.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); // not used on this screen
    output3.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col4); //" << Connection .."
    cxMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col1); // IP Address
    fpsMsg.SetColor(fullColorSchemes[g_currentColorScheme].menuColors.col3);  // fps value

    tUIColorPkg buttonColors = controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors);
    /* Color non-controller buttons */
    g_screen.SetButtonsColors(buttonColors);
    restartButtonCover.SetColor(DIAMOND_COLOR());

#ifdef JOYSENDER_TUI
    restartButton[3].SetColors(buttonColors);  // mode section of restart button
#endif

    COLOR_EGGS();
}

void tUI_REDRAW_MAIN_LOOP() {
    setTextColor(DIAMOND_COLOR());
    if (tUI_FLAGS() & BORDER_EGG_a ||
        tUI_FLAGS() == (DIAMONDS_a | HEART_EGG_a)) {
        tUI_DRAW_BORDER();
    }
    else{
        tUI_DRAW_NO_BORDER();
        if (tUI_FLAGS() == (PTRN_EGG_b | HEART_EGG_a)) {
            tUI_HEART_BORDER();
        }
    }

    tUI_SET_BG_AND_SHOULDER_BUTTONS();
    tUI_DRAW_CONTROLLER_FACE();
    g_screen.DrawButtons();

#ifdef JOYSENDER_TUI
    restartButtonCover.getText()[0] = getCharAtPosition(restartButtonCover.GetPosition().X, restartButtonCover.GetPosition().Y);
    restartButtonCover.getText()[1] = getCharAtPosition(restartButtonCover.GetPosition().X+1, restartButtonCover.GetPosition().Y);
    // in x64 release build the default parameter of -1 does not work??? debug show it set as 0. manually specifying value less then -1 here works
    restartButton[3].Draw(-2); 
#endif    

    output1.Draw();
    cxMsg.Draw();
    output3.Draw();
    PRINT_EGG_X();
}

void tUI_BUILD_MAIN_LOOP(Arguments& args) {
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

#ifdef JOYSENDER_TUI
    if (g_mode == 1) g_screen.AddButton(&mappingButton); 
    quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE);
    newColorsButton.SetPosition(consoleWidth / 2 - 8, QUITLINE - 3);

    restartButton[0].SetPosition(CONSOLE_WIDTH / 2 - 12, QUITLINE - 1);
    restartButton[1].SetPosition(CONSOLE_WIDTH / 2 - 8, QUITLINE - 1);
    restartButton[2].SetPosition(CONSOLE_WIDTH / 2 - 6, QUITLINE - 1);
    restartButton[3].SetPosition(CONSOLE_WIDTH / 2 + 6, QUITLINE - 1);
    restartButtonCover.SetPosition(CONSOLE_WIDTH / 2 + 13, QUITLINE - 1);
    restartButtonCover.setLength(2);
    restartButtonCover.getText()[2] = L'\0';

    g_screen.AddButton(&restartButton[1]);  // mode 1
    g_screen.AddButton(&restartButton[2]);  // mode 2
    g_screen.AddButton(&restartButton[0]);  // main restart
    //g_screen.AddButton(&restartButton[3]); // not needed in g_screen

    restartButton[0].setCallback(restartStatusCallback);
    restartButton[1].setCallback(restartModeCallback);
    restartButton[2].setCallback(restartModeCallback);

    mappingButton.setCallback(mappingButtonCallback);

    swprintf(msgPointer1, 40, L" << Connection To: %S  >> ", args.host.c_str());
    swprintf(cxPointer, 18, L" %S ", args.host.c_str());
    cxMsg.SetPosition(21, 1, 38, 1, ALIGN_LEFT);

#else
    swprintf(msgPointer1, 43, L" << Connection From: %S  >> ", connectionIP);
    swprintf(cxPointer, 18, L" %S ", connectionIP);
    cxMsg.SetPosition(23, 1, 38, 1, ALIGN_LEFT);
    quitButton.SetPosition(consoleWidth / 2 - 5, QUITLINE);
    newColorsButton.SetPosition(consoleWidth / 2 - 7, QUITLINE - 2);
    fpsMsg.SetPosition(51, 1);
#endif

    SetControllerButtonPositions(g_mode);
    g_screen.AddButton(&newColorsButton);
    g_screen.AddButton(&quitButton);

    // Sets controller to color scheme colors with some contrast correction for bg color
    SetControllerFace(g_screen, g_simpleScheme, fullColorSchemes[g_currentColorScheme].controllerBg, g_mode);
    g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]);

    output1.SetText(msgPointer1);
    output1.SetPosition(3, 1, 40, 1, ALIGN_LEFT);
    output3.SetPosition(46, 1, 5, 1, ALIGN_LEFT);
    output3.SetText(L" FPS:");

    cxMsg.SetText(cxPointer);

    if ((g_status & BORDER_EGG_a) || (g_status & tUI_LOADED_f)) {
        tUI_THEME_BUTTONS();
        HEARTS_N_SPADES_BUTTONS();
        tUI_SET_WIDE_MSG_LAYOUT(
#ifdef JOYSENDER_TUI
            args.host.c_str()
#else
            connectionIP
#endif
        );
    }
    else {
        g_screen.DeleteButton(46); // apply theme
        g_screen.DeleteButton(47); // save theme
    }
    if ((g_status & (BORDER_EGG_a | tUI_LOADED_f)) == tUI_LOADED_f) {
        tUI_SET_SUIT_POSITIONS(SUIT_POSITIONS_COLLECTED());
    }

    g_status |= CTRLR_SCREEN_f;
    if (theme_mtx.try_lock()) {
        theme_mtx.unlock();
        g_status |= RECOL_tUI_f | REDRAW_tUI_f;
        tUI_UPDATE_INTERFACE(tUI_RECOLOR_MAIN_LOOP, tUI_REDRAW_MAIN_LOOP);
    }
}

void tUI_THEME_AUTO_ACTIVATION()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch().count() % 69;
    if (!(g_status & tUI_LOADED_f) && (duration == 4 || duration == 20) && !(g_status & CLUB_EGG_a)) {
        MAKE_PATTERNS();
        g_status |= PTRN_EGG_b | REDRAW_tUI_f;
    }
    if (!(g_status & CLUB_EGG_a) && g_status & BORDER_EGG_a && generateRandomInt(1, 20) == 13) {
        CLEAN_CLUBBIN();
        g_status |= REDRAW_tUI_f;
    }
    if (!(g_status & BORDER_EGG_a) && !(g_status & CLUB_EGG_a) && generateRandomInt(1, 20) == 7) {
        MORPH_BORDER();
        g_status |= REDRAW_tUI_f;
    }
}

void tUI_THEME_RESTART() {
    // restart and no theme, carry over autos
    if (g_status & tUI_RESTART_f || !(g_status & tUI_LOADED_f)) {
        if (g_status & BORDER_EGG_a) {
            HEARTS_N_SPADES_BUTTONS();
            CLEAN_CLUBBIN();
        }
        g_status &= ~tUI_RESTART_f;
        g_status |= REDRAW_tUI_f | RECOL_tUI_f;
    }
}


int tUI_GET_HOLIDAY_FLAG() {
    std::time_t t = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &t);

    int day = localTime.tm_mday;
    int month = localTime.tm_mon + 1;
    int value = (month * 100) + day;

    return (g_HOLIDAYS.count(value) > 0) ? value : 0;
}
 
void tUI_DRAW_TITLE_AND_VERSION() {
#ifdef JOYSENDER_TUI
    setCursorPosition(9, 3);
    std::wcout << L"JoySender++";
    setCursorPosition(21, 3);
    std::wcout << L"tUI";
    setCursorPosition(25, 3);
    std::wcout << L"v" << APP_VERSION_NUM;
#else
    setCursorPosition(9, 3);
    std::wcout << L"JoyReceiver++";
    setCursorPosition(23, 3);
    std::wcout << L"tUI";
    setCursorPosition(27, 3);
    std::wcout << L"v" << APP_VERSION_NUM;
#endif
}

void tUI_LOAD_HOLIDAY() {

    struct THREE_IMAGES {
        wchar_t* img[3];
    };
    struct TWO_IMAGES {
        wchar_t* img[2];
    };
    wchar_t BLANK_IMAGE[1] = L"";

    // id's 69400-69406 reserved for heap allocated holiday images
    int idNum = HOLIDAY_BASE_ID;

    // loads a full set of holiday imagery into heap allocated mouseButtons
    auto Set_Holiday_Images = [&](THREE_IMAGES lg, TWO_IMAGES md /*, TWO_IMAGES sm*/, COORD lgPos, uint8_t lgWidth, COORD mdPos, uint8_t mdWidth) {
        // three large images
        for (int i = 0; i < 3; i++) {
            mouseButton* existing = holidayImages->GetButtonById(idNum);
            mouseButton* largeImage = existing ? existing : new mouseButton(lgPos.X, lgPos.Y, lgWidth, lg.img[i]);
            largeImage->SetId(idNum++);
            holidayImages->AddButton(largeImage);
        }

        // two medium images 
        for (int i = 0; i < 2; i++) {
            mouseButton* existing = holidayImages->GetButtonById(idNum);
            mouseButton* mediumImage = existing ? existing : new mouseButton(mdPos.X, mdPos.Y, mdWidth, md.img[i]);
            mediumImage->SetId(idNum++);
            holidayImages->AddButton(mediumImage);
        }

        // two small images -- not used...
        /*
        for (int i = 0; i < 2; i++) {
            mouseButton* existing = holidayImages->GetButtonById(idNum);
            mouseButton* smallImage = existing ? existing : new mouseButton(1, 2, 32, sm.img[i]);
            smallImage->SetId(idNum++);
            holidayImages->AddButton(smallImage);
        }
        */

        };

    //if (holidayImages != nullptr) return; // use these 
    //if (holidayImages->GetButtonById(idNum)) return; // for safety

    if (holidayImages == nullptr) {
        holidayImages = new textUI();
    }

    switch (HOLIDAY_FLAG){

    case 1: {  // TESTING
        
        Set_Holiday_Images(
            THREE_IMAGES{ VHEART_1, VHEART_2, VHEART_1 },
            TWO_IMAGES{ BLANK_IMAGE, BLANK_IMAGE },
            { 32,2 }, 33, { 5,6 }, 1
        );

       
    }break;

    case VALENTINES_DAY: { 
        Set_Holiday_Images(
            THREE_IMAGES{ VHEART_1, VHEART_2, VHEART_1 },
            TWO_IMAGES{ BLANK_IMAGE, BLANK_IMAGE },
            { 32,2 }, 33, { 0,0 }, 1
        );
    }break;

    case ST_PATRICKS_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ LEPERCHAUN, POT_O_GOLD, ST_PATS_HAT },
            TWO_IMAGES{ SHAMROCK_1, SHAMROCK_2 },
            { 3,2 }, 32, { 35,0 }, 36
        );
    }break;

    case APRIL_TWENTY_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ WEED_BANNER, WEED_LEAF, WEED_LEAF },
            TWO_IMAGES{ BLANK_IMAGE, LIGHTER_MEDUIM },
            { 28,2 }, 38, { 10,6 }, 10
        );
    }break;

    case CANADA_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ VIOLA_DESMOND, VIOLA_DESMOND, LOUIS_RIEL },
            TWO_IMAGES{ MAPLELEAF_MEDIUM, BEAVER_MEDIUM },
            { 32,1 }, 37, { 5,6 }, 30
        );
    }break;

    case NEVER_FORGET_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ NINE_11, BUSH_LARGE, NINE_11 },
            TWO_IMAGES{ BLANK_IMAGE, BLANK_IMAGE },
            { 3,1 }, 67, { 0,0 }, 1
        );
    }break;

    case HALLOWEEN_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ GHOST_LARGE, SKULL_LARGE, PUMPKIN_LARGE },
            TWO_IMAGES{ BAT_MEDIUM, CREEP_MEDIUM },
            { 38,2 }, 33, { 6,6 }, 21
        );
    }break;

    case CHRISTMAS_DAY: {
        Set_Holiday_Images(
            THREE_IMAGES{ GIFT_LARGE, REINDEER_LARGE, CHRISTMAS_TREE },
            TWO_IMAGES{ SANTA_MEDIUM, SNOWFLAKE_MEDIUM },
            { 31,2 }, 41, { 5,2 }, 34
        );
    }break;

    default:
        break;
    }
}

mouseButton* tUI_GET_HOLIDAY_IMAGE(uint8_t size) {
    if (holidayImages == nullptr) return nullptr;

    int id = HOLIDAY_BASE_ID;
    if (size == 3) { // Large
        id += generateRandomInt(0, 2);
    }
    else if (size == 2) { // Medium
        id += generateRandomInt(3, 4);
    }
    else if (size == 1) { // Small
        id += generateRandomInt(5, 6);
    }

    return holidayImages->GetButtonById(id);
}