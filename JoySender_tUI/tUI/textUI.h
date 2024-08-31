#include <io.h>
#include <fcntl.h>
#include <map>
#include <set>
#include <vector>

#include "textUI.hpp"
#include "backdrops.h"
#include "uiButtons.h"
#include "controllerFaces.h"

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