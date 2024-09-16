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
#include <filesystem>
#include <string>
#include <functional>

constexpr auto APP_NAME = "NetJoy";
extern const char* THEME_FILE;
extern int screenLoop(textUI& screen);
void* g_extraData2 = nullptr;
int g_indexSelected = -1;

constexpr WORD THEME_SELECTED   =   0x01;
constexpr WORD THEME_EDIT       =   0x02;
constexpr WORD THEME_DONE       =   0x04;
constexpr WORD THEME_CANCEL     =   0x08;
constexpr WORD THEME_FAVED      =   0x10;
constexpr WORD REFRESH_PVIEW    =   0x20;
constexpr WORD SAVE_THEME       =   0x40;
constexpr WORD THEME_FINISHED   =   0x80;
constexpr WORD THEME_RANDOMIZE  =   0x01;
constexpr WORD THEME_COPY       =   0x02;

constexpr WORD SPEC_HEART_f = 0x0100;
constexpr WORD SPEC_SPADE_f = 0x0200;
constexpr WORD SPEC_CLUB_f = 0x0400;
constexpr WORD SPEC_HEART_a = 0x1000;
constexpr WORD SPEC_SPADE_a = 0x2000;
constexpr WORD SPEC_CLUB_a = 0x4000;

constexpr byte BGSTYLE_NONE     =   0x00;
constexpr byte BGSTYLE_PTRN     =   0x01;
constexpr byte BGSTYLE_SHADE1   =   0x02;
constexpr byte BGSTYLE_SHADE2   =   0x04;
constexpr byte BGSTYLE_SHADE3   =   0x08;
constexpr byte BGSTYLE_CHAR     =   0x10;

constexpr WORD defaultColor = (BLACK | WHITE AS_BG);

#define CURRENT_ACTIVE_SPECIALS g_status & (HEART_EGG_a | BORDER_EGG_a | PTRN_EGG_b)

void themeSelectCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        g_indexSelected = button.GetId();
    }
}

void elementSelectCallback(mouseButton& button) {
    if (button.Status() & MOUSE_UP) {
        button.SetStatus(MOUSE_OUT);
        g_indexSelected = button.GetId();
    }
}

void printX_Ntimes(const wchar_t X, const int N) {
    for (int i = 0; i < N; i++) {
        std::wcout << X;
    }
}

void drawPatternedArea(const RECT rect) {
    int w = (rect.right - rect.left), h = (rect.bottom - rect.top);
    if (h <= 0 || w <= 0)
        return;

    srand(static_cast<unsigned int>(time(0)));
    std::vector<wchar_t> rndBlocks = { L'▓', L'▒', L'█', L'▓', L'░', L'▒' };
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine rng(seed);
    std::shuffle(rndBlocks.begin(), rndBlocks.end(), rng);
    std::vector<wchar_t> blocks = {  L'▒', L'▒', L' ', L'▓', L'▒', L'▒', L'▒', rndBlocks[3], L'░', L'▒' };
    std::shuffle(blocks.begin(), blocks.end(), rng);

    int dir = generateRandomInt(-1, 0);
    if (!dir) dir = 1; // must be  -1 or 1
    int numBlocks = std::min(size_t(h + (w - 1)), blocks.size());

    if (dir == -1) {
        for (int y = 0; y < h; ++y) {
            setCursorPosition(rect.left, (rect.bottom - 1) - y);
            for (int x = 0; x < w; ++x) {
                std::wcout << blocks[(x + y) % numBlocks];
            }
        }
    }
    else {
        for (int y = 0; y < h; ++y) {
            setCursorPosition(rect.left, rect.top + y);
            for (int x = 0; x < w; ++x) {
                std::wcout << blocks[(x + y) % numBlocks];
            }
        }
    }
}

