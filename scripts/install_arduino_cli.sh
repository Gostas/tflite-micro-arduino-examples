#!/usr/bin/env bash
# Copyright 2019 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
#
# Installs the latest arduino-cli tool in /tmp/arduino-cli and the 10.3-2021.10 ARM GNU toolchain
# next to the Arduino-installed one.
# Also changes the Nano 33 BLE configuration to use this toolchain.


set -e

cd /tmp

rm -rf arduino-cli*
echo "Downloading Arduino CLI v0.35.2"
curl -L -O "https://github.com/arduino/arduino-cli/releases/download/v0.35.2/arduino-cli_0.35.2_Linux_64bit.tar.gz"
echo "Downloading ARM GNU Toolchain v10.3-2021.10"
curl -L --output "10_3-2021_10.tar.bz2" "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2?rev=78196d3461ba4c9089a67b5f33edf82a&hash=5631ACEF1F8F237389F14B41566964EC"

echo "Unpacking Arduino CLI"
tar xzf arduino-cli_0.35.2_Linux_64bit.tar.gz

# To use with MacOS, replace the previous two lines with:
# curl -L -O "https://github.com/arduino/arduino-cli/releases/download/0.35.2/arduino-cli_0.35.2_MacOS_64bit.tar.gz"
# tar xzf arduino-cli_0.35.2_MacOS_64bit.tar.gz

echo "Installing arduino:mbed_nano:nano33ble core"
/tmp/arduino-cli core update-index
/tmp/arduino-cli core install arduino:mbed_nano:nano33ble

echo "Unpacking toolchain"
tar -xf "10_3-2021_10.tar.bz2"
mv "gcc-arm-none-eabi-10.3-2021.10" "${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/10_3-2021_10"
rm -f *.bz2 *.gz

cd "${HOME}/.arduino15/packages/arduino/hardware/mbed_nano/4.1.1/"

sed "s/\(nano33ble.*\)7-2017q4\(.*\)/\110_3-2021_10\2/" -i boards.txt