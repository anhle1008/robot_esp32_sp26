# CMSC730 SP26 Robot Competition

## Setup
1. Copy `platformio.ini` and `src/main.cpp` into the PlatformIO's project root directory to replace the original ones.
2. Upload the project onto the ESP32 microcontroller.
3. Go to the settings on your phone to connect WiFi `Team 9 Robot` and enter password `12345678`.
4. Open a browser `to http://192.168.4.1` to control the stepper motor.

## Key components of a PlatformIO project
1. `platformio.ini`: The core configuration file that defines build settings, libraries, and board environments.
2. `src/`: Folder for main source code files (e.g., main.cpp).
3. `lib/`: Directory for private, project-specific libraries.
4. `include/`: Directory for header files.
5. `.pio/`: Hidden folder where PlatformIO stores built binaries and library dependencies

## Components of the robot
1. Autodesk Fusion360: wheel
2. Laser Cutting: body + connectors
3. Control Code: PlatformIO on VSCode


