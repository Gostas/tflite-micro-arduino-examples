# Copyright 2023 The TensorFlow Authors. All Rights Reserved.
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

# Generic Makefile target for ARM Cortex M builds.
# For more info see: tensorflow/lite/micro/cortex_m_generic/README.md

FLOAT := soft
GCC_TARGET_ARCH := $(TARGET_ARCH)

# Explicitly set this to true to include the kissfft symbols.
INCLUDE_MICRO_SPEECH := true

ifeq ($(TARGET_ARCH), cortex-m0)
  CORE=M0
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M0

else ifeq ($(TARGET_ARCH), cortex-m0plus)
  CORE=M0plus
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M0plus

else ifeq ($(TARGET_ARCH), cortex-m3)
  CORE=M3
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M3

else ifeq ($(TARGET_ARCH), cortex-m33)
  CORE=M33
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M33
  CMSIS_ARM_FEATURES := _DSP_FP
  FLOAT=hard

else ifeq ($(TARGET_ARCH), cortex-m33+nodsp)
  CORE=M33
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M33.no_dsp.no_fp

else ifeq ($(TARGET_ARCH), cortex-m4)
  CORE=M4
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M4.no_fp
  GCC_TARGET_ARCH := cortex-m4+nofp

else ifeq ($(TARGET_ARCH), cortex-m4+fp)
  CORE=M4
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M4
  CMSIS_ARM_FEATURES := _FP
  FLOAT=hard
  GCC_TARGET_ARCH := cortex-m4

else ifeq ($(TARGET_ARCH), cortex-m4+sfp)
  CORE=M4
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M4
  CMSIS_ARM_FEATURES := _FP
  FLOAT=softfp
  GCC_TARGET_ARCH := cortex-m4

else ifeq ($(TARGET_ARCH), cortex-m55)
  CORE=M55
  ARM_LDFLAGS := -Wl,--cpu=8.1-M.Main.mve.fp
  FLOAT=hard

else ifeq ($(TARGET_ARCH), cortex-m55+nodsp+nofp)
  CORE=M55
  ARM_LDFLAGS := -Wl,--cpu=8.1-M.Main.mve.no_dsp.no_fp

else ifeq ($(TARGET_ARCH), cortex-m55+nofp)
  CORE=M55
  ARM_LDFLAGS := -Wl,--cpu=8.1-M.Main.mve.no_fp

else ifeq ($(TARGET_ARCH), cortex-m7)
  CORE=M7
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M7.no_fp
  GCC_TARGET_ARCH := cortex-m7+nofp

else ifeq ($(TARGET_ARCH), cortex-m7+fp)
  CORE=M7
  ARM_LDFLAGS := -Wl,--cpu=Cortex-M7
  FLOAT=hard
  GCC_TARGET_ARCH := cortex-m7
  CMSIS_ARM_FEATURES := _DP

else ifeq ($(TARGET_ARCH), cortex-m85)
  CORE=M85
  ARM_LDFLAGS := -Wl,--cpu=8.1-M.Main.mve.fp
  FLOAT=hard
  # GCC does not yet support cortex-m85 option hence go with cortex-m55 for now.
  GCC_TARGET_ARCH := cortex-m55

else ifeq ($(TARGET_ARCH), project_generation)
  # No flags needed here as project_generation does not build anything.
else
  $(error "TARGET_ARCH=$(TARGET_ARCH) is not supported")
endif

# Dependency to CMSIS-Device for DWT/PMU counters.
ARM_CPU := "ARMC$(CORE)"
CMSIS_PATH := $(TENSORFLOW_ROOT)third_party/cmsis
INCLUDES += \
  -I$(CMSIS_PATH)/CMSIS/Core/Include
# -I$(CMSIS_PATH)/Device/ARM/$(ARM_CPU)/Include

ifneq ($(filter cortex-m55%,$(TARGET_ARCH)),)
  # soft-abi=soft disables MVE - use softfp instead for M55.
  ifeq ($(FLOAT),soft)
    FLOAT=softfp
  endif
endif

# Toolchain specfic flags
ifeq ($(TOOLCHAIN), armclang)
  CXX_TOOL  := armclang
  CC_TOOL   := armclang
  AR_TOOL   := armar
  LD        := armlink

  FLAGS_ARMC = \
    --target=arm-arm-none-eabi \
    -mcpu=$(TARGET_ARCH)

  CXXFLAGS += $(FLAGS_ARMC)
  CCFLAGS += $(FLAGS_ARMC)
  LDFLAGS += $(ARM_LDFLAGS)

  # Arm Compiler will not link the Math library (see below), therefore we're filtering it out.
  # See Fatal error: L6450U: Cannot find library m:
  # "Arm Compiler is designed to run in a bare metal environment,
  # and automatically includes implementations of these functions,
  # and so no such flag is necessary."
  # https://developer.arm.com/documentation/100891/0611/troubleshooting/general-troubleshooting-advice
  MICROLITE_LIBS := $(filter-out -lm,$(MICROLITE_LIBS))

