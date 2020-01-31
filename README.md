# Groundstation
Software for a CanSat groundstation, developed for usage on the CanSat Júnior 2020 competition.
If you have received this in an already compiled format, you may find the source code in https://github.com/abread/groundstation

The project is divided in two parts: the desktop app and the transceiver code.
The transceiver code is meant to be running on an Arduino connected to your computer and transceiver.
It exposes all data received by the RFM69HCW transceiver and RSSI information.
The desktop app connects to the Arduino, saves all received data in a file and shows it coming in realtime, as well as the current RSSI reported by the transceiver.

## Setting up the Arduino
This is tested to work with the Arduino IDE.

You will need the following libraries:
- [RFM69_LowPowerLab](https://github.com/LowPowerLab/RFM69)
- [SPIFlash_LowPowerLab](https://github.com/LowPowerLab/RFM69) - this one isn't strictly used, but the RFM library depends on it for some reason

Before uploading, make sure you have set all required radio parameters (`#define`d in the beginning of the file) like frequency.

The code is built for an Arduino Nano Every board and the RFM69HCW transceiver, but it should be easy to adapt to other hardware.

## Building the App
Tested to work on Windows (MSVC, 64bit) and Linux (64bit).

Precompiled versions are available for Windows, macOS and Linux. The Linux version does not include the shared libraries required to run the program, if you have a different Qt version installed, it may not work. Building from source is recommended.

If you're not comfortable building from source, [Qt Creator](https://www.qt.io/development-tools) is recommended.

On Linux systems, make sure you have qmake, and QSerialPort installed.
```
cd groundstation-app
qmake groundstation-app.pro
make
```
Ta-dah! The program is built on groundstation-app/groundstation-app.

On macOS the same process can be used after installing some version of Qt. By default an app bundle will be generated (that does not include Qt libraries).

On Windows, if you are using the MSVC compiler, replace the `make` call with `nmake` or Qt's `pom`. Make sure your environment is properly set up.
