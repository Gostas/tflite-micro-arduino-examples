#!/usr/bin/env bash
# Copyright 2021 The TensorFlow Authors. All Rights Reserved.
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
# Creates the project file distributions for the TensorFlow Lite Micro test and
# example targets aimed at embedded platforms.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="${SCRIPT_DIR}/.."
cd "${ROOT_DIR}"
echo BASH_SOURCE=${BASH_SOURCE[0]} SCRIPT_DIR=${SCRIPT_DIR} ROOT_DIR=${ROOT_DIR}

source "${SCRIPT_DIR}"/helper_functions.sh

TEMP_DIR=$(mktemp -d)
cd "${TEMP_DIR}"

git clone --depth 1 --single-branch "https://github.com/tensorflow/tflite-micro.git"
cd tflite-micro

readable_run make -f tensorflow/lite/micro/tools/make/Makefile clean_downloads

BASE_DIR=/tmp/tflm_tree
OUTPUT_DIR=/tmp/tflm_arduino
TARGET=cortex_m_generic
OPTIMIZED_KERNEL_DIR=cmsis_nn
TARGET_ARCH=cortex-m4+sfp

readable_run python3 tensorflow/lite/micro/tools/project_generation/create_tflm_tree.py \
  -e hello_world -e micro_speech -e person_detection \
  --makefile_options="TARGET=${TARGET} OPTIMIZED_KERNEL_DIR=${OPTIMIZED_KERNEL_DIR} TARGET_ARCH=${TARGET_ARCH}" "${BASE_DIR}"


# Need the mbed core
readable_run "${SCRIPT_DIR}"/install_arduino_cli.sh
# Replace downloaded CMSIS lib with the one Arduino uses
cp -a "${HOME}/.arduino15/packages/arduino/hardware/mbed_nano/4.1.1/cores/arduino/mbed/cmsis/CMSIS_5/CMSIS/" ${BASE_DIR}/third_party/cmsis
mv ${BASE_DIR}/third_party/cmsis/CMSIS/TARGET_CORTEX_M ${BASE_DIR}/third_party/cmsis/CMSIS/Core
rm -rf ${BASE_DIR}/third_party/cmsis//CMSIS/TARGET* ${BASE_DIR}/third_party/cmsis/CMSIS/RTOS2

cd "${ROOT_DIR}"

echo create_tflm_tree.py done
#read

readable_run python3 "${SCRIPT_DIR}"/create_tflm_arduino.py \
  --output_dir="${OUTPUT_DIR}" \
  --base_dir="${BASE_DIR}"

mv ${OUTPUT_DIR}/signal ${OUTPUT_DIR}/src

echo create_tflm_arduino.py done
#read


readable_run "${SCRIPT_DIR}"/test_arduino_library.sh \
  "${OUTPUT_DIR}"

rm -rf "${BASE_DIR}"
rm -rf "${OUTPUT_DIR}"
rm -rf "${TEMP_DIR}"
