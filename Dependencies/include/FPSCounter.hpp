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
#include <chrono>

class FPSCounter {
private:
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    //std::chrono::duration<double> elapsed_time;
    int frame_count;

public:
    FPSCounter() : elapsed_time(0.0) {
        start_time = std::chrono::steady_clock::now();
        frame_count = 0;
    }

    int increment_frame_count() {
        return frame_count++;
    }

    double get_fps() {
        elapsed_time = std::chrono::steady_clock::now() - start_time;
        if (elapsed_time.count() > 0) {
            double fps = frame_count / elapsed_time.count();
            return fps;
        }
        return 0;
    }

    double get_elapsed_time() {
        std::chrono::duration<double> elapsed_time = std::chrono::steady_clock::now() - start_time;
        return elapsed_time.count();
    }

    int get_frame_count() {
        return frame_count;
    }

    void reset_timer() {
        start_time = std::chrono::steady_clock::now();
    }

    void reset() {
        start_time = std::chrono::steady_clock::now();
        frame_count = 0;
    }
};

/*
    // Example usage:
int main(){
    FPSCounter fps_counter;

    // Call do_fps_counting in the main loop
    auto do_fps_counting = [&fps_counter](int report_frequency = 30) {
        if (fps_counter.increment_frame_count() >= report_frequency) {
            double fps = fps_counter.get_fps();
            fps_counter.reset();
            return "FPS: " + std::to_string(fps);
        }
        return std::string();
    };

    while (true) {
        std::string fps = do_fps_counting();  // Will return an output every 30 loops by default
        if (!fps.empty()) {
            std::cout << "Achieving " << fps << " loops per second" << std::endl;
        }
    }

    return 0;
}
*/