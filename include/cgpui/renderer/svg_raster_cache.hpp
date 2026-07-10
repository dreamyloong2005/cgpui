#pragma once

#include "cgpui/renderer/svg_rasterization.hpp"

#include <cstddef>
#include <deque>
#include <string>

namespace cgpui {

struct SvgRasterCacheKey {
  ImageAssetId asset_id;
  Size logical_size;
  float scale = 1.0F;
  std::string svg_source;

  [[nodiscard]] bool matches(
      const SvgRasterizationRequest& request) const;
};

struct SvgRasterCacheLookup {
  SvgRasterizationStatus status = SvgRasterizationStatus::invalid_request;
  const SvgRasterizationResult* result = nullptr;
  bool cache_hit = false;

  [[nodiscard]] bool ready() const;
};

class SvgRasterCache {
 public:
  [[nodiscard]] SvgRasterCacheLookup rasterize(
      const SvgRasterizationRequest& request);

  [[nodiscard]] std::size_t entry_count() const;
  [[nodiscard]] std::size_t lookup_count() const;
  [[nodiscard]] std::size_t hit_count() const;
  [[nodiscard]] std::size_t miss_count() const;

  void clear();

 private:
  struct Entry {
    SvgRasterCacheKey key;
    SvgRasterizationResult result;
  };

  std::deque<Entry> entries_;
  std::size_t lookup_count_ = 0;
  std::size_t hit_count_ = 0;
  std::size_t miss_count_ = 0;
};

} // namespace cgpui
