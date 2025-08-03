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

#                                   ~~~HOW TO RUN~~~
# From src directory of library:
#
# make -j4 -f tensorflow/lite/micro/tools/make/Makefile microlite
#
# Configured for Arduino Nano 33 BLE Sense rev.1 (Cortex M4F)
#
# Creates static library /tmp/gen/cortex_m_generic_cortex-m4+sfp_default/lib/libtensorflow-microlite.a
#  which can be copied to <path to tflite lib>/src/cortex-m4/fpv4-sp-d16-softfp for faster compilation
#  of Arduino sketches.
#

ifneq (3.82,$(firstword $(sort $(MAKE_VERSION) 3.82)))
  $(error "Requires make version 3.82 or later (current is $(MAKE_VERSION))")
endif

ifneq ($(TAGS),)
  $(error The TAGS command line option is no longer supported in the TFLM Makefile.)
endif

# root directory of tensorflow
TENSORFLOW_ROOT := ./
RELATIVE_MAKEFILE_DIR := tensorflow/lite/micro/tools/make
MAKEFILE_DIR := $(TENSORFLOW_ROOT)$(RELATIVE_MAKEFILE_DIR)
CORE_VER := $(shell /tmp/bin/arduino-cli core list | grep mbed_nano | tr -s [:space:] | cut -d ' ' -f2)
ARDUINO_BOARD_DIR := ${HOME}/.arduino15/packages/arduino/hardware/mbed_nano/$(CORE_VER)
BAZEL_ROOT := $(TENSORFLOW_ROOT)

ifeq ($(BAZEL_ROOT),)
 BAZEL_ROOT = .
endif


# This is the downloads directory inside the makefiles directory
DOWNLOADS_DIR := $(TENSORFLOW_ROOT)third_party

# Specify which specialized kernel implementation should be pulled in.
OPTIMIZED_KERNEL_DIR := cmsis_nn

