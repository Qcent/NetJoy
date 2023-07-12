#include <iostream>
#include <string>
#include "cxxopts.hpp"

struct Arguments {
    std::string host = "";
    int port = 5000;
    bool latency = true;
    bool select = true;
    int mode = 1;
    int fps = 30;
};

Arguments parse_arguments(int argc, char* argv[]) {
    Arguments args;

    cxxopts::Options options("JoySender++", "Send joystick data over tcp/ip");
    options.allow_unrecognised_options();
    options.add_options()
        ("n,host", "IP address of host/server", cxxopts::value<std::string>()->default_value(""))
        ("p,port", "Port to run on", cxxopts::value<int>()->default_value("5000"))
        ("f,fps", "How many times to attempt to communicate with server per second", cxxopts::value<int>()->default_value("30"))
        ("m,mode", "Operational Mode: 1: Xbox360 Emulation, 2: DS4 Emulation", cxxopts::value<int>()->default_value("1"))
        ("l,latency", "Show latency output", cxxopts::value<bool>()->implicit_value("true"))
        ("a,auto", "Auto select first joystick", cxxopts::value<bool>()->implicit_value("true"))
        ("h,help", "Display this help message");

    options.parse_positional("host");

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

    args.host = result["host"].as<std::string>();
    args.port = result["port"].as<int>();
    args.latency = result["latency"].as<bool>();
    args.select = result["auto"].as<bool>();
    args.mode = result["mode"].as<int>();
    args.fps = result["fps"].as<int>();

    return args;
}