void drawDoubleBorderBox(int startX, int startY, int width, int height, const wchar_t* title, const wchar_t decoration = L'\0', byte bgStyle = BGSTYLE_NONE) {
    // Unicode double border characters
    const wchar_t topLeft = L'╔';
    const wchar_t topRight = L'╗';
    const wchar_t bottomLeft = L'╚';
    const wchar_t bottomRight = L'╝';
    const wchar_t horizontal = L'═';
    const wchar_t vertical = L'║';

    bool decorated = (decoration != L'\0');

    int titleLen = wcslen(title);

    // Top border
    setCursorPosition(startX, startY);
    if(decorated) 
        std::wcout << decoration;
    else
        std::wcout << topLeft;
    for (int i = 0; i < width - 2; ++i) {
        if (i > 2 && i < titleLen + 3) {
            std::wcout << title[i - 3];
        }
        else {
            std::wcout << horizontal;
        }
    }
    if (decorated)
        std::wcout << decoration;
    else
        std::wcout << topRight;

    if (bgStyle) {
        switch (bgStyle) {
        case BGSTYLE_PTRN: {
            drawPatternedArea({ startX + 1, startY + 1, startX + width - 1, startY + height - 1 });
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX, startY + i + 1);
                std::wcout << vertical;
            }
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX + width - 1, startY + i + 1);
                std::wcout << vertical;
            }
        }
                         break;
        case BGSTYLE_SHADE1: {
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX, startY + i + 1);
                std::wcout << vertical;
                printX_Ntimes(L'░', width - 2);
                std::wcout << vertical;
            }
        }
                           break;
        case BGSTYLE_SHADE2: {
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX, startY + i + 1);
                std::wcout << vertical;
                printX_Ntimes(L'▒', width - 2);
                std::wcout << vertical;
            }
        }
                           break;
        case BGSTYLE_SHADE3: {
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX, startY + i + 1);
                std::wcout << vertical;
                // L'▓', L'▒', L'█', L'▓', L'░', L'▒'
                printX_Ntimes(L'▓', width - 2);
                std::wcout << vertical;
            }
        }
                           break;
        case BGSTYLE_CHAR: {
            // must set g_indexSelected = (int)L'♣';
            for (int i = 0; i < height - 2; ++i) {
                setCursorPosition(startX, startY + i + 1);
                std::wcout << vertical;
                printX_Ntimes((wchar_t)g_indexSelected, width - 2);
                std::wcout << vertical;
            }
        }
                           break;
        }
    }
    else {

        // Middle section (vertical bars)
        for (int i = 0; i < height - 2; ++i) {
            setCursorPosition(startX, startY + i + 1);
            std::wcout << vertical;
            for (int j = 0; j < width - 2; ++j) {
                std::wcout << L' ';  // Empty space inside the box
            }
            std::wcout << vertical;
        }
    }

    // Bottom border
    setCursorPosition(startX, startY + height - 1);
    if (decorated)
        std::wcout << decoration;
    else
        std::wcout << bottomLeft;
    for (int i = 0; i < width - 2; ++i) {
        std::wcout << horizontal;
    }
    if (decorated)
        std::wcout << decoration;
    else
        std::wcout << bottomRight;
}

void printSubsetOfBuffer(const COORD pos, const RECT& inputRect, const RECT& outputRect, const wchar_t* source) {
    int inputWidth = inputRect.right - inputRect.left;
    int inputHeight = inputRect.bottom - inputRect.top;
    int outputWidth = outputRect.right - outputRect.left;
    int outputHeight = outputRect.bottom - outputRect.top;

    int innerStartX = outputRect.left;
    int innerStartY = outputRect.top;
    int innerEndX = outputRect.right;
    int innerEndY = outputRect.bottom;

    int bufferIndex = (innerStartY * inputWidth) + innerStartX;
    int x = 0;

    for (int y = 0; y < outputHeight; ++y) {
        setCursorPosition(pos.X, pos.Y + y);
        bufferIndex = (innerStartY+y) * inputWidth + innerStartX ;

        for (x = 0; x < outputWidth; ++x) {
            wchar_t ch = source[bufferIndex++];

            // Handle newlines explicitly
            if (ch == L'\n') {
                bufferIndex++;  // this shoulodnt happen fix your code
                break;          // Move to next line
            }
            std::wcout << ch;


        }
    }
}

WORD colorFromElementIndex(FullColorScheme& colors, int index) {
    switch (index) {
    case 0:
        return colors.controllerColors.col1;
    case 1:
        return colors.controllerColors.col2;
    case 2:
        return colors.controllerColors.col3;
    case 3:
        return colors.controllerColors.col4;
    case 4:
        return colors.menuColors.col2;
    case 5:
        return colors.menuColors.col1;
    case 6:
        return colors.menuColors.col3;
    case 7:
        return colors.menuColors.col4;
    case 8:
        return colors.controllerBg;
    case 9:
        return colors.menuBg;

    default:
        return 0;
    }

}

void updateColorByElementIndex(FullColorScheme& colors, int index, WORD newColor) {
    switch (index) {
    case 0:
        colors.controllerColors.col1 = newColor;
        break;
    case 1:
        colors.controllerColors.col2 = newColor;
        break;
    case 2:
        colors.controllerColors.col3 = newColor;
        break;
    case 3:
        colors.controllerColors.col4 = newColor;
        break;
    case 4:
        colors.menuColors.col2 = newColor;
        break;
    case 5:
        colors.menuColors.col1 = newColor;
        break;
    case 6:
        colors.menuColors.col3 = newColor;
        break;
    case 7:
        colors.menuColors.col4 = newColor;
        break;
    case 8:
        colors.controllerBg = newColor;
        break;
    case 9:
        colors.menuBg = newColor;
        break;

    default:
        return;
    }

}


