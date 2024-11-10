# TensorFlow Lite Micro library for Arduino

This is a fork of the [Arduino TensorFlow Lite Micro](https://github.com/tensorflow/tflite-micro-arduino-examples) library. It has been updated to use the latest code from the [TensorFlow Lite Micro](https://github.com/tensorflow/tflite-micro) repository, with fully functional example sketches and the additional capability of creating a static library for use with the Arduino environment.

## Table of contents
<!--ts-->
* [Table of contents](#table-of-contents)
* [Build Status](#build-status)
* [How to Install](#how-to-install)
  * [GitHub](#github)
  * [Checking your Installation](#checking-your-installation)
* [Compatibility](#compatibility)
* [Create a static library](#create-a-static-library)
  * [Toolchain](#toolchain)
  * [Compilation](#compilation)
* [Debugging](#debugging)
  * [VS Code](#vs-code)
* [Pull new code from the TFLM repo](#pull-the-latest-code-from-tflite-micro)
* [License](#license)
* [Contributing](#contributing)
<!--te-->

## Build Status

Build Type          |     Status    |
---------------     | ------------- |
Arduino CLI on Linux  | [![Arduino](https://github.com/Gostas/tflite-micro-arduino-examples/actions/workflows/ci.yml/badge.svg?event=schedule)](https://github.com/Gostas/tflite-micro-arduino-examples/actions/workflows/ci.yml)
Sync from tflite-micro  | [![Sync from tflite-micro](https://github.com/Gostas/tflite-micro-arduino-examples/actions/workflows/sync.yml/badge.svg)](https://github.com/Gostas/tflite-micro-arduino-examples/actions/workflows/sync.yml)

## How to Install
### GitHub

The officially supported TensorFlow Lite Micro library for Arduino resides
in the [tflite-micro-arduino-examples](https://github.com/tensorflow/tflite-micro-arduino-examples)
GitHub repository.
To install the in-development version of this library, you can use the
latest version directly from the GitHub repository. This requires you to clone the
repo into the folder that holds libraries for the Arduino IDE. The location for
this folder varies by operating system, but typically it's in
`~/Arduino/libraries` on Linux, `~/Documents/Arduino/libraries/` on MacOS, and
`My Documents\Arduino\Libraries` on Windows.

Once you're in that folder in the terminal, you can then grab the code using the
git command line tool:

```
git clone https://github.com/Gostas/tflite-micro-arduino-examples Arduino_TensorFlowLite
```

To update your clone of the repository to the latest code, use the following terminal commands:
```
cd Arduino_TensorFlowLite
git pull
```

### Checking your Installation

Once the library has been installed, you should then start the Arduino IDE.
You will now see an `Arduino_TensorFlowLite`
entry in the `File -> Examples` menu of the Arduino IDE. This submenu contains a list
of sample projects you can try out.

![Hello World](docs/hello_world_screenshot.png)

## Compatibility

This library is designed for the `Arduino Nano 33 BLE Sense` board. The framework
code for running machine learning models should be compatible with most Arm Cortex
M-based boards, such as the `Raspberry Pi Pico`, but the code to access peripherals
like microphones, cameras, and accelerometers is specific to the `Nano 33 BLE Sense`.

## Create a static library

**(Only tested in Linux and with the Arduino Nano 33 BLE Sense rev.1)**

Compiling sketches that use this library takes a lot of time. We can speed up this process by avoiding recompiling the library every time. To create a static library (.a) which is linked to the sketch during compile time, you can do the following steps.

### Toolchain
To compile this library, it is necessary to install a newer toolchain than what Arduino currently provides. I am using the [Arm GNU Embedded Toolchain Version 13.3.Rel1](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). It includes several intrinsics required by the library.

We need to tell Arduino to use the newer toolchain. In `~/.arduino15/packages/arduino/hardware/mbed_nano/4.2.1/` open the files `boards.txt` and `platforms.txt` and replace any occurrences of
```
{runtime.tools.arm-none-eabi-gcc-7-2017q4.path}/bin/
```
with
```
<install dir>/bin/
```
We also need [arduino-cli](https://arduino.github.io/arduino-cli/1.1/) with the latest mbed_nano core installed.
```
arduino-cli core update-index
arduino-cli core install arduino:mbed_nano
```
### Compilation
From the src/ directory of this library, run
```
make -j4 -f tensorflow/lite/micro/tools/make/Makefile TARGET_TOOLCHAIN_ROOT=<install dir>/bin microlite
```
It will create the static library in `/tmp/gen/cortex_m_generic_cortex-m4+sfp_default/lib/libtensorflow-microlite.a` which must be copied to `<path to this repository>/src/cortex-m4/fpv4-sp-d16-softfp`.

## Debugging
I am using the Raspberry Pi Debug Probe and its 3-pin debug to male jumper cable. I soldered three wires connected to the SWDIO, SWCLK and GND pins on the back of the Arduino. These are respectively connected to RX/SD, TX/SC and GND on the probe.

The arduino-cli has proved very useful in debugging and the integration with the rpi debug probe is seamless. My general workflow is:

0. Compile the TFLM library for debug
```
make -j4 -f tensorflow/lite/micro/tools/make/Makefile BUILD_TYPE=debug microlite
```
1. Compile the sketch and export the binaries to the sketch folder
```
arduino-cli compile -b arduino:mbed_nano:nano33ble -e --optimize-for-debug <dir>
```
2. Upload the binary to the board using either USB or the probe (disconnect the probe first to use the USB)
```
arduino-cli upload -b arduino:mbed_nano:nano33ble --input-dir <dir>/build/arduino.mbed_nano.nano33ble/ -v -p /dev/ttyACM0

arduino-cli upload -b arduino:mbed_nano:nano33ble --input-dir <dir>/build/arduino.mbed_nano.nano33ble/ -v -P cmsis-dap
```
3. Launch OpenOCD and connect gdb
```
arduino-cli debug -b arduino:mbed_nano:nano33ble -P cmsis-dap . -v
```
### VS Code
For convenience, I integrated this workflow with VS Code. Under the .vscode directory I created the files tasks.json and launch.json. The tasks.json defines tasks for building and uploading a sketch to the Arduino as well as starting an OpenOCD server to which gdb can connect to. They can be ran by clicking Terminal > Run Task...

The launch.json defines a Debug configuration that starts a debugging session with VS Code. You need to have a running OpenOCD server first. To start the debugging session, simply choose "(gdb) Launch" in the "Run and Debug" side panel.

## Pull the latest code from TFLite Micro
There are several scripts under the scripts/ folder to provide various utilities, including to create a new version of this library with the latest code from the TFLite Micro repository.

## License

This code is made available under the Apache 2 license.

## Contributing

Forks of this library are welcome and encouraged. If you have bug reports or
fixes to contribute, the source of this code is at [https://github.com/tensorflow/tflite-micro](http://github.com/tensorflow/tflite-micro)
and all issues and pull requests should be directed there.

The code here is created through an automatic project generation process
and may differ from
that source of truth, since it's cross-platform and needs to be modified to
work within the Arduino IDE.
