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

#include <iostream>
#include <string>
#include "cxxopts.hpp"

struct Arguments {
    int port = 5000;
    bool latency = false;
};

Arguments parse_arguments(int argc, char* argv[]) {
    Arguments args;

    cxxopts::Options options("JoyReceiver++", "Receive and emulate joystick data over tcp/ip");
    options.allow_unrecognised_options();
    options.add_options()
        ("p,port", "Port to run on", cxxopts::value<int>()->default_value("5000"))
        ("l,latency", "Show latency output", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Display this help message");

    options.parse_positional("port");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.unmatched().size()) {
        std::cout << "Illegal argument" << std::endl;
        std::cout << options.help() << std::endl;
        exit(0);
    }

    args.port = result["port"].as<int>();
    args.latency = result["latency"].as<bool>();
 
    return args;
}