void tUI_THEME_COLOR_PICKER(tUITheme& theme, mouseButton* elementBtns, int selected, mouseButton& rando, std::function<void()>draw_preview) {
    textUI pickerScreen;
    tUITheme tmpTheme = theme;
    WORD* state = (WORD*)g_extraData2;
    WORD selectedColor;

    constexpr int colorPick_x = 26, colorPick_y = 6, colorPick_w = 36, colorPick_h = 6;
    constexpr int palette_x = 26, palette_y = 12, palette_w = 37, palette_h = 7;

    mouseButton saveBtn((colorPick_x + colorPick_w) - 8, (colorPick_y + colorPick_h) - 4, 4, L" Ok ", 1);
    mouseButton cancelBtn((colorPick_x + colorPick_w) - 11, (colorPick_y + colorPick_h) - 2, 9, L" Cancel ", 2);
    mouseButton fgCol(colorPick_x + 6, colorPick_y + 2, 5, L"          ", 3);
    mouseButton bgCol(colorPick_x + 15, colorPick_y + 2, 5, L"          ", 4);
    {
        saveBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= SAVE_THEME;
            }
            });
        cancelBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_CANCEL;
            }
            });
        pickerScreen.AddButton(&saveBtn);
        pickerScreen.AddButton(&cancelBtn);

        fgCol.setCallback(&elementSelectCallback);
        bgCol.setCallback(&elementSelectCallback);
        pickerScreen.AddButton(&fgCol);
        pickerScreen.AddButton(&bgCol);
    }

    // add randomizer button
    pickerScreen.AddButton(&rando);

    // add elemnt buttons
    for (int i = 0; i < 10; i++) {
        pickerScreen.AddButton(&elementBtns[i]);
    }

    // create palette color buttons
    mouseButton paletteColors[16];
    {

        for (int i = 0; i < 16; i++) {
            paletteColors[i] = mouseButton(palette_x + 3 +(4*(i%8)), palette_y + 1 +((i>7)*3), 3, L"      ", 40 + i);
            paletteColors[i].SetAllColors({ WORD(i AS_BG) ,WORD(i AS_BG) ,WORD(i AS_BG) ,WORD(i AS_BG) });
            paletteColors[i].setCallback(&elementSelectCallback);
            paletteColors[i].SetSettings(INACTIVE);
            pickerScreen.AddButton(&paletteColors[i]);
        }
    }

    auto activate_palette_colors = [&]() {
        for (int i = 0; i < 16; i++) {
            paletteColors[i].SetSettings(0);
            paletteColors[i].Update();
        }
        };
    auto deactivate_palette_colors = [&]() {
        for (int i = 0; i < 16; i++) {
            paletteColors[i].SetSettings(INACTIVE);
        }
        };

    auto wait_for_palette_color = [&]() {
        textUI paletteScreen;
        paletteScreen.AddButton(&cancelBtn);
        for (int i = 0; i < 16; i++) {
            paletteScreen.AddButton(&paletteColors[i]);
        }

        while (!APP_KILLED && g_indexSelected == -1) {
            screenLoop(paletteScreen);

            if (*state & THEME_CANCEL) {
                break;
            }

            Sleep(100);
        }
        };

    // draw color palette
    auto draw_palette = [&](const wchar_t* txt) {
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        drawDoubleBorderBox(palette_x, palette_y, palette_w, palette_h, txt, L'\0', BGSTYLE_SHADE1);
        activate_palette_colors();
        };


    // draw color picker
    auto draw_picker = [&]() {
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        drawDoubleBorderBox(colorPick_x, colorPick_y, colorPick_w, colorPick_h, L"");
        setCursorPosition(colorPick_x + 2, colorPick_y);
        std::wcout << "Choose Color:" << elementBtns[selected].getTextPtr();

        // get fg/bg colors
        selectedColor = colorFromElementIndex(tmpTheme._colors, selected);
        fgCol.SetAllColors({ WORD((selectedColor FG_ONLY) AS_BG) ,WORD((selectedColor FG_ONLY) AS_BG) ,WORD((selectedColor FG_ONLY) AS_BG) ,WORD((selectedColor FG_ONLY) AS_BG) });
        bgCol.SetAllColors({ WORD(selectedColor BG_ONLY) ,WORD(selectedColor BG_ONLY) ,WORD(selectedColor BG_ONLY) ,WORD(selectedColor BG_ONLY) });

        // fg color
        COORD pos = fgCol.GetPosition();
        drawDoubleBorderBox(pos.X - 1, pos.Y - 1, 7, 4, L"");
        setCursorPosition(pos.X + 1, pos.Y - 1);
        std::wcout << "FG";

        // bg color
        pos = bgCol.GetPosition();
        drawDoubleBorderBox(pos.X - 1, pos.Y - 1, 7, 4, L"");
        setCursorPosition(pos.X + 1, pos.Y - 1);
        std::wcout << "BG";

        pickerScreen.DrawButtons();
        };
    
    draw_picker();

    while (!APP_KILLED && !(*state & THEME_CANCEL) && !(*state & SAVE_THEME)) {
        screenLoop(pickerScreen);

        if (*state & THEME_RANDOMIZE) {
            *state &= ~THEME_RANDOMIZE;
            updateColorByElementIndex(tmpTheme._colors, selected,
                makeSafeColors(generateRandomInt(0, 15) | (generateRandomInt(0, 15) AS_BG))
                );
            draw_picker();
        }

        if (g_indexSelected > -1) {

            if (g_indexSelected == 3) {
                g_indexSelected = -1;
                draw_palette(L"Choose Foreground Color");
                wait_for_palette_color();
                if (!(*state & THEME_CANCEL)) {
                    updateColorByElementIndex(tmpTheme._colors, selected, (selectedColor BG_ONLY | (g_indexSelected-40)));
                }
                else {
                    *state &= ~THEME_CANCEL;
                }
                deactivate_palette_colors();
                draw_preview();
                draw_picker();
            }
            if (g_indexSelected == 4) {
                g_indexSelected = -1;
                draw_palette(L"Choose Background Color");
                wait_for_palette_color();
                if (!(*state & THEME_CANCEL)) {
                    updateColorByElementIndex(tmpTheme._colors, selected, (selectedColor FG_ONLY | (g_indexSelected-40) AS_BG));
                }
                else {
                    *state &= ~THEME_CANCEL;
                }
                deactivate_palette_colors();
                draw_preview();
                draw_picker();
            }
            if (g_indexSelected > 59 && g_indexSelected < 70) {
                selected = g_indexSelected - 60;
                g_indexSelected = -1;
                draw_picker();
            }
        }
    }

    if (*state & SAVE_THEME) {
        theme = tmpTheme;
    }

    *state &= ~(SAVE_THEME | THEME_CANCEL);
}

