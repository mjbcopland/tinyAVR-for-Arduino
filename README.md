# tinyAVR for Arduino

Atmel tinyAVR microcontroller support (ATtiny45/84/85) for the [Arduino IDE (1.6.4+)](https://www.arduino.cc/en/Main/Software) using the [Micronucleus](https://github.com/micronucleus/micronucleus) bootloader.

**NOTE: tinyAVR for Arduino has been tested on Windows but Linux and OSX are still under development. If using Linux or OSX, you will have to compile the Micronucleus++ uploader yourself as it is not currently included in the download for those operating systems.**

## Installation

In the Arduino IDE's Preferences dialog, add the following URL to the Additional Boards Manager URLs:

    https://github.com/mjbcopland/tinyAVR-for-Arduino/releases/download/1.0/package_mjbcopland_tinyavr_index.json

Then install the "Atmel tinyAVR" package from the Boards Manager.

After installing, ATtiny45/84/85 options will appear in the Tools→Boards menu.

### Platform-specific installation

#### Windows

When first connecting a Micronucleus device, you will likely be asked to install the Micronucleus drivers. If you already have these drivers installed, the installer should update them. These drivers can be installed manually using [the Zadig installer](https://github.com/micronucleus/micronucleus/tree/master/windows_driver_installer).

#### Linux

By default, most Linux distributions will not allow communication with unknown USB devices. To fix this, copy [`49-micronucleus.rules`](https://github.com/mjbcopland/tinyAVR-for-Arduino/releases/download/1.0/49-micronucleus.rules) to `/etc/udev/rules.d/`.

## Usage

Burning the Micronucleus bootloader is done in the same manner as other Arduino bootloaders. See [here](https://www.arduino.cc/en/Tutorial/ArduinoToBreadboard) for details.

After burning the Micronucleus bootloader, sketches can be uploaded via USB like any other Arduino sketch. You will, however, need to manually connect or otherwise reset the microcontroller when prompted.
