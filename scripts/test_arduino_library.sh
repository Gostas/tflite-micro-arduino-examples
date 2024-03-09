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

ARDUINO_HOME_DIR=${HOME}/Arduino
ARDUINO_LIBRARIES_DIR=${ARDUINO_HOME_DIR}/libraries
ARDUINO_CLI_TOOL=/tmp/arduino-cli
# Necessary due to bug in arduino-cli that allows it to build files in pwd
TEMP_BUILD_DIR=/tmp/tflite-arduino-build

LIBRARY_DIR=${1}
LIBRARY_NAME=$(basename ${1})

rm -rf ${TEMP_BUILD_DIR}
rm -rf ${ARDUINO_LIBRARIES_DIR}

mkdir -p ${ARDUINO_LIBRARIES_DIR}
mkdir -p ${TEMP_BUILD_DIR}

cp -a ${LIBRARY_DIR} "${ARDUINO_LIBRARIES_DIR}"

# build static library -- comment out as it's not working currently
# make -f ${LIBRARY_DIR}/src/tensorflow/lite/micro/tools/make/Makefile TENSORFLOW_ROOT="${LIBRARY_DIR}/src/" microlite
# mkdir ${LIBRARY_DIR}/src/cortex-m4
# mkdir ${LIBRARY_DIR}/src/cortex-m4/fpv4-sp-d16-softfp
# mv "/tmp/gen/cortex_m_generic_cortex-m4+sfp_default/lib/libtensorflow-microlite.a" "${LIBRARY_DIR}/src/cortex-m4/fpv4-sp-d16-softfp"

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
  ${ARDUINO_CLI_TOOL} compile --library ${LIBRARY_DIR} --build-cache-path ${TEMP_BUILD_DIR} \
    --build-path ${TEMP_BUILD_DIR} -b arduino:mbed_nano:nano33ble "$f"
  #echo Done. Press enter to continue
  #read
done

rm -rf ${ARDUINO_LIBRARIES_DIR}
rm -rf ${TEMP_BUILD_DIR}
rm -rf /tmp/gen/
