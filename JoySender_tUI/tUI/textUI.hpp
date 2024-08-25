/*

Copyright (c) 2023 Dave Quinn <qcent@yahoo.com>

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
#include <iostream>
#include <Windows.h>

#include "ColorSchemes.h"

#define CONSOLE_WIDTH       72
#define CONSOLE_HEIGHT      24

#define MAX_SCREEN_BUTTONS  32
#define MAX_SCREEN_INPUTS   8

// mouse button status flags
#define MOUSE_OUT       0x0
#define MOUSE_HOVERED   0x1
#define MOUSE_DOWN      0x2
#define MOUSE_UP        0x4
#define ACTIVE_INPUT    0x8

// mouse button settings flags
#define UNCLICKABLE     0x1
#define UNHOVERABLE     0x2
//#define SELF_MANAGED    0x8

// mouse button default colors
#define DEFAULT_TEXT        WHITE | BLACK AS_BG
#define HOVERED_BUTTON      BRIGHT_BLUE | BLACK AS_BG
#define SELECTED_BUTTON     BLACK | YELLOW AS_BG
#define ACTIVE_BUTTON       BLACK | RED AS_BG

// text box alignment values
#define ALIGN_LEFT      0
#define ALIGN_CENTER    1
#define ALIGN_RIGHT     2

// Function sets the console cursor position
void setCursorPosition(int x, int y) {
    COORD coord;
    coord.X = static_cast<SHORT>(x);
    coord.Y = static_cast<SHORT>(y);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void setCursorPosition(COORD coord) {
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

// Function to set console text color
void setTextColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// Function determines where to make line break
int findNextLineBreak(const wchar_t* text, int start, int width) {
    if (text[start+width] == L'\0')
        return start + width;
    int lastSpace=0;
    int i;
    for (i = 0; i < width+1 ; i++) {
        if (text[i + start] == L'\0' || text[i + start] == L'\n')
            return i + start;
        if (i && text[i+start] == L' ') {
            lastSpace = i+start;
        }
    }
    return lastSpace;
}

// Packages 4 colors representing: defaultColor, highlightColor, selectedColor, activeColor
struct tUIColorPkg {
    WORD col1;
    WORD col2;
    WORD col3;
    WORD col4;

    tUIColorPkg(WORD c1, WORD c2, WORD c3, WORD c4 ) : col1(c1), col2(c2), col3(c3), col4(c4) {
    }

    bool operator==(const tUIColorPkg& other) const {
        return (this->col1 == other.col1) &&
            (this->col2 == other.col2) &&
            (this->col3 == other.col3) &&
            (this->col4 == other.col4);
    }
};

// Contains the base attributes shared by all textUI objects 
class tUIObj {
public:
    void SetText(const wchar_t* text) {
        _text = text;
        // Find length of null terminated text
        _length = 0;
        while (text[_length] != L'\0') {
            _length++;
        }
    }

    void SetPosition(int x, int y, int width, int lines, byte align) {
        _pos.X = x;
        _pos.Y = y;
        _width = width;
        _lines = lines;
        _alignment = align;
    }
    void SetPosition(int x, int y) {
        _pos.X = x;
        _pos.Y = y;
    }
    void SetPosition(COORD pos) {
        _pos = pos;
    }

    void SetWidth(int width) {
        _width = width;
    }

    void SetColors(tUIColorPkg colors) {
        _defaultColor = colors.col1;
        _highlightColor = colors.col2;
        _selectColor = colors.col3;
        _activeColor = colors.col4;
    }
    tUIColorPkg GetColors() {
        return tUIColorPkg(_defaultColor, _highlightColor, _selectColor, _activeColor);
    }

    tUIObj() {
        _pos.X = 0;
        _pos.Y = 0;
        _width = 0;
        _text = nullptr;
        _alignment = ALIGN_LEFT;
        _length = 0;
        _lines = 0;
        _defaultColor = DEFAULT_TEXT;
        _highlightColor = HOVERED_BUTTON;
        _selectColor = SELECTED_BUTTON;
        _activeColor = ACTIVE_BUTTON;        
    }

protected:
    COORD _pos;
    byte _alignment;
    int _width;
    int _lines;
    int _length;
    const wchar_t* _text;
    WORD _defaultColor;
    WORD _highlightColor;
    WORD _selectColor;
    WORD _activeColor;  // not really used...
};

// A Text Area to complement the textUI class
class textBox : public tUIObj {
public:
    textBox(int x, int y, int width, int lines, byte align, const wchar_t* text, WORD color){
        _pos.X = x;
        _pos.Y = y;
        _width = width;
        _lines = lines;
        _alignment = align;
        _text = text;
        _color = color;

        SetText(text);
    }
    textBox() {
        _text = L"<empty>";
        _length = 7;
        _width = 80;
        _lines = 1;
        _color = DEFAULT_TEXT;
    }
    ~textBox() {
        //delete[] _text;
    }

    void SetColor(WORD col) {
        _color = col;
    }

    void Draw() {
        setTextColor(_color);
        switch (_alignment) {
        case ALIGN_LEFT:
            _drawLeftAutoBreak();
            break;
        case ALIGN_CENTER:
            _drawCenterAutoBreak();
            break;
        case ALIGN_RIGHT:
            _drawRightAutoBreak();
            break;
        }
    }

    void Clear(int color = -1) {
        if (color > -1)
            setTextColor(color);
        else
            setTextColor(_color); // for background
        switch (_alignment) {
        case ALIGN_LEFT:
            _clearLeftAutoBreak();
            break;
        case ALIGN_CENTER:
            _clearCenterAutoBreak();
            break;
        case ALIGN_RIGHT:
            _clearRightAutoBreak();
            break;
        }
    }

    void Draw_noColor() {
        switch (_alignment) {
        case 0:
            _drawLeftAutoBreak();
            break;
        case 1:
            _drawCenterAutoBreak();
            break;
        case 2:
            _drawRightAutoBreak();
            break;
        }
    }

protected:
    WORD _color;

    void _drawLeftAutoBreak() {
        int line = 0;
        int charsRemaining = _length;
        int i = 1;
        int nextBreak = findNextLineBreak(_text, i - 1, _width);

        setCursorPosition(_pos);
        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << _text[i - 1];
            }
            else if (i < _length) {
                line++;
                if (line >= _lines && _lines) {
                    return;
                }
                setCursorPosition(_pos.X, _pos.Y + line);
                nextBreak = findNextLineBreak(_text, i - 1, _width);
            }
        }
    }

    void _drawCenterAutoBreak() {
        int line = 0;
        int startPoint;
        int i = 1;
        int nextBreak;
        int charsRemaining = _length;

        auto setStartPoint = [&]() {
            if (charsRemaining > _width) {
                nextBreak = findNextLineBreak(_text, i - 1, _width);
                startPoint = _pos.X - (nextBreak - i) / 2;
            }
            else {
                nextBreak = _length;
                startPoint = _pos.X - charsRemaining / 2;
            }
        };
        setStartPoint();
        setCursorPosition(startPoint, _pos.Y);

        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << _text[i - 1];
            }
            else if (i < _length) {
                line++;
                if (line > _lines && _lines) {
                    return;
                }
                setStartPoint();
                setCursorPosition(startPoint, _pos.Y + line);
            }
        }
    }

    void _drawRightAutoBreak() {
        int line = 0;
        int startPoint;
        int i = 0;
        int nextBreak;
        int charsRemaining = _length;
        auto setStartPoint = [&]() {
            if (charsRemaining > _width) {
                nextBreak = findNextLineBreak(_text, i, _width);
                startPoint = _pos.X - (nextBreak - 1 - i);
            }
            else {
                nextBreak = _length;
                startPoint = _pos.X - charsRemaining + 1;
            }
        };

        setStartPoint();
        setCursorPosition(startPoint, _pos.Y);
        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << _text[i - 1];
            }
            else if (i < _length) {
                line++;
                if (line > _lines && _lines) {
                    return;
                }
                setStartPoint();
                setCursorPosition(startPoint, _pos.Y + line);

            }
        }
    }

    void _clearLeftAutoBreak() {
        int line = 0;
        int charsRemaining = _length;
        int i = 1;
        int nextBreak = findNextLineBreak(_text, i - 1, _width);

        setCursorPosition(_pos);
        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << ' ';
            }
            else if (i < _length) {
                line++;
                if (line >= _lines && _lines) {
                    return;
                }
                setCursorPosition(_pos.X, _pos.Y + line);
                nextBreak = findNextLineBreak(_text, i - 1, _width);
            }
        }
    }

    void _clearCenterAutoBreak() {
        int line = 0;
        int startPoint;
        int i = 1;
        int nextBreak;
        int charsRemaining = _length;

        auto setStartPoint = [&]() {
            if (charsRemaining > _width) {
                nextBreak = findNextLineBreak(_text, i - 1, _width);
                startPoint = _pos.X - (nextBreak - i) / 2;
            }
            else {
                nextBreak = _length;
                startPoint = _pos.X - charsRemaining / 2;
            }
        };
        setStartPoint();
        setCursorPosition(startPoint, _pos.Y);

        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << ' ';
            }
            else if (i < _length) {
                line++;
                if (line > _lines && _lines) {
                    return;
                }
                setStartPoint();
                setCursorPosition(startPoint, _pos.Y + line);
            }
        }
    }

    void _clearRightAutoBreak() {
        int line = 0;
        int startPoint;
        int i = 0;
        int nextBreak;
        int charsRemaining = _length;
        auto setStartPoint = [&]() {
            if (charsRemaining > _width) {
                nextBreak = findNextLineBreak(_text, i, _width);
                startPoint = _pos.X - (nextBreak - 1 - i);
            }
            else {
                nextBreak = _length;
                startPoint = _pos.X - charsRemaining + 1;
            }
        };

        setStartPoint();
        setCursorPosition(startPoint, _pos.Y);
        for (i = 1; i - 1 < _length; i++) {
            charsRemaining--;
            if (i - 1 < nextBreak) {
                std::wcout << ' ';
            }
            else if (i < _length) {
                line++;
                if (line > _lines && _lines) {
                    return;
                }
                setStartPoint();
                setCursorPosition(startPoint, _pos.Y + line);

            }
        }
    }
};

// Extends textBox for text input to a heap allocated wchar_t*
class textInput : public textBox {
public:
    const int INPUT_MAX_LENGTH = 96;
    textInput() {
        _maxLength = INPUT_MAX_LENGTH;
        _currentPosition = 0;
        _input = new wchar_t[_maxLength + 1];
        _text = _input;
        _lines = 1;
        _status = 0;
    }
    textInput(int x, int y, int width, int maxLen, byte align) : _maxLength(maxLen) {
        _pos.X = x;
        _pos.Y = y;
        _alignment = align;
        _width = width;
        _currentPosition = 0;
        _length = 0;
        _input = new wchar_t[_maxLength + 1];
        _text = _input;
        _lines = 1;
        _status = 0;
    }
    textInput(int x, int y, int width, int maxLen, byte align, WORD color) : _maxLength(maxLen) {
        _pos.X = x;
        _pos.Y = y;
        _alignment = align;
        _width = width;
        _color = color;
        _currentPosition = 0;
        _length = 0;
        _input = new wchar_t[_maxLength + 1];
        _text = _input;
        _lines = 1;
        _status = 0;
    }
    ~textInput() {
        delete[] _input;
    }

    void back() {
        if (!_currentPosition) return;

        if (_currentPosition == _length)
            _input[_currentPosition-1] = L'\0';
        else
            swprintf(_input + _currentPosition, min(_length - (_currentPosition + 1), _maxLength - (_currentPosition + 1)) + 1, L"%s", _input + _currentPosition + 1);
                
        --_length;
        --_currentPosition;
    }

    void del() {
        if (_currentPosition == _length) return;

        swprintf(_input + _currentPosition + 1, min(_length - (_currentPosition + 2), _maxLength - (_currentPosition + 2)) + 1, L"%s", _input + _currentPosition + 2);
        --_length;
    }

    void insert(wchar_t chR) {
        if (_currentPosition == _maxLength) return;

        swprintf(_input + _currentPosition, (_length - _currentPosition) + 1 + 1, L"%lc%s", chR, _input + _currentPosition);
        ++_currentPosition;
        ++_length;
    }

    void overwrite(wchar_t chR) {
        if (_currentPosition == _maxLength) return;

        _input[_currentPosition] = chR;
        _input[_currentPosition+1] = L'\0';
        ++_currentPosition;
        if (_currentPosition > _length)
            ++_length;
    }

    void cursorRight(unsigned int move = 1) {
        if (_currentPosition == _length) return;
        _currentPosition += min(move, _length - _currentPosition);
    }

    void cursorLeft(unsigned int move = 1) {
        if (!_currentPosition) return;
        _currentPosition -= min(move, _currentPosition);
    }

    void cursorToBegin() {
        _currentPosition = 0;
    }

    void cursorToEnd() {
        _currentPosition = _length;
    }

    void clearBuffer() {
        for (int i = 0; i < _maxLength; ++i) {
            _input[i] = L'\0';
        }
        _length = 0;
        _currentPosition = 0;
    }

    void setWidth(int width) {
        _width = width;
    }

    wchar_t* getText() {
        return _input;
    }

    int getCursorPosition() {
        return _currentPosition;
    }

    int getLength() {
        return _length;
    }

    void CheckMouseHover(COORD mousePos) {
        if (mousePos.X >= _pos.X && mousePos.X <= _pos.X + _width - 1 && mousePos.Y >= _pos.Y && mousePos.Y <= _pos.Y + _lines - 1) {
            if (!(_status & MOUSE_HOVERED)) {
                _status = MOUSE_HOVERED;
                _color = _highlightColor;
                Draw();
            }
        }
        else if (_status & MOUSE_HOVERED) {
            _status = MOUSE_OUT;
            _color = _defaultColor;
            Draw();
        }
    }

    void CheckMouseClick(BYTE mouseState) {
        if (_status & MOUSE_HOVERED) {

            if (mouseState == MOUSE_DOWN && !(_status & MOUSE_DOWN)) {
                _status |= MOUSE_DOWN;
                _status &= ~MOUSE_UP;
                _color = _selectColor;
                Draw();
            }
            else if (_status & MOUSE_DOWN) { // mouse up
                _status &= ~MOUSE_DOWN;
                _status |= MOUSE_UP;
                _status |= ACTIVE_INPUT;
                _color = _activeColor;
                Draw();
            }
        }
    }

    BYTE Status() {
        return _status;
    }

private:
    unsigned int _maxLength;
    unsigned int _currentPosition;
    wchar_t* _input;
    BYTE _status;
};

// On screen click-able button and star of the textUI class
class mouseButton : public tUIObj {
public:
    typedef void (*CallbackFunction)(mouseButton&);

    mouseButton(int x, int y, int w, const wchar_t* text, WORD dcol, WORD hcol, WORD scol, WORD acol) {
        _pos.X = x;
        _pos.Y = y;
        _width = w;
        _text = text;
        
        _defaultColor = dcol;
        _highlightColor = hcol;
        _selectColor = scol;
        _activeColor = acol;

        _status = MOUSE_OUT;
        _Settings = 0;
        _length = 0;
        // Find length of null terminated text
        while (text[_length] != L'\0') {
            _length++;
        }

        _lines = max(1,_length / _width);
        _callback = nullptr;
        _partner = nullptr;
        _id = 0;
    }
    mouseButton(int x, int y, int w, const wchar_t* text) {
        _pos.X = x;
        _pos.Y = y;
        _width = w;
        _text = text;

        _status = MOUSE_OUT;
        _Settings = 0;

        // Find length of null terminated text
        while (text[_length] != L'\0') {
            _length++;
        }
        _lines = max(1, _length / _width);

        _callback = nullptr;
        _partner = nullptr;
        _id = 0;
    }
    mouseButton(int x, int y, int w, const wchar_t* text, byte set) {
        _pos.X = x;
        _pos.Y = y;
        _width = w;
        _text = text;
        _Settings = set;
        _status = MOUSE_OUT;

        // Find length of null terminated text
        while (text[_length] != L'\0') {
            _length++;
        }
        _lines = max(1, _length / _width);

        _callback = nullptr;
        _partner = nullptr;
        _id = 0;
    }
    mouseButton() {
        _status = MOUSE_OUT;
        _Settings = 0;
        _callback = nullptr;
        _partner = nullptr;
        _id = 0;
    }

    ~mouseButton() {
    }

    void SetId(int id) {
        _id = id;
    }

    void SetStatus(BYTE status) {
        _status = status;
    }

    BYTE Status() {
        return _status;
    }

    void Update() {
        if(_callback != nullptr) _callback(*this);

        if (_status & MOUSE_DOWN)       // clicked
            setTextColor(_selectColor);
        //else if (_status & ACTIVE_INPUT)    // active
            //setTextColor(_activeColor);
        else if (_status & MOUSE_HOVERED)    // hovered
            setTextColor(_highlightColor);
        else                            // default
            setTextColor(_defaultColor);
        
        int line = 0;
        setCursorPosition(_pos);
        for (int i = 1; i - 1 < _length; i++) {
            if (_text[i - 1] == '\t') {
                // advance cursor
                setCursorPosition(_pos.X + i%_width, _pos.Y + line);
            }
            else
                std::wcout << _text[i - 1];
            if (i % _width == 0 && i < _length){
                line++;
                setCursorPosition(_pos.X, _pos.Y + line);
            }
        }
        //setTextColor(_defaultColor); // ?? get rid of this ??
    }

    void Clear(int color = -1) {
        if(color>-1)
            setTextColor(color);
        else
            setTextColor(_defaultColor);

        int line = 0;
        setCursorPosition(_pos);
        for (int i = 1; i - 1 < _length; i++) {
            if (_text[i - 1] == '\t') {
                // advance cursor
                setCursorPosition(_pos.X + i % _width, _pos.Y + line);
            }
            else
                std::wcout << ' ';
            if (i % _width == 0 && i < _length) {
                line++;
                setCursorPosition(_pos.X, _pos.Y + line);
            }
        }
    }

    void Draw(int color = -1) {
        if (color > -1)
            setTextColor(color);
        else
            setTextColor(_defaultColor);

        int line = 0;
        setCursorPosition(_pos);
        for (int i = 1; i - 1 < _length; i++) {
            if (_text[i - 1] == '\t') {
                // advance cursor
                setCursorPosition(_pos.X + i % _width, _pos.Y + line);
            }
            else
                std::wcout << _text[i - 1];
            if (i % _width == 0 && i < _length) {
                line++;
                setCursorPosition(_pos.X, _pos.Y + line);
            }
        }
    }

    void CheckMouseHover(COORD mousePos) {
        if (mousePos.X >= _pos.X && mousePos.X <= _pos.X + _width-1 && mousePos.Y >= _pos.Y && mousePos.Y <= _pos.Y + _lines - 1) {
            if (!(_status & MOUSE_HOVERED)) {
                _status = MOUSE_HOVERED;
                if (_Settings & UNHOVERABLE) return;
                Update();
            }
        }
        else if (_status & MOUSE_HOVERED) {
            _status = MOUSE_OUT;
            Update();
        }
    }

    void CheckMouseClick(BYTE mouseState) {      
        if (_Settings & UNCLICKABLE) return;
        if (_status & MOUSE_HOVERED) {

            if (mouseState == MOUSE_DOWN && !(_status & MOUSE_DOWN)) {
                _status |= MOUSE_DOWN; 
                _status &= ~MOUSE_UP;
                Update();
            }
            else if (_status & MOUSE_DOWN) { // mouse up
                _status &= ~MOUSE_DOWN; 
                _status |= MOUSE_UP; 
                Update();
            }
        }
    }

    void setCallback(CallbackFunction callback) {
        _callback = callback;
    }

    void Couple(mouseButton* partner) {
        _partner = partner;
    }

    auto GetPartner() {
        return _partner;
    }

    int GetId() {
        return _id;
    }

    void SetAllColors(tUIColorPkg colors) {
        if (_partner != nullptr)
            _partner->SetColors(colors);

        SetColors(colors);
    }

    void SetDefaultColor(WORD col) {
        if (_partner != nullptr)
            _partner->SetDefaultColor(col);

        _defaultColor = col;
    }

    void SetHighlightColor(WORD col) {
        if (_partner != nullptr)
            _partner->SetHighlightColor(col);

        _highlightColor = col;
    }

    void SetSelectColor(WORD col) {
        if (_partner != nullptr)
            _partner->SetSelectColor(col);

        _selectColor = col;
    }

    void SetActiveColor(WORD col) {
        if (_partner != nullptr)
            _partner->SetActiveColor(col);

        _activeColor = col;
    }

    WORD getHighlightColor() {
        return _highlightColor;
    }

    WORD getSelectColor() {
        return _selectColor;
    }

    WORD getDefaultColor() {
        return _defaultColor;
    }

    WORD getActiveColor() {
        return _activeColor;
    }

    const wchar_t* getTextPtr() {
        return _text;
    }

    WORD getCurrentColor() {
        if (_status & MOUSE_DOWN)       // clicked
            return _selectColor;
        //else if (_status & ACTIVE_INPUT)    // active
            //return _activeColor;
        else if (_status & MOUSE_HOVERED)    // hovered
            return _highlightColor;
        else                            // default
            return _defaultColor;
    }

private:
    int _id;
    byte _Settings;
    BYTE _status;
    CallbackFunction _callback;
    mouseButton* _partner;
};

// Holds pointers to a text 'backdrop' and all buttons and textInputs,
//  currently on screen, for easy update handling
class textUI {
public:
    textUI() {
        _backdrop = L" ( Uninitialized ) ";
        ClearButtons();
        ClearInputs();
        _backdropColor = DEFAULT_TEXT;
    }
    ~textUI() {
    }

    void SetBackdrop(const wchar_t* text) {
        _backdrop = text;
    }

    void SetBackdropColor(WORD col) {
        _backdropColor = col;
    }
    
    WORD GetBackdropColor() {
       return _backdropColor;
    }

    void DrawBackdrop() {
        setTextColor(_backdropColor);
        setCursorPosition(0, 0);
        std::wcout << _backdrop;
    }

    void DrawButtons() {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->Update();
        }
    }

    void DrawInputs() {
        for (int i = 0; i < _inputsCount; i++) {
            _textInputs[i]->Draw();
        }
    }
    
    void ReDraw() {
        DrawBackdrop();
        DrawButtons();
        DrawInputs();
    }

    void ClearButtons() {
        for (int i = 0; i < MAX_SCREEN_BUTTONS; ++i) {
            _buttons[i] = nullptr;
        }
        _buttonsCount = 0;
    }

    void AddButton(mouseButton* ptr) {
        if (_buttonsCount == MAX_SCREEN_BUTTONS) return;
        _buttons[_buttonsCount] = ptr;
        _buttonsCount++;
    }

    void ClearInputs() {
        for (int i = 0; i < MAX_SCREEN_INPUTS; ++i) {
            _textInputs[i] = nullptr;
        }
        _inputsCount = 0;
    }

    void AddInput(textInput* ptr) {
        if (_inputsCount == MAX_SCREEN_BUTTONS) return;
        _textInputs[_inputsCount] = ptr;
        _inputsCount++;
    }

    void CheckMouseClick(BYTE mouseState) {
        for (int i = 0; i < max(_buttonsCount, _inputsCount); i++) {
            if (i < _buttonsCount)
                _buttons[i]->CheckMouseClick(mouseState);
            if (i < _inputsCount)
                _textInputs[i]->CheckMouseClick(mouseState);
        }
    }

    void CheckMouseHover(COORD mousePos) {
        for (int i = 0; i < max(_buttonsCount,_inputsCount); i++) {
            if(i < _buttonsCount)
                _buttons[i]->CheckMouseHover(mousePos);
            if (i < _inputsCount)
                _textInputs[i]->CheckMouseHover(mousePos);
        }
    }

    
    void SetButtonsHighlightColor(WORD col) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->SetHighlightColor(col);
        }
    }
    
    void SetButtonsSelectColor(WORD col) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->SetSelectColor(col);
        }
    }

    void SetButtonsActiveColor(WORD col) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->SetActiveColor(col);
        }
    }

    void SetButtonsDefaultColor(WORD col) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->SetDefaultColor(col);
        }
    }

    void SetButtonsColors(tUIColorPkg colors) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->SetAllColors(colors);
        }
    }

    void SetInputsColors(tUIColorPkg colors) {
        for (int i = 0; i < _inputsCount; i++) {
            _textInputs[i]->SetColors(colors);
        }
    }

    void SetButtonsCallback(mouseButton::CallbackFunction cbFunc) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->setCallback(cbFunc);
        }
    }
    
    void ClearButtonsCallback(mouseButton::CallbackFunction cbFunc) {
        for (int i = 0; i < _buttonsCount; i++) {
            _buttons[i]->setCallback(nullptr);
        }
    }

    /*
    WORD getSafeColors() {
        // A list of replacement colors to select from
        std::vector<WORD> colorList = {		// new color selection is as follows:
            inverseFGColor(_backdropColor >> 4),	// inverse of BG color
            WHITE,							// white
            BLACK							// black
        };

        WORD safe_col = _backdropColor;
        if (sameFG_BG(_backdropColor, _backdropColor)) {
            safe_col = inverseFGColor(_backdropColor);
        }

        if (CheckContrastMismatch(safe_col, _backdropColor)) {
            safe_col = findSafeFGColor(_backdropColor, colorList, colorList.begin()) | (safe_col BG_ONLY);
        }

        return safe_col;
    }
    */

private:
    const wchar_t* _backdrop;
    WORD _backdropColor;
    mouseButton* _buttons[MAX_SCREEN_BUTTONS];
    int _buttonsCount;
    textInput* _textInputs[MAX_SCREEN_INPUTS];
    int _inputsCount;
};
