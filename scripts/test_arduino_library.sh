#!/usr/bin/env bash
# Copyright 2022 The TensorFlow Authors. All Rights Reserved.
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
# Tests an individual Arduino library. Because libraries need to be installed
# globally, this can cause problems with previously-installed modules, so we
# recommend that you only run this within a VM.

set -e

ARDUINO_LIBRARIES_DIR=${HOME}/Arduino/libraries
LIBRARY_DIR=${ARDUINO_LIBRARIES_DIR}/${1}
TOOLCHAIN_ROOT="${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/14_2/bin/"
ARDUINO_CLI_TOOL=/tmp/bin/arduino-cli
# Necessary due to bug in arduino-cli that allows it to build files in pwd
TEMP_BUILD_DIR=/tmp/tflite-arduino-build

rm -rf ${TEMP_BUILD_DIR}
mkdir -p ${TEMP_BUILD_DIR}

cd ${LIBRARY_DIR}/src
make -f tensorflow/lite/micro/tools/make/Makefile clean
make -j4 -f tensorflow/lite/micro/tools/make/Makefile TARGET_TOOLCHAIN_ROOT=${TOOLCHAIN_ROOT} BUILD_TYPE=release_with_logs microlite
mkdir -p ${LIBRARY_DIR}/src/cortex-m4/fpv4-sp-d16-softfp
mv "/tmp/gen/cortex_m_generic_cortex-m4+sfp_release_with_logs/lib/libtensorflow-microlite.a" "${LIBRARY_DIR}/src/cortex-m4/fpv4-sp-d16-softfp/libtflm.a"

# Installs all dependencies for Arduino
InstallLibraryDependencies () {
  # Required by magic_wand
  ${ARDUINO_CLI_TOOL} lib install Arduino_LSM9DS1
  ${ARDUINO_CLI_TOOL} lib install ArduinoBLE
}

InstallLibraryDependencies

# in case file glob expansion is empty
shopt -s nullglob

ino_files=()
ino_files+=(${LIBRARY_DIR}/examples/*/*.ino)
ino_files+=(${LIBRARY_DIR}/src/peripherals/examples/*/*.ino)
ino_files+=(${LIBRARY_DIR}/src/peripherals/tests/test_*/*.ino)


for f in "${ino_files[@]}"; do
  echo "Compiling $(basename ${f} .ino)"
  ${ARDUINO_CLI_TOOL} compile --build-path ${TEMP_BUILD_DIR} -b arduino:mbed_nano:nano33ble "$f"
done

rm -rf ${TEMP_BUILD_DIR}
