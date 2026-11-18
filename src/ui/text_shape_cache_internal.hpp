#pragma once

#include "text_shaping_internal.hpp"

namespace cgpui {

// Returns the cached run for an identical request on this thread, or null.
// A hit performs no allocation; the pointer is valid until the next store.
[[nodiscard]] const TextShapeRun* find_cached_text_shape(
    const TextShapingRequest& request);

void store_cached_text_shape(
    const TextShapingRequest& request,
    const TextShapeRun& run);

} // namespace cgpui