void tUI_THEME_EDITOR_SCREEN(tUITheme& theme) {
    tUITheme workinTheme = theme;
    textUI screen;
    WORD* state = (WORD*)g_extraData2;

    constexpr int topBox_x = 1, topBox_y = 0, topBox_w = 58, topBox_h = 5;
    constexpr int pviewBox_x = 25, pviewBox_y = 5, pviewBox_w = consoleWidth-pviewBox_x, pviewBox_h = consoleHeight-(pviewBox_y-1);
    constexpr int elementsBox_x = 1, elementsBox_y = 4, elementsBox_w = 23, elementsBox_h = consoleHeight- (elementsBox_y-1)-3;
    constexpr int extrasBox_x = 1, extrasBox_y = consoleHeight-3, extrasBox_w = 17, extrasBox_h = 3;

    screen.SetBackdrop(JoyRecvMain_Backdrop);
    screen.SetBackdropColor(BLACK | BLACK AS_BG);

    mouseButton saveBtn(topBox_x + 3 , topBox_y + 2, 7, L" Save ", 0);
    mouseButton cancelBtn(topBox_x + 3+7, topBox_y + 2, 9, L" Cancel ", 1);
    mouseButton copyBtn(topBox_x + 3+7+9+10, topBox_y + 2, 15, L" Copy Current ", 2);
    mouseButton rndBtn(topBox_x + 3+7+9+15+10, topBox_y + 2, 12, L" Randomize ", 4);

    // preview buttons
    mouseButton screenBtnSample(pviewBox_x + 2, pviewBox_y + 13, 10, L" Click Me ", 5);
    mouseButton menuBtnSample(pviewBox_x + 31, pviewBox_y + 13, 10, L" Click Me ", 5);
        // controller buttons
    mouseButton stickOutlineSample(pviewBox_x+1, pviewBox_y+6, 3, L"--.\t\t\t--'");
    mouseButton startBtnOutline(pviewBox_x + 1, pviewBox_y + 4, 3, L"( )");
    mouseButton stickHiLightSample = button_RStickRight_highlight;
    mouseButton ctrlBtnOutline(pviewBox_x + 10, pviewBox_y + 2, 4, letterButton);
    mouseButton ctrlBtnHiLight = button_X_highlight;
    {
        screen.AddButton(&saveBtn);
        screen.AddButton(&cancelBtn);
        screen.AddButton(&copyBtn);
        screen.AddButton(&rndBtn);
        screen.AddButton(&screenBtnSample); 
        screen.AddButton(&menuBtnSample);

        screenBtnSample.SetSettings(INACTIVE);
        menuBtnSample.SetSettings(INACTIVE);

        stickHiLightSample.SetPosition(pviewBox_x + 4, pviewBox_y + 7);
        ctrlBtnHiLight.SetPosition(pviewBox_x + 7, pviewBox_y + 5);

        saveBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= SAVE_THEME;
            }
        });

        cancelBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_CANCEL;
            }
            });

        rndBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_RANDOMIZE;
            }
            });

        copyBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_COPY;
            }
            });
    }

    // blackout
    screen.DrawBackdrop();

    // Draw Top Box
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    drawDoubleBorderBox(topBox_x, topBox_y, topBox_w, topBox_h, L"Theme Editor: ", L'♣', BGSTYLE_SHADE1);
    setCursorPosition(topBox_x + 18, 0);
    std::wcout << theme._colors.name;

    auto draw_preview = [&]() {
        // Draw Preview
        setTextColor(workinTheme._colors.menuBg);
        drawDoubleBorderBox(pviewBox_x, pviewBox_y, pviewBox_w, pviewBox_h, L"Preview", L'♣');
        // draw controller section
        {
            // outline
            setTextColor(workinTheme._colors.controllerBg);
            printSubsetOfBuffer({ pviewBox_x + 1 , pviewBox_y + 1 }, { 0, 0, 74, 21 }, { 41,5,consoleWidth - 4, consoleHeight - 1 }, XBOX_Backdrop);

            // face
            setTextColor(workinTheme._colors.controllerColors.col1);
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 1);
            printX_Ntimes(L' ', 15);
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 2);
            std::wcout << "          ..    ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 3);
            std::wcout << "         |  |    ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 4);
            std::wcout << "( )   ..  `` ..   ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 5);
            std::wcout << "     |  |   |  |   ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 6);
            std::wcout << "--.   ``  .. ``     ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 7);
            std::wcout << "   }     |  |        ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 8);
            std::wcout << "--'       ``          ";
            setCursorPosition(pviewBox_x + 1, pviewBox_y + 9);
            std::wcout << "                       ";
            // handle
            setCursorPosition(pviewBox_x + 12, pviewBox_y + 10);
            std::wcout << "            ";
            setCursorPosition(pviewBox_x + 14, pviewBox_y + 11);
            std::wcout << "          ";
            setCursorPosition(pviewBox_x + 16, pviewBox_y + 12);
            std::wcout << "        ";
            setCursorPosition(pviewBox_x + 17, pviewBox_y + 13);
            std::wcout << ".     ,";
        }
        // draw controller screen messages
        {
            // bg fill
            setTextColor(workinTheme._colors.controllerBg);
            for (int i = 0; i < 4; i++) {
                setCursorPosition(pviewBox_x + 28, pviewBox_y + 1 + i);
                printX_Ntimes(L' ', 18);
            }

            // connection message
            setCursorPosition(pviewBox_x + 2, pviewBox_y + 1);
            setTextColor(workinTheme._colors.menuColors.col4);
            std::wcout << " << Connection: 127.0.0.1 >> ";
            setCursorPosition(pviewBox_x + 17, pviewBox_y + 1);
            setTextColor(workinTheme._colors.menuColors.col1);
            std::wcout << " 127.0.0.1 ";

            // fps message
            setCursorPosition(pviewBox_x + 33, pviewBox_y + 1);
            setTextColor(workinTheme._colors.menuColors.col4);
            std::wcout << " FPS:";
            setCursorPosition(pviewBox_x + 38, pviewBox_y + 1);
            setTextColor(workinTheme._colors.menuColors.col3);
            std::wcout << " 33.86 ";

            // error message
            setCursorPosition(pviewBox_x + 23, pviewBox_y + 3);
            setTextColor(workinTheme._colors.menuColors.col2);
            std::wcout << " << Error Message >> ";
        }
        // draw screen/controller buttons
        {
            tUIColorPkg buttonColors = controllerButtonsToScreenButtons(workinTheme._colors.controllerColors);
            // buttons
            screenBtnSample.SetSettings(0);
            screenBtnSample.SetColors(buttonColors);
            menuBtnSample.SetColors(buttonColors);

            stickOutlineSample.SetDefaultColor(workinTheme._colors.controllerColors.col2);
            startBtnOutline.SetDefaultColor(workinTheme._colors.controllerColors.col2);
            ctrlBtnOutline.SetDefaultColor(workinTheme._colors.controllerColors.col2);
            ///

            stickHiLightSample.SetDefaultColor(workinTheme._colors.controllerColors.col4);
            ctrlBtnHiLight.SetDefaultColor(workinTheme._colors.controllerColors.col4);

            screenBtnSample.Update();

            ctrlBtnHiLight.Draw();
            startBtnOutline.Draw();
            stickHiLightSample.Draw();
            stickOutlineSample.Draw();

            // Y pos
            ctrlBtnOutline.SetPosition(pviewBox_x + 10, pviewBox_y + 2);
            ctrlBtnOutline.Draw();
            // B pos
            ctrlBtnOutline.SetPosition(pviewBox_x + 13, pviewBox_y + 4);
            ctrlBtnOutline.Draw();
            // A pos
            ctrlBtnOutline.SetPosition(pviewBox_x + 10, pviewBox_y + 6);
            ctrlBtnOutline.Draw();
            // X pos
            ctrlBtnOutline.SetPosition(pviewBox_x + 6, pviewBox_y + 4);
            ctrlBtnOutline.SetDefaultColor(workinTheme._colors.controllerColors.col4 FG_ONLY | workinTheme._colors.controllerColors.col1 BG_ONLY);
            ctrlBtnOutline.Draw();

        }

        // menu preview
        {
            setTextColor(workinTheme._colors.menuBg);
            drawDoubleBorderBox(pviewBox_x + 22, pviewBox_y + 5, 24, (20 - pviewBox_y) - 4, L"Menu");
            setCursorPosition(55, 20);
            std::wcout << L"© Quinnco. 2024";

            // message 1
            setCursorPosition(pviewBox_x + 26, pviewBox_y + 7);
            setTextColor(workinTheme._colors.menuColors.col1);
            std::wcout << " Message Box 1 ";

            // message 2
            setCursorPosition(pviewBox_x + 26, pviewBox_y + 9);
            setTextColor(workinTheme._colors.menuColors.col3);
            std::wcout << " Message Box 2 ";

            // message 3
            setCursorPosition(pviewBox_x + 26, pviewBox_y + 11);
            setTextColor(workinTheme._colors.menuColors.col4);
            std::wcout << " Message Box 3 ";

            // button
            menuBtnSample.SetSettings(0);
            menuBtnSample.Update();
        }

        };

    // Draw Elements select
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    drawDoubleBorderBox(elementsBox_x, elementsBox_y, elementsBox_w, elementsBox_h, L"UI Element");
    mouseButton elementBtns[10];
    {
        elementBtns[0] = mouseButton(elementsBox_x + 3, elementsBox_y + 2, 16, L" Gamepad Color ", 60);
        elementBtns[1] = mouseButton(elementsBox_x + 3, elementsBox_y + 3, 18, L" Button Inactive ", 61);
        elementBtns[2] = mouseButton(elementsBox_x + 3, elementsBox_y + 4, 19, L" Button Highlight ", 62);
        elementBtns[3] = mouseButton(elementsBox_x + 3, elementsBox_y + 5, 16, L" Button Active ", 63);

        elementBtns[4] = mouseButton(elementsBox_x + 3, elementsBox_y + 6, 16, L" Error Message ", 64);
        elementBtns[5] = mouseButton(elementsBox_x + 3, elementsBox_y + 7, 12, L" Message 1 ", 65);
        elementBtns[6] = mouseButton(elementsBox_x + 3, elementsBox_y + 8, 12, L" Message 2 ", 66);
        elementBtns[7] = mouseButton(elementsBox_x + 3, elementsBox_y + 9, 12, L" Message 3 ", 67);

        elementBtns[8] = mouseButton(elementsBox_x + 3, elementsBox_y + 10, 13, L" Gamepad Bg ", 68);
        elementBtns[9] = mouseButton(elementsBox_x + 3, elementsBox_y + 11, 10, L" Menu Bg ", 69);

        for (int i = 0; i < 10; i++) {
            elementBtns[i].setCallback(&elementSelectCallback);
            screen.AddButton(&elementBtns[i]);
        }
    }


    auto set_toggle_active = [&](mouseButton& toggleBtn) {
        toggleBtn.SetDefaultColor(elementBtns[0].getSelectColor());
        toggleBtn.SetHighlightColor((elementBtns[0].getHighlightColor() FG_ONLY | elementBtns[0].getSelectColor() BG_ONLY));
        toggleBtn.SetActiveColor(elementBtns[0].getDefaultColor());
        toggleBtn.Update();
        };

    auto set_toggle_inactive = [&](mouseButton& toggleBtn) {
        toggleBtn.SetAllColors(elementBtns[0].GetColors());
        toggleBtn.Update();
        };

    // Draw Extras
    setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
    drawDoubleBorderBox(extrasBox_x, extrasBox_y, extrasBox_w, extrasBox_h, L"Special", L'♣');
    mouseButton heartToggle(extrasBox_x + 3, extrasBox_y + 1, 3, L" ♥ ", 70);
    mouseButton spadeToggle(extrasBox_x + 3 + 4, extrasBox_y + 1, 3, L" ♠ ", 71);
    mouseButton borderToggle(extrasBox_x + 3 + 8, extrasBox_y + 1, 3, L" ♣ ", 72);
    {
        screen.AddButton(&heartToggle);
        screen.AddButton(&spadeToggle);
        screen.AddButton(&borderToggle);

        heartToggle.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= SPEC_HEART_f;
            }
            });
        spadeToggle.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= SPEC_SPADE_f;
            }
            });
        borderToggle.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= SPEC_CLUB_f;
            }
            });

        if (workinTheme.getState() & HEART_EGG_a) {
            *state |= SPEC_HEART_a;
            set_toggle_active(heartToggle);
        }
        if (workinTheme.getState() & PTRN_EGG_b) {
            *state |= SPEC_SPADE_a;
            set_toggle_active(spadeToggle);
        }
        if (workinTheme.getState() & BORDER_EGG_a) {
            *state |= SPEC_CLUB_a;
            set_toggle_active(borderToggle);
        }
    }

    draw_preview();
    screen.DrawButtons();

    while (!APP_KILLED && !(*state & THEME_CANCEL)) {
        screenLoop(screen);

        // color picker for elements
        if (g_indexSelected > 59 && g_indexSelected < 70) {
            int selected = g_indexSelected - 60;
            tUI_THEME_COLOR_PICKER(workinTheme, elementBtns, selected, rndBtn, draw_preview);
            g_indexSelected = -1;
            draw_preview();
        }

        // specal toggles
        {
            if (*state & SPEC_HEART_f) {
                *state &= ~SPEC_HEART_f;
                if (*state & SPEC_HEART_a) {
                    *state &= ~SPEC_HEART_a;
                    set_toggle_inactive(heartToggle);
                }
                else {
                    *state |= SPEC_HEART_a;
                    set_toggle_active(heartToggle);
                }
            }
            if (*state & SPEC_SPADE_f) {
                *state &= ~SPEC_SPADE_f;
                if (*state & SPEC_SPADE_a) {
                    *state &= ~SPEC_SPADE_a;
                    set_toggle_inactive(spadeToggle);
                }
                else {
                    *state |= SPEC_SPADE_a;
                    set_toggle_active(spadeToggle);
                }
            }
            if (*state & SPEC_CLUB_f) {
                *state &= ~SPEC_CLUB_f;
                if (*state & SPEC_CLUB_a) {
                    *state &= ~SPEC_CLUB_a;
                    set_toggle_inactive(borderToggle);
                }
                else {
                    *state |= SPEC_CLUB_a;
                    set_toggle_active(borderToggle);
                }
            }
        }

        if (*state & THEME_COPY) {
            *state &= ~THEME_COPY;

            workinTheme.setColors(fullColorSchemes[g_currentColorScheme]);
            workinTheme.setState(CURRENT_ACTIVE_SPECIALS);

            draw_preview();
        }

        if (*state & THEME_RANDOMIZE) {
            *state &= ~THEME_RANDOMIZE;
            ColorScheme menuScheme = createRandomScheme();
            WORD newRandomBG = generateRandomInt(0, 15) AS_BG;
            ColorScheme randomScheme = createRandomScheme();
            workinTheme._colors = fullSchemeFromSimpleScheme(randomScheme, menuScheme, newRandomBG);
            draw_preview();
        }

        if (*state & SAVE_THEME) {
            workinTheme.setState((
                (*state& SPEC_HEART_a) ? HEART_EGG_a : 0 |
                (*state & SPEC_SPADE_a) ? PTRN_EGG_b : 0 |
                (*state & SPEC_CLUB_a) ? BORDER_EGG_a : 0 )
            );
            theme = workinTheme;
            break;
        }

        Sleep(100);
    }

    screen.DrawBackdrop();
    *state &= ~THEME_CANCEL;
}