# Override this variable from the command line in case the optimized kernels are
# in a different directory.
OPTIMIZED_KERNEL_DIR_PREFIX := $(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels
OPTIMIZED_SIGNAL_KERNEL_DIR_PREFIX := $(TENSORFLOW_ROOT)signal/micro/kernels

# This is the way to specify any code path that we want to Include in the build
# process. This will help us to include external code (code that is not part of
# github repo) be tested.
EXTERNAL_DIR := 

# Pull in some convenience functions.
include $(MAKEFILE_DIR)/helper_functions.inc

# Try to figure out the host system
HOST_OS :=
ifeq ($(OS),Windows_NT)
	HOST_OS = windows
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		HOST_OS := linux
	endif
	ifeq ($(UNAME_S),Darwin)
		HOST_OS := osx
	endif
endif

# Determine the host architecture, with any ix86 architecture being labelled x86_32
HOST_ARCH := $(shell if uname -m | grep -Eq 'i[345678]86'; then echo x86_32; else echo $(shell uname -m); fi)

# Override these on the make command line to target a specific architecture. For example:
# make -f tensorflow/lite/Makefile TARGET=rpi TARGET_ARCH=armv7l
ARDUINO_BOARD := NANO33BLE
TARGET := cortex_m_generic
TARGET_ARCH := cortex-m4+sfp

# Specify which co-processor's kernel implementation should be pulled in.
# If the same kernel is implemented in both kernels/OPTIMIZED_KERNEL_DIR and
# kernels/CO_PROCESSOR, then the implementation from kernels/CO_PROCESSOR will
# be used.
CO_PROCESSOR :=

# Default compiler and tool names:
TOOLCHAIN := gcc
CXX_TOOL := g++
CC_TOOL := gcc
AR_TOOL := ar


INCLUDES := \
-I. \
-I$(DOWNLOADS_DIR) \
-I$(DOWNLOADS_DIR)/gemmlowp \
-I$(DOWNLOADS_DIR)/flatbuffers/include \
-I$(DOWNLOADS_DIR)/kissfft \
-I$(DOWNLOADS_DIR)/ruy

ifneq ($(TENSORFLOW_ROOT),)
  INCLUDES += -I$(TENSORFLOW_ROOT)
endif

TEST_SCRIPT :=

MICROLITE_LIBS := -lm

# For the optimized_kernel_dir, and co-processor as specified on the
# command line we add -D<tag> to the cflags to allow for #idefs in the code.
#
# We apply the following transformations (via the tr command):
#   1. Convert to uppercase (OPTIMIZED_KERNEL_DIR=xtensa -> -DXTENSA)
ADDITIONAL_DEFINES :=
ifneq ($(OPTIMIZED_KERNEL_DIR),)
  ADDITIONAL_DEFINES += -D$(shell echo $(OPTIMIZED_KERNEL_DIR) | tr [a-z] [A-Z])
endif

ifneq ($(CO_PROCESSOR),)
  ADDITIONAL_DEFINES += -D$(shell echo $(CO_PROCESSOR) | tr [a-z] [A-Z])
endif


ifeq ($(TOOLCHAIN), armclang)
  CORE_OPTIMIZATION_LEVEL := -Oz
else
  CORE_OPTIMIZATION_LEVEL := -Os # optimize for size (uses mostly -O2 opts)
endif
KERNEL_OPTIMIZATION_LEVEL := -O2
THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL := -O2

# Warn if deprecated optimization level is set.
OPTIMIZATION_LEVEL :=
ifneq ($(OPTIMIZATION_LEVEL),)
$(error "OPTIMIZATION_LEVEL is no longer used.")
endif

CC_WARNINGS := \
  -Wsign-compare \
  -Wdouble-promotion \
  -Wunused-variable \
  -Wunused-function \
  -Wswitch \
  -Wvla \
  -Wall \
  -Wextra \
  -Wno-missing-field-initializers \
  -Wstrict-aliasing \
  -Wno-unused-parameter

ifneq ($(TOOLCHAIN), gcc)
  # GCC can be overly aggressive with shadow warnings, such as warning when a
  # lambda has variable with the same name as a non-captured variable from the
  # enclosing scope. As such, we don't enable shadow warnings on gcc.
  # https://stackoverflow.com/q/66404751
  CC_WARNINGS += -Wshadow
endif

COMMON_FLAGS := \
  -Werror \
  -fno-unwind-tables \
  -ffunction-sections \
  -fdata-sections \
  -fmessage-length=0 \
  -DTF_LITE_DISABLE_X86_NEON \
  -DTF_LITE_STATIC_MEMORY \
  $(CC_WARNINGS) \
  $(ADDITIONAL_DEFINES)

ifeq ($(TARGET), $(HOST_OS))
  # If we are not doing a cross-compilation then -DTF_LITE_USE_CTIME is what we
  # want to have by default.
  COMMON_FLAGS += -DTF_LITE_USE_CTIME
endif

CXXFLAGS := \
  -std=gnu++14 \
  -fno-rtti \
  -fno-exceptions \
  -fno-threadsafe-statics \
  $(COMMON_FLAGS)
  #-Wnon-virtual-dtor  will not compile the arduino core

CCFLAGS := \
  -Wimplicit-function-declaration \
  -std=gnu11 \
  $(COMMON_FLAGS)

ARFLAGS := -r

LDFLAGS := 

ifeq ($(TOOLCHAIN), gcc)
  ifneq ($(TARGET), osx)
    # GCC on MacOS uses an LLVM backend so we avoid the additional linker flags
    # that are unsupported with LLVM.
    LDFLAGS += \
      -Wl,--fatal-warnings \
      -Wl,--gc-sections
  endif
endif

# Specifying BUILD_TYPE=<blah> as part of the make command gives us a few
# options to choose from.
#
# If BUILD_TYPE is not specified, the default build (which should be suitable
# most of the time) has all of the error checking logic at the expense of a
# latency increase of ~5-10% relative to BUILD_TYPE=release_with_logs.
#
# This default build is most suited for usual development and testing as is
# highlighted by the discussion on this github pull request:
# https://github.com/tensorflow/tensorflow/pull/42314#issuecomment-694360567
BUILD_TYPE := default
ifeq ($(BUILD_TYPE), debug)
# Specifying BUILD_TYPE=debug adds debug symbols to the binary (and makes it
# larger) and should be used to run a binary with gdb.
# Also sets the optimization flags to optimize the debugging experience.
	CXXFLAGS += -g
	CCFLAGS  += -g
  ifeq ($(TOOLCHAIN), armclang)
    CORE_OPTIMIZATION_LEVEL := -O1
    KERNEL_OPTIMIZATION_LEVEL := -O1
    THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL := -O1
  else
    CORE_OPTIMIZATION_LEVEL := -Og
    KERNEL_OPTIMIZATION_LEVEL := -Og
    THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL := -Og
  endif
else ifeq ($(BUILD_TYPE), release)
# The 'release' build results in the smallest binary (by virtue of removing
# strings from log messages, DCHECKs ...).
#
# The down-side is that we currently do not have a good mechanism to allow
# for logging that is not related to errors (e.g. profiling information, or
# logs that help determine if tests pass or fail). As a result, we are unable
# to run tests or benchmarks with BUILD_TYPE=release (which is a bit
# counter-intuitive). TODO(b/158205789): A global error reporter might help.
#
# For a close approximation of the release build use
# BUILD_TYPE=release_with_logs.
	CXXFLAGS += -DNDEBUG -DTF_LITE_STRIP_ERROR_STRINGS
	CCFLAGS  += -DNDEBUG -DTF_LITE_STRIP_ERROR_STRINGS
else ifeq ($(BUILD_TYPE), release_with_logs)
# The latency with BUILD_TYPE=release_with_logs will be close to the 'release'
# build and there will still be error logs. This build type may be preferable
# for profiling and benchmarking.
	CXXFLAGS += -DNDEBUG
	CCFLAGS  += -DNDEBUG
else ifeq ($(BUILD_TYPE), no_tf_lite_static_memory)
# TODO(b/287320282): remove the no_tf_lite_static_memory build.
#
# This build should not be used to run any binaries/tests since
# TF_LITE_STATIC_MEMORY should be defined for all micro builds. However,
# having a build without TF_LITE_STATIC_MEMORY is useful to catch errors in
# code that is shared between TfLite Mobile and TfLite Micro. See this issue
# for more details:
# https://github.com/tensorflow/tensorflow/issues/43076
	CXXFLAGS := $(filter-out -DTF_LITE_STATIC_MEMORY, $(CXXFLAGS))
	CCFLAGS := $(filter-out -DTF_LITE_STATIC_MEMORY, $(CCFLAGS))

# We are using C++17 for the no_tf_lite_static_memory_build to make it close
# to the TfLite bazel build.
	CXXFLAGS := $(filter-out -std=gnu++14, $(CXXFLAGS))
	CXXFLAGS += -std=c++17
	CCFLAGS := $(filter-out -std=gnu11, $(CCLAGS))
	CCFLAGS += -std=c17
endif

# This library is the main target for this makefile. It will contain a minimal
# runtime that can be linked in to other programs.
MICROLITE_LIB_NAME := libtensorflow-microlite.a

# Where compiled objects are stored.
BASE_GENDIR := /tmp/gen
GENDIR := $(BASE_GENDIR)/$(TARGET)_$(TARGET_ARCH)_$(BUILD_TYPE)/
CORE_OBJDIR := $(GENDIR)obj/core/
KERNEL_OBJDIR := $(GENDIR)obj/kernels/
THIRD_PARTY_KERNEL_OBJDIR := $(GENDIR)obj/third_party_kernels/
THIRD_PARTY_OBJDIR := $(GENDIR)obj/third_party/
GENERATED_SRCS_DIR := $(GENDIR)genfiles/
BINDIR := $(GENDIR)bin/
LIBDIR := $(GENDIR)lib/
PRJDIR := $(GENDIR)prj/

# These two must be defined before we include the target specific Makefile.inc
# because we filter out the examples that are not supported for those targets.
# See targets/xtensa_xpg_makefile.inc for an example.
#
# We limit max depth of directories to search to not include target specific
# Makefiles that are included directly by the main example Makefile. See
# examples/micro_speech/Makefile.inc for an example. At the same time, we
# search till an arbitrary depth for files named Makefile_internal.inc as a way
# to bypass this check and allow for deeper directory structures.
#MICRO_LITE_EXAMPLE_TESTS := $(shell find $(TENSORFLOW_ROOT)tensorflow/lite/micro/examples/ -maxdepth 2 -name Makefile.inc)

# Internal examples are copied outside the TFLM repo in google.
# ifneq ($(EXTERNAL_DIR),)
#   MICRO_LITE_EXAMPLE_TESTS += $(shell find $(EXTERNAL_DIR) -name Makefile_internal.inc)
# endif

# Kernel integration tests must be excluded on certain targets.
# MICRO_LITE_INTEGRATION_TESTS += $(shell find $(TENSORFLOW_ROOT)tensorflow/lite/micro/integration_tests -name Makefile.inc)

# MICRO_LITE_GEN_MUTABLE_OP_RESOLVER_TEST += \
#   $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tools/gen_micro_mutable_op_resolver_test/person_detect/Makefile.inc)

# OLD_MICRO_LITE_BENCHMARKS := $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/benchmarks/Makefile.inc)
# MICRO_LITE_BENCHMARKS := $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tools/benchmarking/Makefile.inc)

# MICRO_LITE_LAYER_BY_LAYER_OUTPUT := $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tools/Makefile.inc)

# TODO(b/152645559): move all benchmarks to benchmarks directory.
# MICROLITE_BENCHMARK_SRCS := \
# $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/benchmarks/*benchmark.cpp) \
# $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tools/benchmarking/*benchmark.cpp)


# MICROLITE_TEST_SRCS := \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/fake_micro_context_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/flatbuffer_utils_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_arena_threshold_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_helpers_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_allocation_info_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_interpreter_context_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_log_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_interpreter_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_mutable_op_resolver_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_resource_variable_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_time_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_utils_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/recording_micro_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing_helpers_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_planner/greedy_memory_planner_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_planner/linear_memory_planner_test.cpp \
# $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim_test.cpp

MICROLITE_CC_KERNEL_SRCS := \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/activations.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/activations_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/add.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/add_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/add_n.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/arg_min_max.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/assign_variable.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/batch_matmul.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/batch_to_space_nd.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/broadcast_args.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/broadcast_to.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/call_once.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/cast.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/ceil.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/circular_buffer.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/circular_buffer_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/comparisons.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/concatenation.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/conv.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/conv_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/cumsum.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/depth_to_space.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/depthwise_conv.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/depthwise_conv_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/dequantize.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/dequantize_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/detection_postprocess.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/div.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/elementwise.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/elu.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/embedding_lookup.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/ethosu.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/exp.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/expand_dims.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/fill.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/floor.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/floor_div.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/floor_mod.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/fully_connected.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/fully_connected_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/gather.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/gather_nd.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/hard_swish.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/hard_swish_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/if.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/kernel_runner.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/kernel_util.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/l2norm.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/l2_pool_2d.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/leaky_relu.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/leaky_relu_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/logical.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/logical_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/logistic.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/logistic_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/log_softmax.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/lstm_eval.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/lstm_eval_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/maximum_minimum.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/micro_tensor_utils.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/mirror_pad.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/mul.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/mul_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/neg.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/pack.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/pad.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/pooling.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/pooling_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/prelu.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/prelu_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/quantize.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/quantize_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/read_variable.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/reduce.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/reduce_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/reshape.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/reshape_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/resize_bilinear.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/resize_nearest_neighbor.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/round.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/select.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/shape.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/slice.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/softmax.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/softmax_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/space_to_batch_nd.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/space_to_depth.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/split.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/split_v.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/squared_difference.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/squeeze.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/strided_slice.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/strided_slice_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/sub.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/sub_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/svdf.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/svdf_common.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/tanh.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/transpose.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/transpose_conv.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/unidirectional_sequence_lstm.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/unpack.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/var_handle.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/while.cpp \
$(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/zeros_like.cpp

MICROLITE_CC_SIGNAL_KERNEL_SRCS := \
$(TENSORFLOW_ROOT)signal/micro/kernels/delay.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/energy.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/fft_auto_scale_kernel.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/fft_auto_scale_common.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/filter_bank.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/filter_bank_log.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/filter_bank_square_root.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/filter_bank_square_root_common.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/filter_bank_spectral_subtraction.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/framer.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/irfft.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/rfft.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/stacker.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/overlap_add.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/pcan.cpp \
$(TENSORFLOW_ROOT)signal/micro/kernels/window.cpp \
$(TENSORFLOW_ROOT)signal/src/circular_buffer.cpp \
$(TENSORFLOW_ROOT)signal/src/energy.cpp \
$(TENSORFLOW_ROOT)signal/src/fft_auto_scale.cpp \
$(TENSORFLOW_ROOT)signal/src/filter_bank.cpp \
$(TENSORFLOW_ROOT)signal/src/filter_bank_log.cpp \
$(TENSORFLOW_ROOT)signal/src/filter_bank_square_root.cpp \
$(TENSORFLOW_ROOT)signal/src/filter_bank_spectral_subtraction.cpp \
$(TENSORFLOW_ROOT)signal/src/irfft_float.cpp \
$(TENSORFLOW_ROOT)signal/src/irfft_int16.cpp \
$(TENSORFLOW_ROOT)signal/src/irfft_int32.cpp \
$(TENSORFLOW_ROOT)signal/src/log.cpp \
$(TENSORFLOW_ROOT)signal/src/max_abs.cpp \
$(TENSORFLOW_ROOT)signal/src/msb_32.cpp \
$(TENSORFLOW_ROOT)signal/src/msb_64.cpp \
$(TENSORFLOW_ROOT)signal/src/overlap_add.cpp \
$(TENSORFLOW_ROOT)signal/src/pcan_argc_fixed.cpp \
$(TENSORFLOW_ROOT)signal/src/rfft_float.cpp \
$(TENSORFLOW_ROOT)signal/src/rfft_int16.cpp \
$(TENSORFLOW_ROOT)signal/src/rfft_int32.cpp \
$(TENSORFLOW_ROOT)signal/src/square_root_32.cpp \
$(TENSORFLOW_ROOT)signal/src/square_root_64.cpp \
$(TENSORFLOW_ROOT)signal/src/window.cpp

# MICROLITE_TEST_HDRS := \
# $(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/*.h)

# lite/experimental needed for micro_speech example
TFL_CC_SRCS := \
$(shell find $(TENSORFLOW_ROOT)tensorflow/ -type d \( -path $(TENSORFLOW_ROOT)tensorflow/lite/micro \) -prune -false -o -name "*.cpp" -o -name "*.c")

TFL_CC_HDRS := \
$(shell find $(TENSORFLOW_ROOT)tensorflow/ -type d \( -path $(TENSORFLOW_ROOT)tensorflow/lite/micro \) -prune -false -o -name "*.h")


ifneq ($(BUILD_TYPE), no_tf_lite_static_memory)
  EXCLUDED_TFL_CC_SRCS := \
  	$(TENSORFLOW_ROOT)tensorflow/lite/array.cpp
  TFL_CC_SRCS := $(filter-out $(EXCLUDED_TFL_CC_SRCS), $(TFL_CC_SRCS))

  EXCLUDED_TFL_CC_HDRS := \
  	$(TENSORFLOW_ROOT)tensorflow/lite/array.h
  TFL_CC_HDRS := $(filter-out $(EXCLUDED_TFL_CC_HDRS), $(TFL_CC_HDRS))
endif

MICROLITE_CC_BASE_SRCS := \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/*.cpp) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/*.cpp) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_planner/*.cpp) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tflite_bridge/*.cpp) \
$(TFL_CC_SRCS)

MICROLITE_CC_HDRS := \
$(wildcard $(TENSORFLOW_ROOT)signal/micro/kernels/*.h) \
$(wildcard $(TENSORFLOW_ROOT)signal/src/*.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/*.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/benchmarks/*model_data.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/*.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/arena_allocator/*.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_planner/*.h) \
$(wildcard $(TENSORFLOW_ROOT)tensorflow/lite/micro/tflite_bridge/*.h) \
$(TENSORFLOW_ROOT)LICENSE \
$(TFL_CC_HDRS)

# TODO(b/165940489): Figure out how to avoid including fixed point
# platform-specific headers.
# some kiss fft source file has to be included in header sections because
# the implemenation of three different resolution fft from one single c file.
# See http://b/201319430 for additional context.
THIRD_PARTY_CC_HDRS := \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/allocator.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/array.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/base.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/buffer.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/buffer_ref.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/code_generator.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/code_generators.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/default_allocator.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/detached_buffer.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/file_manager.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/flatbuffer_builder.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/flatbuffers.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/flex_flat_util.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/flexbuffers.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/grpc.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/hash.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/idl.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/minireflect.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/reflection.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/reflection_generated.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/registry.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/stl_emulation.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/string.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/struct.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/table.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/util.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/vector.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/vector_downward.h \
$(DOWNLOADS_DIR)/flatbuffers/include/flatbuffers/verifier.h \
$(DOWNLOADS_DIR)/flatbuffers/LICENSE \
$(DOWNLOADS_DIR)/gemmlowp/fixedpoint/fixedpoint.h \
$(DOWNLOADS_DIR)/gemmlowp/fixedpoint/fixedpoint_neon.h \
$(DOWNLOADS_DIR)/gemmlowp/fixedpoint/fixedpoint_sse.h \
$(DOWNLOADS_DIR)/gemmlowp/internal/detect_platform.h \
$(DOWNLOADS_DIR)/gemmlowp/LICENSE \
$(DOWNLOADS_DIR)/kissfft/COPYING \
$(DOWNLOADS_DIR)/kissfft/kiss_fft.c \
$(DOWNLOADS_DIR)/kissfft/kiss_fft.h \
$(DOWNLOADS_DIR)/kissfft/_kiss_fft_guts.h \
$(DOWNLOADS_DIR)/kissfft/tools/kiss_fftr.c \
$(DOWNLOADS_DIR)/kissfft/tools/kiss_fftr.h \
$(DOWNLOADS_DIR)/ruy/ruy/profiler/instrumentation.h

THIRD_PARTY_CC_SRCS :=
THIRD_PARTY_KERNEL_CC_SRCS :=

# Load custom kernels.
include $(MAKEFILE_DIR)/additional_kernels.inc

MICROLITE_CC_SRCS := $(filter-out $(MICROLITE_TEST_SRCS), $(MICROLITE_CC_BASE_SRCS))
MICROLITE_CC_SRCS := $(filter-out $(MICROLITE_BENCHMARK_SRCS), $(MICROLITE_CC_SRCS))


# The download scripts require that the downloads directory already exist for
# improved error checking. To accomodate that, we first create a downloads
# directory.
# $(shell mkdir -p ${DOWNLOADS_DIR})

# # Directly download the flatbuffers library.
# DOWNLOAD_RESULT := $(shell $(MAKEFILE_DIR)/flatbuffers_download.sh ${DOWNLOADS_DIR} $(TENSORFLOW_ROOT))
# ifneq ($(DOWNLOAD_RESULT), SUCCESS)
#   $(error Something went wrong with the flatbuffers download: $(DOWNLOAD_RESULT))
# endif

# DOWNLOAD_RESULT := $(shell $(MAKEFILE_DIR)/kissfft_download.sh ${DOWNLOADS_DIR} $(TENSORFLOW_ROOT))
# ifneq ($(DOWNLOAD_RESULT), SUCCESS)
#   $(error Something went wrong with the kissfft download: $(DOWNLOAD_RESULT))
# endif

# DOWNLOAD_RESULT := $(shell $(MAKEFILE_DIR)/pigweed_download.sh ${DOWNLOADS_DIR} $(TENSORFLOW_ROOT))
# ifneq ($(DOWNLOAD_RESULT), SUCCESS)
#   $(error Something went wrong with the pigweed download: $(DOWNLOAD_RESULT))
# endif

# include $(MAKEFILE_DIR)/third_party_downloads.inc
# THIRD_PARTY_DOWNLOADS :=
# $(eval $(call add_third_party_download,$(GEMMLOWP_URL),$(GEMMLOWP_MD5),gemmlowp,))
# $(eval $(call add_third_party_download,$(RUY_URL),$(RUY_MD5),ruy,))

# The target-specific makefile must have a name that is exactly
# TARGET_makefile.inc and is only needed for cross-compilation (i.e. when TARGET
# is different from the HOST_OS).
TARGETS_WITHOUT_MAKEFILES := \
$(HOST_OS)

# This specific string needs to be outputted for a test to be recognized as
# having passed.
TEST_PASS_STRING := '~~~ALL TESTS PASSED~~~'

# ${TARGET}_makefile.inc can set this to true to allow it to defined a custom
# implementation for `make test`. See bluepill_makefile as an example.
TARGET_SPECIFIC_MAKE_TEST:=0

ifeq ($(findstring $(TARGET),$(TARGETS_WITHOUT_MAKEFILES)),)
  include $(MAKEFILE_DIR)/targets/$(TARGET)_makefile.inc
endif

ifneq ($(OPTIMIZED_KERNEL_DIR),)
  PATH_TO_OPTIMIZED_KERNELS := $(OPTIMIZED_KERNEL_DIR_PREFIX)/$(OPTIMIZED_KERNEL_DIR)
  PATH_TO_SIGNAL_OPTIMIZED_KERNELS := $(OPTIMIZED_SIGNAL_KERNEL_DIR_PREFIX)/$(OPTIMIZED_KERNEL_DIR)

  # Check that OPTIMIZED_KERNEL_DIR is valid to avoid unexpected fallback to
  # reference kernels. See http://b/183546742 for more context.
  RESULT := $(shell $(MAKEFILE_DIR)/check_optimized_kernel_dir.sh $(PATH_TO_OPTIMIZED_KERNELS))
  ifneq ($(RESULT), SUCCESS)
    $(error Incorrect OPTIMIZED_KERNEL_DIR: $(RESULT))
  endif

  include $(MAKEFILE_DIR)/ext_libs/$(OPTIMIZED_KERNEL_DIR).inc
  # Specialize for the optimized kernels
  MICROLITE_CC_KERNEL_SRCS := $(shell python3 $(MAKEFILE_DIR)/specialize_files.py \
		--base_files "$(MICROLITE_CC_KERNEL_SRCS)" \
		--specialize_directory $(PATH_TO_OPTIMIZED_KERNELS))

  ifneq ($(filter $(OPTIMIZED_KERNEL_DIR), xtensa),)
    # Check that OPTIMIZED_KERNEL_DIR is valid to avoid unexpected fallback to
    # reference kernels. See http://b/183546742 for more context.
    RESULT := $(shell $(MAKEFILE_DIR)/check_optimized_kernel_dir.sh $(PATH_TO_SIGNAL_OPTIMIZED_KERNELS))
    ifneq ($(RESULT), SUCCESS)
      $(error Incorrect SIGNAL OPTIMIZED_KERNEL_DIR: $(RESULT))
    endif

    # Specialize for the optimized kernels
    MICROLITE_CC_SIGNAL_KERNEL_SRCS := $(shell python3 $(MAKEFILE_DIR)/specialize_files.py \
      --base_files "$(MICROLITE_CC_SIGNAL_KERNEL_SRCS)" \
      --specialize_directory $(PATH_TO_SIGNAL_OPTIMIZED_KERNELS))

    MICROLITE_CC_KERNEL_SRCS += $(wildcard $(PATH_TO_SIGNAL_OPTIMIZED_KERNELS)/*.S)
    MICROLITE_CC_HDRS += $(wildcard $(PATH_TO_SIGNAL_OPTIMIZED_KERNELS)/*.h)
  endif

  # The first ifneq is needed to be compatible with make versions prior to 4.2
  # which do not support .SHELLSTATUS. While make 4.2 was released in 2016,
  # Ubuntu 18.04 only has version 4.1
  ifneq ($(.SHELLSTATUS),)
    ifneq ($(.SHELLSTATUS),0)
      $(error Error with specialize_files.py $(MICROLITE_CC_KERNEL_SRCS))
    endif
  endif

  # Optimized kernel directories can have their own header files which need to
  # be included in MICROLITE_CC_HDRS for project generation to have a complete
  # list of headers.
  MICROLITE_CC_HDRS += $(wildcard $(PATH_TO_OPTIMIZED_KERNELS)/*.h)
endif

MICROLITE_CC_KERNEL_SRCS += $(MICROLITE_CC_SIGNAL_KERNEL_SRCS)

MICROLITE_CC_KERNEL_SRCS += $(wildcard $(TENSORFLOW_ROOT)test_over_serial/*.cpp)
MICROLITE_CC_HDRS += $(wildcard $(TENSORFLOW_ROOT)test_over_serial/*.h)

MICROLITE_CC_KERNEL_SRCS += $(wildcard $(TENSORFLOW_ROOT)peripherals/*.cpp)
MICROLITE_CC_HDRS += $(wildcard $(TENSORFLOW_ROOT)peripherals/*.h)

# If a co-processor is specified on the command line with
# CO_PROCESSOR=<co_processor> then we will include ext_libs/<co_processor>.inc
# and find additional kernel sources in kernels/<co_processor>/
#
# That the co-processor specialization of the kernel sources happens after the
# optimized_kernel_dir means that if there is an implementation of the same
# kernel in both directories, the one from co_processor will be used.
ifneq ($(CO_PROCESSOR),)
  include $(MAKEFILE_DIR)/ext_libs/$(CO_PROCESSOR).inc
  # Specialize for the coprocessor kernels.
  PATH_TO_COPROCESSOR_KERNELS := $(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/$(CO_PROCESSOR)
  MICROLITE_CC_KERNEL_SRCS := $(shell python3 $(MAKEFILE_DIR)/specialize_files.py \
    --base_files "$(MICROLITE_CC_KERNEL_SRCS)" \
    --specialize_directory $(PATH_TO_COPROCESSOR_KERNELS))

  # The first ifneq is needed to be compatible with make versions prior to 4.2
  # which do not support .SHELLSTATUS. While make 4.2 was released in 2016,
  # Ubuntu 18.04 only has version 4.1
  ifneq ($(.SHELLSTATUS),)
    ifneq ($(.SHELLSTATUS),0)
      $(error Error with specialize_files.py $(MICROLITE_CC_KERNEL_SRCS))
    endif
  endif
endif

ALL_SRCS := \
	$(MICROLITE_CC_SRCS) \
	$(MICROLITE_CC_KERNEL_SRCS) \
	$(MICROLITE_TEST_SRCS)

MICROLITE_LIB_PATH := $(LIBDIR)$(MICROLITE_LIB_NAME)

CXX := $(TARGET_TOOLCHAIN_ROOT)${TARGET_TOOLCHAIN_PREFIX}${CXX_TOOL}
CC := $(TARGET_TOOLCHAIN_ROOT)${TARGET_TOOLCHAIN_PREFIX}${CC_TOOL}
AR := $(TARGET_TOOLCHAIN_ROOT)${TARGET_TOOLCHAIN_PREFIX}${AR_TOOL}

# The default Makefile target(all) must appear before any target,
# which is compiled if there's no command-line arguments.
all: $(MICROLITE_LIB_PATH)

# Include output directory since example cc files depend on generated headers.
# INCLUDES += -I$(GENERATED_SRCS_DIR)
# INCLUDES += -I$(GENERATED_SRCS_DIR)$(TENSORFLOW_ROOT)

ifneq ($(EXTERNAL_DIR),)
  INCLUDES += $(EXTERNAL_DIR)
endif

# # Load the examples.
# include $(MICRO_LITE_EXAMPLE_TESTS)

# # Load the integration tests.
# include $(MICRO_LITE_INTEGRATION_TESTS)

# # Load generated micro mutable op resolver test.
# include ${MICRO_LITE_GEN_MUTABLE_OP_RESOLVER_TEST}


# # Load the benchmarks.
# include $(OLD_MICRO_LITE_BENCHMARKS)
# include $(MICRO_LITE_BENCHMARKS)

# # Load custom kernel tests.
# include $(MAKEFILE_DIR)/additional_tests.inc


# Create rules for downloading third-party dependencies.
THIRD_PARTY_TARGETS :=
$(foreach DOWNLOAD,$(THIRD_PARTY_DOWNLOADS),$(eval $(call create_download_rule,$(DOWNLOAD))))
third_party_downloads: $(THIRD_PARTY_TARGETS)

MICROLITE_LIB_OBJS := $(addprefix $(CORE_OBJDIR), \
$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(MICROLITE_CC_SRCS)))))

MICROLITE_THIRD_PARTY_OBJS := $(addprefix $(THIRD_PARTY_OBJDIR), \
$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(THIRD_PARTY_CC_SRCS)))))

MICROLITE_THIRD_PARTY_KERNEL_OBJS := $(addprefix $(THIRD_PARTY_KERNEL_OBJDIR), \
$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(THIRD_PARTY_KERNEL_CC_SRCS)))))

MICROLITE_KERNEL_OBJS := $(addprefix $(KERNEL_OBJDIR), \
$(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(MICROLITE_CC_KERNEL_SRCS)))))

$(CORE_OBJDIR)%.o: %.cpp $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(CORE_OBJDIR)%.o: %.c $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(CORE_OBJDIR)%.o: %.S $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_OBJDIR)%.o: %.cpp $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_OBJDIR)%.o: %.c $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_OBJDIR)%.o: %.S $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(CORE_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_KERNEL_OBJDIR)%.o: %.cpp $(THIRD_PARTY_KERNEL_TARGETS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_KERNEL_OBJDIR)%.o: %.c $(THIRD_PARTY_KERNEL_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(THIRD_PARTY_KERNEL_OBJDIR)%.o: %.S $(THIRD_PARTY_KERNEL_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(THIRD_PARTY_KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(KERNEL_OBJDIR)%.o: %.cpp $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(KERNEL_OBJDIR)%.o: %.c $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

$(KERNEL_OBJDIR)%.o: %.S $(THIRD_PARTY_TARGETS)
	@mkdir -p $(dir $@)
	$(CC) $(CCFLAGS) $(KERNEL_OPTIMIZATION_LEVEL) $(INCLUDES) -c $< -o $@

# Gathers together all the objects we've compiled into a single '.a' archive.
$(MICROLITE_LIB_PATH): $(MICROLITE_LIB_OBJS) $(MICROLITE_KERNEL_OBJS) $(MICROLITE_THIRD_PARTY_OBJS) $(MICROLITE_THIRD_PARTY_KERNEL_OBJS) $(MICROLITE_CUSTOM_OP_OBJS)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $(MICROLITE_LIB_PATH) $(MICROLITE_LIB_OBJS) \
	$(MICROLITE_KERNEL_OBJS) $(MICROLITE_THIRD_PARTY_OBJS) $(MICROLITE_THIRD_PARTY_KERNEL_OBJS) $(MICROLITE_CUSTOM_OP_OBJS)

microlite: $(MICROLITE_LIB_PATH)

# $(BINDIR)%_test : $(CORE_OBJDIR)%_test.o $(MICROLITE_LIB_PATH)
# 	@mkdir -p $(dir $@)
# 	$(CXX) $(CXXFLAGS) $(INCLUDES) \
# 	-o $@ $< \
# 	$(MICROLITE_LIB_PATH) $(LDFLAGS) $(MICROLITE_LIBS)

# $(BINDIR)%.test_target: $(BINDIR)%_test
# 	@test -f $(TEST_SCRIPT) || (echo 'Unable to find the test script. Is the software emulation available in $(TARGET)?'; exit 1)
# 	$(TEST_SCRIPT) $< $(TEST_PASS_STRING)

# Create kernel test targets.
# include $(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/Makefile.inc

# Create binary size test target.
# include $(TENSORFLOW_ROOT)tensorflow/lite/micro/tools/ci_build/binary_size_test/Makefile.inc

# Some tests have additional dependencies (beyond libtensorflow-microlite.a) and
# those need to be explicitly specified with their own individual call to the
# microlite_test helper function. For these tests, we also need to make sure to
# not add targets for them if they have been excluded as part of the target
# specific Makefile.
# EXPLICITLY_SPECIFIED_TEST:= $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_arena_threshold_test.cpp
# ifneq ($(findstring $(EXPLICITLY_SPECIFIED_TEST),$(MICROLITE_TEST_SRCS)),)
#   MICROLITE_TEST_SRCS := $(filter-out $(EXPLICITLY_SPECIFIED_TEST), $(MICROLITE_TEST_SRCS))
#   EXPLICITLY_SPECIFIED_TEST_SRCS := \
#   $(EXPLICITLY_SPECIFIED_TEST) \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.cpp
#   EXPLICITLY_SPECIFIED_TEST_HDRS := \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.h
#   EXPLICITLY_SPECIFIED_TEST_GENERATOR_INPUTS := \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/models/keyword_scrambled.tflite
#   $(eval $(call microlite_test,memory_arena_threshold_test,\
#   $(EXPLICITLY_SPECIFIED_TEST_SRCS),$(EXPLICITLY_SPECIFIED_TEST_HDRS), \
#   $(EXPLICITLY_SPECIFIED_TEST_GENERATOR_INPUTS)))
# endif

# EXPLICITLY_SPECIFIED_TEST:= $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_allocator_test.cpp
# ifneq ($(findstring $(EXPLICITLY_SPECIFIED_TEST),$(MICROLITE_TEST_SRCS)),)
#   MICROLITE_TEST_SRCS := $(filter-out $(EXPLICITLY_SPECIFIED_TEST), $(MICROLITE_TEST_SRCS))
#   EXPLICITLY_SPECIFIED_TEST_SRCS := \
#   $(EXPLICITLY_SPECIFIED_TEST) \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.cpp
#   EXPLICITLY_SPECIFIED_TEST_HDRS := \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.h
#   $(eval $(call microlite_test,micro_allocator_test,\
#   $(EXPLICITLY_SPECIFIED_TEST_SRCS),$(EXPLICITLY_SPECIFIED_TEST_HDRS)))
# endif

# EXPLICITLY_SPECIFIED_TEST:= $(TENSORFLOW_ROOT)tensorflow/lite/micro/recording_micro_allocator_test.cpp
# ifneq ($(findstring $(EXPLICITLY_SPECIFIED_TEST),$(MICROLITE_TEST_SRCS)),)
#   MICROLITE_TEST_SRCS := $(filter-out $(EXPLICITLY_SPECIFIED_TEST), $(MICROLITE_TEST_SRCS))
#   EXPLICITLY_SPECIFIED_TEST_SRCS := \
#   $(EXPLICITLY_SPECIFIED_TEST) \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.cpp
#   EXPLICITLY_SPECIFIED_TEST_HDRS := \
#   $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_conv_model.h
#   $(eval $(call microlite_test,recording_micro_allocator_test,\
#   $(EXPLICITLY_SPECIFIED_TEST_SRCS),$(EXPLICITLY_SPECIFIED_TEST_HDRS)))
# endif

# For all the tests that do not have any additional dependencies, we can
# add a make target in a common way.
# $(foreach TEST_TARGET,$(filter-out $(TENSORFLOW_ROOT)tensorflow/lite/micro/kernels/%,$(MICROLITE_TEST_SRCS)),\
# $(eval $(call microlite_test,$(notdir $(basename $(TEST_TARGET))),$(TEST_TARGET))))

# ifeq ($(TARGET_SPECIFIC_MAKE_TEST),0)
# test: $(MICROLITE_TEST_TARGETS)
# integration_tests: $(MICROLITE_INTEGRATION_TEST_TARGETS)
# generated_micro_mutable_op_resolver: $(MICROLITE_GEN_OP_RESOLVER_TEST_TARGETS)
# endif


# # Just build the test targets
# build: $(MICROLITE_BUILD_TARGETS)

list_library_sources:
	@echo $(MICROLITE_CC_SRCS) $(MICROLITE_CC_KERNEL_SRCS)

list_library_headers:
	@echo $(MICROLITE_CC_HDRS)

list_third_party_sources:
	@echo $(THIRD_PARTY_CC_SRCS) $(THIRD_PARTY_KERNEL_CC_SRCS)

list_third_party_headers:
	@echo $(THIRD_PARTY_CC_HDRS)

list_generator_dir:
	@echo $(GENERATED_SRCS_DIR)

# Gets rid of all generated files.
clean:
	rm -rf $(BASE_GENDIR)

# Removes third-party downloads.
# clean_downloads:
# 	rm -rf $(DOWNLOADS_DIR)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d
.PRECIOUS: $(BINDIR)%_test

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(ALL_SRCS)))
