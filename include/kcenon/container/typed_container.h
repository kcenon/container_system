// BSD 3-Clause License
//
// DEPRECATED: This header is deprecated. Use simd_batch.h instead.
// This file now redirects to simd_batch.h for backward compatibility.
// See Issue #328 for migration details.

#pragma once

#if defined(__GNUC__) || defined(__clang__)
    #pragma message("Warning: typed_container.h is deprecated. Use simd_batch.h instead. See Issue #328.")
#elif defined(_MSC_VER)
    #pragma message("Warning: typed_container.h is deprecated. Use simd_batch.h instead. See Issue #328.")
#endif

// Include the new header which provides the deprecated alias
#include "simd_batch.h"

// Note: typed_container is now a deprecated alias for simd_batch
// defined in simd_batch.h
