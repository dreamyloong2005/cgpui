#include "cgpui/renderer/glyph_cache.hpp"

#include <utility>

namespace cgpui {

void GlyphCache::set_rasterization_error(Error error) {
  if (!rasterization_error_) {
    rasterization_error_ = std::move(error);
  }
}

void GlyphCache::clear_rasterization_error() {
  rasterization_error_.reset();
}

const std::optional<Error>& GlyphCache::rasterization_error() const {
  return rasterization_error_;
}

} // namespace cgpui
