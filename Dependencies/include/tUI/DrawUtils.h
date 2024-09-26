

struct lineMask {
    byte x1 = 255;
    byte x2 = 255;
};
typedef std::vector<std::vector<lineMask>> LineMaskGrid;

LineMaskGrid mergeMasks(const LineMaskGrid m1, const LineMaskGrid m2) {
    // get new mask origin
    short startX = std::min(m1[0][0].x1, m2[0][0].x1);  // x
    short startY = std::min(m1[0][0].x2, m2[0][0].x2);  // y
    byte lower = (m1[0][0].x2 < m2[0][0].x2) ? 1 : 2;
    if ((m1[0][0].x2 == m2[0][0].x2)) lower = 0;
    // calc rows in output mask
    size_t totalRows = std::max(m1.size() - 1 + (m1[0][0].x2 - startY),
        m2.size() - 1 + (m2[0][0].x2 - startY)) + 1;
    LineMaskGrid outMask(totalRows);

    // insert the new origin into outMask
    lineMask tmp = { startX, startY };
    outMask[0] = { {tmp} };

    // adjust m1, x and y values, reletive to outmask origin
    for (size_t i = 1; i < m1.size(); ++i) {
        short rowOffset = (m1[0][0].x2 - startY) + i;  // y
        for (size_t j = 0; j < m1[i].size(); ++j) {
            lineMask adjustedM1 = m1[i][j];
            adjustedM1.x1 += (m1[0][0].x1 - startX);  // x1
            adjustedM1.x2 += (m1[0][0].x1 - startX);  // x2

            if (rowOffset < outMask.size()) { // this shouldn't happen
                outMask[rowOffset].push_back(adjustedM1);
            }
            else {
                outMask.push_back({ adjustedM1 });
            }
        }
    }

    // Adjust m2's x1 and x2 values relative to startX and adjust rows relative to startY
    for (size_t i = 1; i < m2.size(); ++i) {
        short rowOffset = (m2[0][0].x2 - startY) + i;  // Offset by starting y
        for (size_t j = 0; j < m2[i].size(); ++j) {
            lineMask adjustedM2 = m2[i][j];
            adjustedM2.x1 += (m2[0][0].x1 - startX);  // Adjust x position
            adjustedM2.x2 += (m2[0][0].x1 - startX);  // Adjust x position

            // Ensure the row exists in outMask
            if (rowOffset < outMask.size()) {
                outMask[rowOffset].push_back(adjustedM2);
            }
            else {
                outMask.push_back({ adjustedM2 });
            }
        }
    }

    return outMask;
}

bool pointIsMasked(COORD checkPoint, const LineMaskGrid& maskList) {
    short maskHeight = maskList.size() - 1;
    short maskX = maskList[0][0].x1;
    short maskY = maskList[0][0].x2;

    // Check if the checkPoint.Y falls within the range of the mask's height
    if (checkPoint.Y < maskY || checkPoint.Y >= maskY + maskHeight) {
        return false;
    }

    // mask-line index based on the Y coordinate
    int index = (checkPoint.Y - maskY) + 1;

    for (const auto& mask : maskList[index]) {

        // Check if the checkPoint.X is within the x1 and x2 range for the current line mask
        if (checkPoint.X >= (maskX + mask.x1) && checkPoint.X < (maskX + mask.x2)) {
            return true;  // Point is inside the mask
        }
    }

    return false;  // Point is outside all masks
}

const LineMaskGrid clubMASK = { {{8,7}},
    {{11,15}},{{8,18}}, {{6,20}}, {{6,20}}, {{7,19}}, {{3,23}}, {{1,25}},
    {{0,26}}, {{0,26}}, {{2,24}}, {{4,9},{12,14},{17,22}}, { {12,14} }, { {11,15} }, { {8,18} } };



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



void draw_block_line(COORD start, COORD end, byte block, bool useMask = false, const LineMaskGrid& mask = {}) {
    // Calculate width and height
    int w = (end.X - start.X);
    int h = (end.Y - start.Y);

    bool flipped = (abs(h) > abs(w)); // Flip if height is greater than width
    double m = flipped ? double(w) / h : double(h) / w;

    wchar_t ch = getWcharFromByte(block);

    int length = flipped ? abs(h) : abs(w);
    short _x, _y;

    for (int i = 0; i <= length; ++i) {
        if (flipped) {
            _y = start.Y + i * (h > 0 ? 1 : -1); // Increment or decrement y
            _x = start.X + std::round(m * (_y - start.Y)); // Calculate x based on y
        }
        else {
            _x = start.X + i * (w > 0 ? 1 : -1); // Increment or decrement x
            _y = start.Y + std::round(m * (_x - start.X)); // Calculate y based on x
        }

        COORD position = { _x, _y };
        if (!useMask || !pointIsMasked(position, mask)) {
            setCursorPosition(position);
            std::wcout << ch;
        }

    }
}

