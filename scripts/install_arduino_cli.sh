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
# Installs the latest arduino-cli tool in /tmp/bin/arduino-cli and the latest ARM GNU toolchain
# next to the Arduino-installed one.
# Also changes the Nano 33 BLE configuration to use this toolchain.


set -e

if [[ $# -eq 0 ]]; then
	DRY_RUN=0
	arduino_cli=/tmp/bin/arduino-cli
else
	while getopts "d" opt; do
		case $opt in
		  d)
			DRY_RUN=1
			arduino_cli=arduino-cli
			;;
		  *)
			echo "Invalid option"
			exit 1
			;;
		esac
	done
fi

cd /tmp

if [[ $DRY_RUN -eq 0 ]]; then	
	rm -rf arduino-cli*
	echo "Downloading Arduino CLI"
	curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
	echo "Downloading ARM GNU Toolchain v14.2"
	curl -L --output "14_2.tar.xz" "https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-aarch64-arm-none-eabi.tar.xz"
		
	echo "Installing arduino:mbed_nano core"
	$arduino_cli core update-index
	$arduino_cli lib update-index
	$arduino_cli core install arduino:mbed_nano
	
	echo "Unpacking toolchain"
	tar -xJf "14_2.tar.xz"
	mv "arm-gnu-toolchain-14.2.rel1-aarch64-arm-none-eabi" "${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/14_2"
	rm -f *.tar.xz *.gz
fi

CORE_VER=$(${arduino_cli} core list | grep mbed_nano | tr -s [:space:] | cut -d ' ' -f2)
cd "${HOME}/.arduino15/packages/arduino/hardware/mbed_nano/${CORE_VER}/"

files=$(grep -lr "7-2017q4")

if [[ $DRY_RUN -eq 0 ]]; then
	for f in $files; do
	    sed "s/\(nano33ble.*\)7-2017q4\(.*\)/\114_2\2/" -i $f
	done
fi