else ifeq ($(TOOLCHAIN), gcc)
  TARGET_TOOLCHAIN_PREFIX := arm-none-eabi-
  TARGET_TOOLCHAIN_ROOT := ${HOME}/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/10_3-2021_10/bin/

  FLAGS_GCC = -mcpu=$(GCC_TARGET_ARCH)
  ifeq ($(GCC_TARGET_ARCH), cortex-m4)
    FLAGS_GCC += -mfpu=fpv4-sp-d16
  else ifeq ($(GCC_TARGET_ARCH), cortex-m7)
    FLAGS_GCC += -mfpu=fpv4-sp-d16
  else
    FLAGS_GCC += -mfpu=auto
  endif
  CXXFLAGS += $(FLAGS_GCC)
  CCFLAGS += $(FLAGS_GCC)

else
  $(error "TOOLCHAIN=$(TOOLCHAIN) is not supported.")
endif

PLATFORM_FLAGS = \
  -DTF_LITE_MCU_DEBUG_LOG \
  -mthumb \
  -mfloat-abi=$(FLOAT) \
  -funsigned-char \
  -mlittle-endian \
  -Wno-type-limits \
  -Wno-unused-private-field \
  -fomit-frame-pointer \
  -MD \
  -DCPU_$(CORE)=1

ifeq ($(ARDUINO_BOARD), NANO33BLE)
  # Link to Arduino core, the Wire library and precompiled libraries for Mbed OS
  EXTERNAL_DIR += \
    -I$(ARDUINO_BOARD_DIR)/cores/arduino/api/deprecated \
    -I$(ARDUINO_BOARD_DIR)/cores/arduino/api/deprecated-avr-comp \
    -I$(ARDUINO_BOARD_DIR)/cores/arduino \
    -I$(ARDUINO_BOARD_DIR)/variants/ARDUINO_NANO33BLE \
    -I$(ARDUINO_BOARD_DIR)/libraries/Wire \
    -iprefix$(ARDUINO_BOARD_DIR)/cores/arduino @$(ARDUINO_BOARD_DIR)/variants/ARDUINO_NANO33BLE/includes.txt

  # Add Arduino defines
  PLATFORM_DEFINES += @$(ARDUINO_BOARD_DIR)/variants/ARDUINO_NANO33BLE/defines.txt
  PLATFORM_DEFINES += -DARDUINO_ARCH_NRF52840 -DARDUINO=10607 -DARDUINO_ARDUINO_NANO33BLE
  PLATFORM_DEFINES += -DARDUINO_ARCH_MBED_NANO -DARDUINO_ARCH_MBED -DARDUINO_LIBRARY_DISCOVERY_PHASE=0

  # Add specific compilation flags
  PLATFORM_FLAGS += \
    -nostdlib \
    -MMD \
    -DAPPLICATION_ADDR=0x10000 \
    -DAPPLICATION_SIZE=0xf0000 \
    -DMBED_RAM_SIZE=0x40000 \
    -DMBED_RAM_START=0x20000000 \
    -DMBED_ROM_SIZE=0x100000 \
    -DMBED_ROM_START=0x0 \
    -DMBED_TRAP_ERRORS_ENABLED=1
  
  # Add specfic linker flags
  LDFLAGS += \
    -DMBED_APP_SIZE=0xf0000 \
    -DMBED_APP_START=0x10000 \
    -DMBED_BOOT_STACK_SIZE=1024 \
    -DMBED_RAM_SIZE=0x40000 \
    -DMBED_RAM_START=0x20000000 \
    -DMBED_ROM_SIZE=0x100000 \
    -DMBED_ROM_START=0x0 \
    -DXIP_ENABLE=0 \
    -Wl,--wrap,_calloc_r \
    -Wl,--wrap,_free_r \
    -Wl,--wrap,_malloc_r \
    -Wl,--wrap,_memalign_r \
    -Wl,--wrap,_realloc_r \
    -Wl,--wrap,atexit \
    -Wl,--wrap,exit \
    -Wl,--wrap,main \
    -Wl,-n
endif

# For DWT/PMU counters. Header file name is depending on target architecture.
#PLATFORM_FLAGS += -DCMSIS_DEVICE_ARM_CORTEX_M_XX_HEADER_FILE=\"$(ARM_CPU)$(CMSIS_ARM_FEATURES).h\"
PLATFORM_FLAGS += -D$(ARM_CPU)

# Arm Cortex-M55 and Cortex-M85 use PMU counters.
ifneq ($(filter "ARMCM55" "ARMCM85",$(ARM_CPU)),)
  PLATFORM_FLAGS += -DARM_MODEL_USE_PMU_COUNTERS
endif

# Common + C/C++ flags
CXXFLAGS += $(PLATFORM_FLAGS) 
CCFLAGS  += $(PLATFORM_FLAGS) 
CXXFLAGS += $(PLATFORM_DEFINES)
CCFLAGS += $(PLATFORM_DEFINES)

# Needed for the project generation interface.
#MICROLITE_CC_HDRS += \
#  $(TENSORFLOW_ROOT)tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h

# We only include micro_speech for project generation to allow for all the files
# to be downloaded. We do not include it for an actual build with the
# cortex_m_generic target to prevent kissfft symbols from getting included in
# libtensorflow-microlite.a which can result in symbol collision.
ifneq ($(TARGET_ARCH), project_generation)
  EXCLUDED_EXAMPLE_TESTS := \
    $(TENSORFLOW_ROOT)tensorflow/lite/micro/examples/micro_speech/Makefile.inc
  MICRO_LITE_EXAMPLE_TESTS := $(filter-out $(EXCLUDED_EXAMPLE_TESTS), $(MICRO_LITE_EXAMPLE_TESTS))
endif