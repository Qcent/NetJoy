# JoySender++
JoySender++ is a console application that enables you to send joystick data over TCP/IP to a host/server. It simplifies the process of remote joystick control, allowing you to enjoy gaming experiences on a different machine within a local network or even over the internet.

## Usage

To use JoySender++, follow these steps:

    Run the JoySender++ executable without any command-line parameters.
    JoySender++ will guide you through the initial joystick mapping process. Follow the on-screen instructions to assign buttons and axes to your joystick.
    Once mapped, JoySender++ will start transmitting joystick data to the specified host/server.

Command-Line Parameters

```
JoySender++ [OPTIONS]
```

**Options:**

- `-n, --host <IP>`: Specifies the IP address of the host/server. Provide the IP address where you want to send the joystick data. By default, it is set to an empty string.

- `-p, --port <PORT>`: Sets the port number to run JoySender++ on. Specify the port number for communication with the host/server. The default port is set to `5000`.

- `-f, --fps <FPS>`: Defines the communication frequency with the server in attempts per second. Set the desired frequency for communicating with the server. The default is `30` attempts per second.

- `-m, --mode <MODE>`: Sets the operational mode for JoySender++. Use `1` for Xbox 360 emulation mode or `2` for DS4 emulation mode. Choose the desired mode based on your requirements. The default mode is Xbox 360 emulation.

- `-l, --latency`: Enables the display of latency output. Use this option if you want to see the latency information during communication. By default, this option is disabled.

- `-a, --auto`: Automatically selects the first joystick recognized by the system. If you have multiple joysticks connected, this option will automatically choose the first one. By default, this option is disabled.

- `-h, --help`: Displays the help message with information on how to use JoySender++ and its available options.

**Initial Joystick Mapping:**

The first time you run JoySender++ or when a new joystick is selected, the program will guide you through a mapping process. You will be prompted to interact with your joystick and assign the appropriate buttons and axes. The mapping results will be saved to a file for future use.

**Hotkey Buttons:**

Once you establish a connection with the host/server, JoySender++ provides several hotkey buttons for convenient control:

- `Shift + M`: Pressing Shift and M together will initiate the joystick remapping process. This allows you to reconfigure the joystick mapping on the fly.

- `Shift + R`: Pressing Shift and R together will restart the JoySender++ program, resetting the connection and settings.
 Additionaly, while holding the R key, pressing 1 or 2 will allow you to switch between different modes while in a connection. This is useful for changing the operational mode on the fly.

- `Shift + Q`: Pressing Shift and Q together will quit the JoySender++ program.

**Note:** You can also quit the program at any time by pressing `Ctrl + C` in the console window.

**Example Usage:**

To run JoySender++ with default settings, simply execute the following command:

```
JoySender++
```

To specify the IP address and port of the host/server, use the `-n/--host` and `-p/--port` options:

```
JoySender++ -n 192.168.1.100 -p 8080
```

For DS4 emulation mode and latency output enabled, use the following command:

```
JoySender++ -m 2 -l
```

These are just a few examples, and you can customize the usage based on your specific needs.

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
