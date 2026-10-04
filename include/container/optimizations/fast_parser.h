/**
 * @file fast_parser.h
 * @brief Forwarding header to canonical fast_parser location.
 *
 * @deprecated Since v1.1.0. Use `<kcenon/container/optimizations/fast_parser.h>` instead.
 *             This forwarding header is scheduled for removal in the next minor release
 *             after v1.1.0. See CHANGELOG.md > Deprecations.
 */

#pragma once

// Deprecation notice: this header is a pure `#include` forwarder, so the C++
// `[[deprecated]]` attribute cannot attach to it. We use `#pragma message`
// (portable across GCC, Clang, and MSVC) to emit a build-time warning when
// downstream consumers include the legacy path.
#pragma message("warning: <container/optimizations/fast_parser.h> is deprecated; include <kcenon/container/optimizations/fast_parser.h> instead. Scheduled for removal in the next minor release after v1.1.0.")

// Forwarding header — canonical location: include/kcenon/container/optimizations/fast_parser.h
#include <kcenon/container/optimizations/fast_parser.h>
