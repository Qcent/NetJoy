///  
/// ANIMATIONS
///  
int g_frameNum = 0;  //
const int CX_ANI_FRAME_COUNT = 12;

const wchar_t* ConnectAnimation[CX_ANI_FRAME_COUNT + 1] = {
    {L" (>                               ) "},
    {L" (<>                              ) "},
    {L" (   <>                           ) "},
    {L" (      <>                        ) "},
    {L" (         <>                     ) "},
    {L" (            <>                  ) "},
    {L" (               <>               ) "},
    {L" (                  <>            ) "},
    {L" (                     <>         ) "},
    {L" (                        <>      ) "},
    {L" (                           <>   ) "},
    {L" (                              <>) "},
    {L" (                               <) "}
};

const wchar_t* ConnectAnimationLeft[CX_ANI_FRAME_COUNT + 1] = {
    {L"     "},
    {L"   . "},
    {L"  .. "},
    {L" ... "},
    {L" ... "},
    {L"  .. "},
    {L"   . "},
    {L"     "},
    {L" * * "},
    {L"  *  "},
    {L" * * "},
    {L"  *  "},
    {L" *** "},
};

const wchar_t* ConnectAnimationRight[CX_ANI_FRAME_COUNT + 1] = {
    {L"     "},
    {L" .   "},
    {L" ..  "},
    {L" ... "},
    {L" ... "},
    {L" ..  "},
    {L" .   "},
    {L"     "},
    {L" * * "},
    {L"  *  "},
    {L" * * "},
    {L"  *  "},
    {L" *** "},
};

const int FOOTER_ANI_FRAME_COUNT = 9;
wchar_t FooterAnimation[FOOTER_ANI_FRAME_COUNT + 1][31] = {
{L" .--.     :::::.\\:::'      `--"},
{L".--.      ::::.\\::::      `--'"},
{L"--.      .:::*\\:::::     `--' "},
{L"-.      .-::.\\::::::    `--'  "},
{L".      .--:.\\:::::::   `--'   "},
{L"      .--..\\::::::::  `--'    "},
{L"     .--. \\::::::::. `--'     "},
{L"    .--.  ::::::::.\\`--'      "},
{L"   .--.   :::::::.\\:--'      *"},
{L"  .--.    ::::::.\\::-'      `-"},
};

// function will inc/dec &counter up to maxCount and down to 0
// for animating back and forth through frames
__declspec(noinline) // gets optimized out in Release x64? yes!
void countUpDown(int& counter, int maxCount) { // * a counter value outside of (0 - maxCount) will cause issues
    static int dir = 1; // 1: up, -1: down

    // this non intuitive block makes sure we always count up from 0
    if (!counter) {
        dir = -1;
    }

    // if 0 or maxCount, direction of counting will be reversed
    if (!(counter < maxCount && counter > 0)) {
        dir *= -1;
    }

    counter += dir; // count by one in direction +1 or -1
}

// function will loop from 0 to maxCount
// for animation cyclically through frames
void loopCount(int& counter, int maxCount) {
    counter = (counter + 1) % (maxCount + 1);
}

// function will loop from maxCount to 0
// for animation cyclically through frames
void revLoopCount(int& counter, int maxCount) {
    --counter;
    if (counter < 0)
        counter = maxCount-1;
}

// for smooth and controllable animation advance frames in a separate thread
// not currently used
void threadedFrameAdvance(int& run, int delay, int& counter, int maxCount) {
    while (!APP_KILLED && run) {
        Sleep(delay);
        loopCount(counter, maxCount);
    }
}

__declspec(noinline)
void printDiagonalPattern(int top, int bottom, int start_x, int start_y, int width, int step, const wchar_t* block = L"░", int dir = 1) {
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
