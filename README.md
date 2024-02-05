# TensorFlow Lite Micro library for Arduino

This is a fork of the [Arduino TensorFlow Lite Micro](https://github.com/tensorflow/tflite-micro-arduino-examples) library. It has been updated to use the latest code from the [TensorFlow Lite Micro](github.com/tensorflow/tflite-micro) repository, with fully functional example sketches and the additional capability of creating a static library for use with the Arduino environment.

## Table of contents
<!--ts-->
* [Table of contents](#table-of-contents)
* [Build Status](#build-status)
* [How to Install](#how-to-install)
  * [Toolchain](#toolchain)
  * [GitHub](#github)
  * [Checking your Installation](#checking-your-installation)
* [Compatibility](#compatibility)
* [Create a static library](#create-a-static-library)
* [Pull new code from the TfLM repo](#pull-the-latest-code-from-tflite-micro)
* [License](#license)
* [Contributing](#contributing)
<!--te-->

## Build Status

Build Type          |     Status    |
---------------     | ------------- |
Arduino CLI on Linux  | [![Arduino](https://github.com/tensorflow/tflite-micro-arduino-examples/actions/workflows/ci.yml/badge.svg?event=schedule)](https://github.com/tensorflow/tflite-micro-arduino-examples/actions/workflows/ci.yml)
Sync from tflite-micro  | [![Sync from tflite-micro](https://github.com/tensorflow/tflite-micro-arduino-examples/actions/workflows/sync.yml/badge.svg)](https://github.com/tensorflow/tflite-micro-arduino-examples/actions/workflows/sync.yml)

## How to Install

### Toolchain

To compile this library, it is necessary to install a newer toolchain than what Arduino currently provides. I am using the Arm GNU Embedded Toolchain Version [10.3-2021.10](https://developer.arm.com/downloads/-/gnu-rm). The new one includes several intrinsics required by the library.
It should be installed next to the Arduino provided toolchain, which for Linux is in `~/.arduino15/packages/arduino/tools/arm-none-eabi-gcc` and Windows `C:\Users\<username>\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc`.

Then we need to tell Arduino to use the newer toolchain.  Open the file `~/.arduino15/packages/arduino/hardware/mbed_nano/4.0.10/boards.txt` and change the line that says
```
nano33ble.build.compiler_path={runtime.tools.arm-none-eabi-gcc-7-2017q4.path}/bin/
```
to
```
 nano33ble.build.compiler_path={runtime.tools.arm-none-eabi-gcc-<folder name>.path}/bin/
```
where *\<folder name\>* is the name of the folder you installed the toolchain in.

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

**(Only tested in Linux and tailored to the Arduino Nano 33 BLE Sense rev.1)**

Compiling sketches that use this library takes a lot of time. We can speed up this process by avoiding recompiling the library every time. To create a static library (.a) which is linked to the sketch during compile time, you can do the following steps.
From the src/ directory of this library, run
```
make -f tensorflow/lite/micro/tools/make/Makefile TARGET=cortex_m_generic TARGET_ARCH=cortex-m4+sfp OPTIMIZED_KERNEL_DIR=cmsis_nn microlite
```
It will create the static library in `/tmp/gen/cortex_m_generic_cortex-m4+sfp_default/lib/libtensorflow-microlite.a` which must be copied to `<path to Arduino lib>/src/cortex-m4/fpv4-sp-d16-softfp`.

That's it!

## Pull the latest code from TfLite Micro

There are several scripts under the scripts/ folder to provide various utilities, including to create a new version of this library with the latest code from the TfLite Micro repository.

## License

This code is made available under the Apache 2 license.

## Contributing

Forks of this library are welcome and encouraged. If you have bug reports or
fixes to contribute, the source of this code is at [https:://github.com/tensorflow/tflite-micro](github.com/tensorflow/tflite-micro)
and all issues and pull requests should be directed there.

The code here is created through an automatic project generation process
and may differ from
that source of truth, since it's cross-platform and needs to be modified to
work within the Arduino IDE.
