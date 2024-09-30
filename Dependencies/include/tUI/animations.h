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

const int SEND_FOOTER_ANI_FRAME_COUNT = 15;
wchar_t SendFooterAnimation[SEND_FOOTER_ANI_FRAME_COUNT + 1][31] = {
{L"  .+-.    :.:::.\\:::'      `--"},
{L"   __     :::::.~:::          "},
{L"          -::::.----          "},
{L"        __::::::~.::          "},
{L".      .+-:^::::/.::  `--'    "},
{L"        __::::::~.::          "},
{L"          :------:::          "},
{L"   __     :~.:::::::          "},

{L"  .-+.    :/.:::^:::'      `--"},
{L"   __     :~.:::::::          "},
{L"          -::::.----          "},
{L"        __:~:::::::.          "},
{L".      .-+:\\::::.::.  `--'    "},
{L"        __:~:::::::.          "},
{L"          :------:::          "},
{L"   __     :::::.~:::          "},
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

wchar_t rsideFooter[] = L"\t/   \t\t:}-     ";


wchar_t H_STAMP_S[45] = L"\t▒▒\t▒▒\t▒\t\t▒\t\t▒\t▒\t\t\t▒\t\t\t▒\t▒\t\t\t\t\t▒\t\t\t";

wchar_t H_STAMP_M[66] = L"\t▒▒▒\t▒▒▒\t▒\t\t\t▒\t\t\t▒\t▒\t\t\t\t\t▒\t\t\t▒\t\t\t▒\t\t\t\t\t▒\t▒\t\t\t\t\t\t\t▒\t\t\t\t";

wchar_t H_STAMP_L[128] = L"\
\t\t▒▒▒▒\t\t\t▒▒▒▒\t\t\
▒\t\t\t\t\t▒\t▒\t\t\t\t\t▒\
▒\t\t\t\t\t\t▒\t\t\t\t\t\t▒\
\t▒\t\t\t\t\t\t\t\t\t\t\t▒\t\
\t\t▒\t\t\t\t\t\t\t\t\t▒\t\t\
\t\t\t\t▒\t\t\t\t\t▒\t\t\t\t\
\t\t\t\t\t\t▒\t▒\t\t\t\t\t\t\
\t\t\t\t\t\t\t▒\t\t\t\t\t\t\t";

mouseButton heartStamp[3] = {
    {mouseButton(0,0,7,H_STAMP_S)},
    {mouseButton(0,0,9,H_STAMP_M)},
    {mouseButton(0,0,15,H_STAMP_L)}
};


const wchar_t C_STAMP[365] = L"\t\t\t\t\t\t\t\t\t\t\t▒▒▒▒\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒▒░░░░▒▒▒\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒░░░░░░░░░░▒▒\t\t\t\t\t\t\t\t\t\t\t\t▒▒░░░░░░░░░░▒▒\t\t\t\t\t\t\t\t\t\t\
\t\t\t▒░░░░░░░░░░▒\t\t\t\t\t\t\t\t\t\t▒▒▒▒░░░░░░░░░░░░▒▒▒▒\t\t\t\t▒▒░░░░░░░░░░░░░░░░░░░░▒▒\t\
▒▒░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒░░░░░░░░░░░░░░░░░░░░░░▒▒\t\t▒▒░░░░░▒▒▒▒▒▒▒▒░░░░░▒▒\t\t\t\t\t\t▒▒▒▒▒\t\t\t\
▒▒\t\t\t▒▒▒▒▒\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒▒▒\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒▒▒▒▒▒▒▒▒\t\t\t\t\t\t\t\t";

#define CLUB_STAMP_POS 8, 7
mouseButton clubStamp(CLUB_STAMP_POS, 26, C_STAMP);

//wchar_t blockys[] = L"█ ▓ ▒ ░ ' ' ";
void drawClubShine(int step, bool shineOn = true, int dir = 1) {
    int ob = 0;
    wchar_t ch = L' ';
    constexpr COORD start = { CLUB_STAMP_POS };
    short x = start.X;
    short y = start.Y + step;
    const int height = clubStamp.GetLines();
    while (ob < 1) {
        // bottom-left to top-right
        if (x >= start.X && pointIsMasked({x,y}, clubMASK)) {
            ob = 0;
            if(shineOn)
                ch = (getCharAtPosition(x, y) == L'▒' ? L'░' : L' ');
            else
                ch = (getCharAtPosition(x, y) == L'░' ? L'▒' : L'░');

            setCursorPosition(x, y);
            std::wcout << ch;
        }
        // up and right
        y--;
        x++;
        if (y < start.Y){
            ob++;
        }
    }
}

int stepAroundScreen(COORD& pos, short screenWidth, short screenHeight, short step) {
    bool OnTop = (pos.Y == 0);
    bool OnLeft = (pos.X == 0);
    bool OnRight = (pos.X == screenWidth);;
    bool OnBottom = (pos.Y == screenHeight);

    short moveTo = 0;

    if (OnTop) {
        if (!(OnLeft || OnRight)) {
            moveTo = pos.X + step;
            if (moveTo > screenWidth) {
                pos.X = screenWidth;
                pos.Y += (moveTo - screenWidth)/2; // divide by 2 to compensate for width to height ratio
                return 2;
            }
            else if (moveTo < 0) {
                pos.X = 0;
                pos.Y += (-1 * moveTo)/2;
                return 1;
            }
            else {
                pos.X = moveTo;
            }
            return 0;
        }
    }

    else if (OnBottom) {
        if (!(OnLeft || OnRight)) {
            moveTo = pos.X - step;
            if (moveTo > screenWidth) {
                pos.X = screenWidth;
                pos.Y -= (moveTo - screenWidth) / 2; // divide by 2 to compensate for width to height ratio
                return 4;
            }
            else if (moveTo < 0) {
                pos.X = 0;
                pos.Y += moveTo/2;
                return 3;
            }
            else {
                pos.X = moveTo;
            }
            return 0;
        }
    }

    if (OnLeft) {
        moveTo = pos.Y - (step / 2);
        if (moveTo > screenHeight) {
            pos.Y = screenHeight;
            pos.X += (moveTo - screenHeight) * 2;
            return 3;
        }
        else if (moveTo < 0) {
            pos.Y = 0;
            pos.X += moveTo * -2;
            return 1;
        }
        else {
            pos.Y = moveTo;
        }
        return 0;
    }

    else if (OnRight) {
        moveTo = pos.Y + (step / 2);
        if (moveTo > screenHeight) {
            pos.Y = screenHeight;
            pos.X -= (moveTo - screenHeight) * 2;
            return 4;
        }
        else if (moveTo < 0) {
            pos.Y = 0;
            pos.X += moveTo * 2;
            return 2;
        }
        else {
            pos.Y = moveTo;
        }
        return 0;
    }

}


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