void tUI_THEME_SELECTOR_SCREEN() {
    textUI screen;
    tUITheme tmpTheme;
    FullColorScheme tmpScheme;
    int selected = -1;
    WORD state = 0;
    g_extraData2 = (void*)&state;
    g_indexSelected = -1;

    constexpr int CUSTOM_THEMES = 4;
    constexpr int menu_x = 2, menu_y = 0, menu_w = consoleWidth - 4, menu_h = 7;
    constexpr int optns_x = 40, optns_y = 6, optns_w = 27, optns_h = 7;

    wchar_t customThemeNames[CUSTOM_THEMES][11] = { 0 };

    // custom theme pathing
    std::string appdataFolder = g_getenv("APPDATA");
    appdataFolder += "\\" + std::string(APP_NAME);
    std::filesystem::path saveFolder = std::filesystem::path(appdataFolder);
    std::filesystem::path filename = "";
    auto gen_custom_filename = [&]() {
        filename = saveFolder / ("cust" + std::to_string(selected - NUM_COLOR_SCHEMES) + ".theme");
        };


    // Select Theme Buttons
    mouseButton colorThemes[NUM_COLOR_SCHEMES + CUSTOM_THEMES + 1];
    int offset = 0;
      // predefined themes
    for (int i = 1; i < NUM_COLOR_SCHEMES + 1; ++i) {
        int x = menu_x + 3 + offset;
        int namelen = wcslen(fullColorSchemes[i].name);
        offset += namelen + 3;
        colorThemes[i] = mouseButton(x, menu_y+2, namelen, fullColorSchemes[i].name, i);
        colorThemes[i].SetSettings(PADDED_TEXT);
        colorThemes[i].setCallback(&themeSelectCallback);
        screen.AddButton(&colorThemes[i]);
    }
      // custom themes
    offset = 0;
    for (int i = 0; i < CUSTOM_THEMES; ++i) {
        int x = menu_x + 3 + offset;
        int namelen = 8;
        swprintf_s(customThemeNames[i], 11, L"Custom %d", i + 1);

        offset += namelen + 3;
        colorThemes[NUM_COLOR_SCHEMES + i + 1] = mouseButton(x, menu_y+4, namelen, customThemeNames[i], NUM_COLOR_SCHEMES + i + 1);
        colorThemes[NUM_COLOR_SCHEMES + i + 1].SetSettings(PADDED_TEXT);
        colorThemes[NUM_COLOR_SCHEMES + i + 1].setCallback(&themeSelectCallback);
        screen.AddButton(&colorThemes[NUM_COLOR_SCHEMES + i + 1]);
    }

    // Done button
    mouseButton doneBtn(consoleWidth / 2 - 3, consoleHeight - 2, 6, L" Done ");
    doneBtn.setCallback([](mouseButton& btn) {
        if (btn.Status() & MOUSE_UP) {
            btn.SetStatus(MOUSE_OUT);
            WORD* state = (WORD*)g_extraData2;
            *state |= THEME_FINISHED;
        }
        });
    screen.AddButton(&doneBtn);

    // Theme Options Box & Buttons
    mouseButton setActiveBtn(optns_x + 3, optns_y + 2, 13, L" Set Active ");
    mouseButton setFavBtn(optns_x + 3, optns_y + 4, 16, L" Set Favourite ");
    mouseButton editThemeBtn(optns_x + 17, optns_y + 2, 7, L" Edit ");
    {
        setActiveBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_SELECTED;
            }
            });
        setActiveBtn.SetId(50);

        setFavBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_FAVED;
            }
            });
        setFavBtn.SetId(52);

        editThemeBtn.setCallback([](mouseButton& btn) {
            if (btn.Status() & MOUSE_UP) {
                btn.SetStatus(MOUSE_OUT);
                WORD* state = (WORD*)g_extraData2;
                *state |= THEME_EDIT;
            }
            });
        editThemeBtn.SetId(51);
        screen.AddButton(&editThemeBtn);
        screen.AddButton(&setActiveBtn);
        screen.AddButton(&setFavBtn);
    }

    auto draw_menu = [&]() {
        // Select Theme Box/Menu
          // clear screen
        setTextColor(fullColorSchemes[g_currentColorScheme].menuBg);
        for (int i = 0; i < consoleHeight; ++i) {
            setCursorPosition(0, i);
            printX_Ntimes(L' ', 74);
        }
          // draw menu area
        drawDoubleBorderBox(menu_x, menu_y, menu_w, menu_h, L"Select Theme", L'♣', BGSTYLE_SHADE2);
          // reset option buttons
        editThemeBtn.SetSettings(INACTIVE);
        setActiveBtn.SetSettings(INACTIVE);
        setFavBtn.SetSettings(INACTIVE);
          // color and draw buttons
        screen.SetButtonsColors(controllerButtonsToScreenButtons(fullColorSchemes[g_currentColorScheme].controllerColors));
        screen.DrawButtons();
        };

    draw_menu();

    //////////////////////////////
    // Loop
    while (!APP_KILLED && !(state & THEME_FINISHED)) {
        screenLoop(screen);

        // Select A Theme
        if (g_indexSelected > -1) {
            selected = g_indexSelected;
            g_indexSelected = -1;

            // load theme to tmpTheme
            if (selected < NUM_COLOR_SCHEMES + 1) {
                tmpTheme.recordTheme(fullColorSchemes[selected], CURRENT_ACTIVE_SPECIALS);

                screen.SetButtonsColorsById(controllerButtonsToScreenButtons(fullColorSchemes[selected].controllerColors), { 50,51,52 });
                setTextColor(tmpTheme._colors.menuBg);
                drawDoubleBorderBox(optns_x, optns_y, optns_w, optns_h, fullColorSchemes[selected].name);
            }
            else {
                gen_custom_filename();
                if (!tmpTheme.loadFromFile(filename.string().c_str())) {
                    // no existing file, generate a random theme 
                    ColorScheme menuScheme = createRandomScheme();
                    WORD newRandomBG = generateRandomInt(0, 15) AS_BG;
                    ColorScheme randomScheme = createRandomScheme();
                    tmpScheme = fullSchemeFromSimpleScheme(randomScheme, menuScheme, newRandomBG);

                    // save as custom theme file
                    tmpTheme.setColors(tmpScheme);
                    tmpTheme.saveToFile(filename.string().c_str());
                }
                tmpTheme._colors.name = customThemeNames[selected - (NUM_COLOR_SCHEMES + 1)];

                screen.SetButtonsColorsById(controllerButtonsToScreenButtons(tmpTheme._colors.controllerColors), { 50,51,52 });
                setTextColor(tmpTheme._colors.menuBg);
                drawDoubleBorderBox(optns_x, optns_y, optns_w, optns_h, tmpTheme._colors.name, L'♣');
            }

            // draw buttons
            setActiveBtn.SetSettings(0);
            setFavBtn.SetSettings(0);
            if (selected == 0 || selected > NUM_COLOR_SCHEMES) {
                editThemeBtn.SetSettings(0);
            }
            else{
                editThemeBtn.SetSettings(INACTIVE);
            }
            setActiveBtn.Update();
            setFavBtn.Update();
            editThemeBtn.Update();
        }

        // Set Active
        if (state & THEME_SELECTED) {
            state &= ~THEME_SELECTED;
            
            // make selected theme the active theme
            if (selected == 0 || selected > NUM_COLOR_SCHEMES) {
                g_currentColorScheme = RANDOMSCHEME;
            }
            else {
                g_currentColorScheme = selected;
            }

            if (!(tmpTheme == g_theme)){
                g_status &= ~tUI_THEME_af;
                tmpTheme._colors.name = L"Custom Theme";
                tmpTheme.restoreColors(fullColorSchemes[RANDOMSCHEME]);
                g_theme = tmpTheme;
                g_screen.SetBackdropColor(fullColorSchemes[g_currentColorScheme].menuBg);
                g_simpleScheme = simpleSchemeFromFullScheme(fullColorSchemes[g_currentColorScheme]); /* for DrawControllerFace */
                g_status |= tUI_LOADED_f | REFLAG_tUI_f | REDRAW_tUI_f | RECOL_tUI_f;
            }   
            draw_menu();
        }

        // Set Favourite
        if (state & THEME_FAVED) {
            state &= ~THEME_FAVED;
            if (tmpTheme == g_theme) {
                g_status |= tUI_THEME_af;
            }
            else {
                g_status &= ~tUI_THEME_af;
            }
            tmpTheme.saveToFile(THEME_FILE);
            g_status |= tUI_THEME_f | REFLAG_tUI_f;
        }

        // Edit Selected
        if (state & THEME_EDIT) {
            state &= ~THEME_EDIT;
            // do the editing thing
            tUI_THEME_EDITOR_SCREEN(tmpTheme);
            if (state & SAVE_THEME) {
                state &= ~SAVE_THEME;
                tmpTheme.saveToFile(filename.string().c_str());
            }

            // reset
            draw_menu();
        }

        Sleep(100);
    }

    screen.ClearButtons();
    g_indexSelected = -1;
    g_extraData2 = nullptr;
}
