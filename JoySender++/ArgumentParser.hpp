/*

Copyright (c) 2025 Dave Quinn <qcent@yahoo.com>

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
    std::string host = "";
    int port = 5000;
#ifndef NetJoyTUI
    bool latency = true;
#endif
    bool udp = true;
    bool tcp = false;
    bool select = true;
    int mode = 1;
    int fps = 0;

};

Arguments parse_arguments(int argc, char* argv[]) {
    Arguments args;

#ifdef NetJoyTUI 
    cxxopts::Options options("JoySender_tUI", "JoySender++ (tUI) : Send joystick data over tcp/udp");
#else
    cxxopts::Options options("JoySender++", "JoySender++ : Send joystick data over tcp/udp");
#endif
    options.allow_unrecognised_options();
    options.add_options()
        ("n,host", "IP address of host/server", cxxopts::value<std::string>()->default_value(""))
        ("p,port", "Port to run on", cxxopts::value<int>()->default_value("5000"))
        ("f,fps", "How many times to attempt to communicate with server per second", cxxopts::value<int>()->default_value("0"))
        ("m,mode", "Operational Mode: 1: Xbox360 Emulation, 2: DS4 Emulation", cxxopts::value<int>()->default_value("1"))
        ("t,tcp", "Use TCP protocol", cxxopts::value<bool>()->implicit_value("true"))
        ("u,udp", "Use UDP protocol", cxxopts::value<bool>()->implicit_value("true"))
#ifndef NetJoyTUI 
        ("l,latency", "Show latency output", cxxopts::value<bool>()->implicit_value("true"))
#endif        
        ("a,auto", "Auto select first joystick", cxxopts::value<bool>()->implicit_value("true"))
        ("h,help", "Display this help message");

    options.parse_positional("host");
    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (result.unmatched().size()) {
        std::cout << "Illegal argument" << std::endl;
        std::cout << options.help() << std::endl;
        exit(0);
    }

    args.host = result["host"].as<std::string>();
    args.port = result["port"].as<int>();
#ifndef NetJoyTUI 
    args.latency = result["latency"].as<bool>();
#endif
    args.select = result["auto"].as<bool>();
    args.mode = result["mode"].as<int>();
    args.fps = result["fps"].as<int>();
    args.udp = result["udp"].as<bool>();
    args.tcp = result["tcp"].as<bool>();

    args.udp = args.tcp ? false : true;
    if (args.fps == 0) args.fps = (args.udp ?  80 : 60); // default 80fps for udp, 60/tcp

    return args;
}