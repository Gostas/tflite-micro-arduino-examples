/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "tensorflow/lite/core/c/common.h"

#include <cstring>
#include <type_traits>
#include <utility>

#include "tensorflow/lite/core/c/c_api_types.h"
#ifdef TF_LITE_TENSORFLOW_PROFILER
#include "tensorflow/lite/tensorflow_profiler_logger.h"
#endif

namespace {

template <class T>
size_t TfLiteVarArrayGetSizeInBytes(const int size) {
  constexpr size_t data_size = sizeof(std::declval<T>().data[0]);
  size_t computed_size = sizeof(T) + data_size * size;
#if defined(_MSC_VER)
  // Context for why this is needed is in http://b/189926408#comment21
  computed_size -= data_size;
#endif
  return computed_size;
}

template <class T, class U>
int TfLiteVarArrayEqualsArray(const T* const a, const int b_size,
                              const U* const b_data) {
  static_assert(std::is_same<decltype(a->data[0]), const U&>::value,
                "TfLiteVarArrayEqualsArray can only compare same type arrays");
  if (a == nullptr) {
    return b_size == 0;
  }
  if (a->size != b_size) {
    return 0;
  }
  return !memcmp(a->data, b_data, a->size * sizeof(a->data[0]));
}

template <class T>
int TfLiteVarArrayEqual(const T* const a, const T* const b) {
  // This goes first because null arrays must compare equal.
  if (a == b) {
    return 1;
  }
  if (a == nullptr || b == nullptr) {
    return 0;
  }
  return TfLiteVarArrayEqualsArray(a, b->size, b->data);
}

template <class T>
void TfLiteVarArrayFree(T* a) {
  free(a);
}

}  // namespace

extern "C" {

size_t TfLiteIntArrayGetSizeInBytes(int size) {
  return TfLiteVarArrayGetSizeInBytes<TfLiteIntArray>(size);
}

int TfLiteIntArrayEqual(const TfLiteIntArray* a, const TfLiteIntArray* b) {
  return TfLiteVarArrayEqual(a, b);
}

int TfLiteIntArrayEqualsArray(const TfLiteIntArray* a, int b_size,
                              const int b_data[]) {
  return TfLiteVarArrayEqualsArray(a, b_size, b_data);
}

int TfLiteFloatArrayGetSizeInBytes(int size) {
  return TfLiteVarArrayGetSizeInBytes<TfLiteFloatArray>(size);
}

const char* TfLiteTypeGetName(TfLiteType type) {
  switch (type) {
    case kTfLiteNoType:
      return "NOTYPE";
    case kTfLiteFloat32:
      return "FLOAT32";
    case kTfLiteUInt16:
      return "UINT16";
    case kTfLiteInt16:
      return "INT16";
    case kTfLiteInt32:
      return "INT32";
    case kTfLiteUInt32:
      return "UINT32";
    case kTfLiteUInt8:
      return "UINT8";
    case kTfLiteInt8:
      return "INT8";
    case kTfLiteInt64:
      return "INT64";
    case kTfLiteUInt64:
      return "UINT64";
    case kTfLiteBool:
      return "BOOL";
    case kTfLiteComplex64:
      return "COMPLEX64";
    case kTfLiteComplex128:
      return "COMPLEX128";
    case kTfLiteString:
      return "STRING";
    case kTfLiteFloat16:
      return "FLOAT16";
    case kTfLiteFloat64:
      return "FLOAT64";
    case kTfLiteResource:
      return "RESOURCE";
    case kTfLiteVariant:
      return "VARIANT";
    case kTfLiteInt4:
      return "INT4";
  }
  return "Unknown type";
}

TfLiteDelegate TfLiteDelegateCreate() { return TfLiteDelegate{}; }

// Returns a tensor data allocation strategy.
TfLiteAllocationStrategy TfLiteTensorGetAllocationStrategy(
    const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteAllocationStrategyNone;
    case kTfLiteMmapRo:
      return kTfLiteAllocationStrategyMMap;
    case kTfLiteArenaRw:
      return kTfLiteAllocationStrategyArena;
    case kTfLiteArenaRwPersistent:
      return kTfLiteAllocationStrategyArena;
    case kTfLiteDynamic:
      return kTfLiteAllocationStrategyMalloc;
    case kTfLitePersistentRo:
      return kTfLiteAllocationStrategyUnknown;
    case kTfLiteCustom:
      return kTfLiteAllocationStrategyUnknown;
    case kTfLiteVariantObject:
      return kTfLiteAllocationStrategyNew;
  }
  return kTfLiteAllocationStrategyUnknown;
}

// Returns how stable a tensor data buffer address is across runs.
TfLiteRunStability TfLiteTensorGetBufferAddressStability(
    const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteMmapRo:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteArenaRw:
      return kTfLiteRunStabilityUnstable;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStabilityUnstable;
    case kTfLiteDynamic:
      return kTfLiteRunStabilitySingleRun;
    case kTfLitePersistentRo:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteCustom:
      return kTfLiteRunStabilityUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStabilityAcrossRuns;
  }
  return kTfLiteRunStabilityUnknown;
}

// Returns how stable a tensor data values are across runs.
TfLiteRunStability TfLiteTensorGetDataStability(const TfLiteTensor* const t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteMmapRo:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteArenaRw:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStabilityAcrossRuns;
    case kTfLiteDynamic:
      return kTfLiteRunStabilitySingleRun;
    case kTfLitePersistentRo:
      return kTfLiteRunStabilitySingleRun;
    case kTfLiteCustom:
      return kTfLiteRunStabilityUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStabilitySingleRun;
  }
  return kTfLiteRunStabilityUnknown;
}

// Returns the operation step when the data of a tensor is populated.
//
// Some operations can precompute their results before the evaluation step. This
// makes the data available earlier for subsequent operations.
TfLiteRunStep TfLiteTensorGetDataKnownStep(const TfLiteTensor* t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStepInit;
    case kTfLiteMmapRo:
      return kTfLiteRunStepInit;
    case kTfLiteArenaRw:
      return kTfLiteRunStepEval;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStepEval;
    case kTfLiteDynamic:
      return kTfLiteRunStepEval;
    case kTfLitePersistentRo:
      return kTfLiteRunStepPrepare;
    case kTfLiteCustom:
      return kTfLiteRunStepUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStepEval;
  }
  return kTfLiteRunStepUnknown;
}

// Returns the operation steop when the shape of a tensor is computed.
//
// Some operations can precompute the shape of their results before the
// evaluation step. This makes the shape available earlier for subsequent
// operations.
TfLiteRunStep TfLiteTensorGetShapeKnownStep(const TfLiteTensor* t) {
  switch (t->allocation_type) {
    case kTfLiteMemNone:
      return kTfLiteRunStepInit;
    case kTfLiteMmapRo:
      return kTfLiteRunStepInit;
    case kTfLiteArenaRw:
      return kTfLiteRunStepPrepare;
    case kTfLiteArenaRwPersistent:
      return kTfLiteRunStepPrepare;
    case kTfLiteDynamic:
      return kTfLiteRunStepEval;
    case kTfLitePersistentRo:
      return kTfLiteRunStepPrepare;
    case kTfLiteCustom:
      return kTfLiteRunStepUnknown;
    case kTfLiteVariantObject:
      return kTfLiteRunStepEval;
  }
  return kTfLiteRunStepUnknown;
}

}  // extern "C"
