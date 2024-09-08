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

wchar_t rsideFooter[] = L"\t/   \t\t:}-     ";

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

