# JoyReceiver++
JoyReceiver++ is a console application that receives and emulates joystick data over TCP/IP. It allows you to seamlessly control a joystick connected to a remote machine, providing an immersive gaming experience.

## Usage

To use JoyReceiver++, follow these steps:
- Run the JoyReceiver++ executable without any command-line parameters.
- JoyReceiver++ will start listening for incoming joystick data on the default port (5000) and emulate the received input using the ViGEM driver.

### With Command-Line Parameters

JoyReceiver++ provides command-line parameters for advanced settings and customization. Here are the available options:

    -p, --port <PORT>: Sets the port number to run JoyReceiver++ on for communication with the client/sender.
    -l, --latency: Enables the display of latency output during communication.
    -h, --help: Displays the help message with information on how to use JoyReceiver++ and its available options.

By default, JoyReceiver++ uses port 5000 for communication. If you wish to use a different port, specify it using the -p/--port option.

```
JoyReceiver++ [OPTIONS]
```

**Example Usage:**

To run JoySender++ with default settings, simply execute the following command:

```
JoyReceiver++
```

To specify the port to listen on and output latency, use the `-p/--port` and '-l/--latency' options:

```
JoyReceiver++ -p 8080 -l
```

**Note:** You can safely quit the program at any time by pressing `Ctrl + C` in the console window.

## License
MIT Licence

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


## Contact
If you are interested in contributing or just want to chat email me at qcent@yahoo.com
