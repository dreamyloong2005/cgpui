#pragma once

#include <cstddef>

namespace cgpui {

// Natively shaped runs are cached per thread, keyed by text, font
// descriptor, size, scale and shaping options, with least-recently-used
// eviction. The deterministic fallback is not cached.
struct TextShapeCacheStatistics {
  std::size_t entry_count = 0;
  std::size_t capacity = 0;
  std::size_t lookup_count = 0;
  std::size_t hit_count = 0;
  std::size_t miss_count = 0;
  std::size_t eviction_count = 0;
};

[[nodiscard]] TextShapeCacheStatistics text_shape_cache_statistics();

void set_text_shape_cache_capacity(std::size_t capacity);

void clear_text_shape_cache();

} // namespace cgpui