void draw_block_triangle(COORD a, COORD b, COORD c, byte block, bool useMask = false, const LineMaskGrid& mask = {}) {
    wchar_t ch = getWcharFromByte(block);

    // Draw the triangle edges
    draw_block_line(a, b, block, useMask, mask);
    draw_block_line(b, c, block, useMask, mask);
    draw_block_line(c, a, block, useMask, mask);

    // Fill the triangle using scanline algorithm
    // Sort the vertices by their Y-coordinate (vertical scanline)
    COORD vertices[3] = { c, a, b };
    std::sort(vertices, vertices + 3, [](COORD v1, COORD v2) { return v1.Y < v2.Y; });

    COORD v1 = vertices[0];
    COORD v2 = vertices[1];
    COORD v3 = vertices[2];

    // Function to interpolate X between two points based on Y
    auto interpolate_x = [](COORD p1, COORD p2, int y) -> SHORT {
        if (p1.Y == p2.Y) return p1.X;  // Avoid division by zero
        return SHORT(p1.X + (p2.X - p1.X) * (y - p1.Y) / (p2.Y - p1.Y));
        };

    //Fill between v1 and v2, and v1 and v3 (upper part)
    for (short y = v1.Y; y <= v2.Y; ++y) {
        short x1 = interpolate_x(v1, v3, y);
        short x2 = interpolate_x(v1, v2, y);
        if (x1 > x2) std::swap(x1, x2);
        for (short x = x1; x <= x2; ++x) {
            if (!useMask || !pointIsMasked({ x, y }, mask)) {
                setCursorPosition({ x, y });
                std::wcout << ch;
            }
        }
    }
    //Fill between v2 and v3 (lower part)
    for (short y = v2.Y; y <= v3.Y; ++y) {
        short x1 = interpolate_x(v1, v3, y);
        short x2 = interpolate_x(v2, v3, y);
        if (x1 > x2) std::swap(x1, x2);
        for (short x = x1; x <= x2; ++x) {
            if (!useMask || !pointIsMasked({ x, y }, mask)) {
                setCursorPosition({ x, y });
                std::wcout << ch;
            }
        }
    }
}



void printSubsetOfBuffer(const COORD pos, const RECT& inputRect, const RECT& outputRect, const wchar_t* source, bool drawWhiteSpace = true) {
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
        bufferIndex = (innerStartY + y) * inputWidth + innerStartX;

        for (x = 0; x < outputWidth; ++x) {
            wchar_t ch = source[bufferIndex++];

            // Handle whitespace
            if (!drawWhiteSpace && ch == L' ') {
                setCursorPosition(pos.X + x + 1, pos.Y + y);
                continue;
            }
            std::wcout << ch;


        }
    }
}

void printX_Ntimes(const wchar_t X, const int N) {
    for (int i = 0; i < N; i++) {
        std::wcout << X;
    }
}

void drawPatternedArea(const RECT rect, byte* ptrn = nullptr) {
    int w = (rect.right - rect.left), h = (rect.bottom - rect.top);
    if (h <= 0 || w <= 0)
        return;
    int dir;
    std::vector<wchar_t> blocks(10);
    if (ptrn == nullptr) {
        srand(static_cast<unsigned int>(time(0)));
        std::vector<wchar_t> rndBlocks = { L'▓', L'▒', L'█', L'▓', L'░', L'▒' };
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::default_random_engine rng(seed);
        std::shuffle(rndBlocks.begin(), rndBlocks.end(), rng);
        blocks = { L'▒', L'▒', L' ', L'▓', L'▒', L'▒', L'▒', rndBlocks[3], L'░', L'▒' };
        std::shuffle(blocks.begin(), blocks.end(), rng);
        dir = generateRandomInt(-1, 0);
    }
    else {
        // shade ptrns
        if (ptrn[0] == 255) {
            wchar_t ch;
            switch (ptrn[1]) {
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
            for (int i = 0; i < h; ++i) {
                setCursorPosition(rect.left, rect.top + i);
                printX_Ntimes(ch, w);
            }
            return;
        }

        // diags
        for (int i = 0; i < 10; i++)
            blocks[i] = getWcharFromByte(ptrn[i]);
        dir = (ptrn[10]) ? 1 : -1;
    }

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