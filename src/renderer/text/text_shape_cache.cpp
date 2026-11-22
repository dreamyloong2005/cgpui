#include "cgpui/renderer/text/text_shape_cache.hpp"

#include "text_shape_cache_internal.hpp"

#include <bit>
#include <cstdint>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace cgpui {
namespace {

struct ShapeKeyView {
  std::string_view text;
  const FontDescriptor* font = nullptr;
  float font_size = 0.0F;
  float scale = 1.0F;
  TextShapingBackend backend = TextShapingBackend::deterministic_fallback;
  TextShapingDirection direction = TextShapingDirection::left_to_right;
  TextShapingScript script = TextShapingScript::common;
  std::string_view language;
  float tab_width = 4.0F;
};

struct ShapeKey {
  std::string text;
  FontDescriptor font;
  float font_size = 0.0F;
  float scale = 1.0F;
  TextShapingBackend backend = TextShapingBackend::deterministic_fallback;
  TextShapingDirection direction = TextShapingDirection::left_to_right;
  TextShapingScript script = TextShapingScript::common;
  std::string language;
  float tab_width = 4.0F;

  [[nodiscard]] ShapeKeyView view() const {
    return ShapeKeyView{text, &font, font_size, scale, backend, direction,
                        script, language, tab_width};
  }
};

ShapeKeyView key_view(const TextShapingRequest& request) {
  return ShapeKeyView{
      .text = request.text,
      .font = &request.font,
      .font_size = request.font_size,
      .scale = normalized_scale(request.scale),
      .backend = request.backend.used,
      .direction = request.direction,
      .script = request.script,
      .language = request.language,
      .tab_width = request.tab_width_in_spaces,
  };
}

void mix(std::size_t& seed, std::size_t value) {
  seed ^= value + 0x9E3779B97F4A7C15ULL + (seed << 6U) + (seed >> 2U);
}

struct ShapeKeyHash {
  using is_transparent = void;

  std::size_t operator()(const ShapeKeyView& key) const {
    std::size_t seed = std::hash<std::string_view>{}(key.text);
    mix(seed, std::hash<std::string>{}(key.font->family));
    mix(seed, key.font->weight);
    mix(seed, static_cast<std::size_t>(key.font->style));
    for (const FontFeature& feature : key.font->features) {
      mix(seed, std::bit_cast<std::uint32_t>(feature.tag));
      mix(seed, feature.value);
    }
    mix(seed, std::bit_cast<std::uint32_t>(key.font_size));
    mix(seed, std::bit_cast<std::uint32_t>(key.scale));
    mix(seed, static_cast<std::size_t>(key.backend));
    mix(seed, static_cast<std::size_t>(key.direction));
    mix(seed, static_cast<std::size_t>(key.script));
    mix(seed, std::hash<std::string_view>{}(key.language));
    mix(seed, std::bit_cast<std::uint32_t>(key.tab_width));
    return seed;
  }
  std::size_t operator()(const ShapeKey& key) const {
    return (*this)(key.view());
  }
};

struct ShapeKeyEqual {
  using is_transparent = void;

  static bool equal(const ShapeKeyView& a, const ShapeKeyView& b) {
    return a.text == b.text && *a.font == *b.font &&
        a.font_size == b.font_size && a.scale == b.scale &&
        a.backend == b.backend && a.direction == b.direction &&
        a.script == b.script && a.language == b.language &&
        a.tab_width == b.tab_width;
  }
  template <typename A, typename B>
  bool operator()(const A& a, const B& b) const {
    return equal(as_view(a), as_view(b));
  }
  static ShapeKeyView as_view(const ShapeKeyView& key) { return key; }
  static ShapeKeyView as_view(const ShapeKey& key) { return key.view(); }
};

struct Entry {
  ShapeKey key;
  TextShapeRun run;
};

class ShapeCache {
 public:
  const TextShapeRun* find(const ShapeKeyView& key) {
    stats_.lookup_count += 1;
    const auto found = index_.find(key);
    if (found == index_.end()) {
      stats_.miss_count += 1;
      return nullptr;
    }
    stats_.hit_count += 1;
    entries_.splice(entries_.begin(), entries_, found->second);
    return &found->second->run;
  }

  void store(ShapeKey key, const TextShapeRun& run) {
    if (capacity_ == 0 || index_.contains(key.view())) {
      return;
    }
    entries_.push_front(Entry{.key = std::move(key), .run = run});
    index_.emplace(entries_.front().key, entries_.begin());
    trim();
  }

  void set_capacity(std::size_t capacity) {
    capacity_ = capacity;
    trim();
  }

  void clear() {
    index_.clear();
    entries_.clear();
    stats_ = {};
  }

  TextShapeCacheStatistics statistics() const {
    TextShapeCacheStatistics result = stats_;
    result.entry_count = entries_.size();
    result.capacity = capacity_;
    return result;
  }

 private:
  void trim() {
    while (entries_.size() > capacity_) {
      index_.erase(entries_.back().key);
      entries_.pop_back();
      stats_.eviction_count += 1;
    }
  }

  std::list<Entry> entries_;
  std::unordered_map<ShapeKey, std::list<Entry>::iterator, ShapeKeyHash,
                     ShapeKeyEqual>
      index_;
  std::size_t capacity_ = 2048;
  TextShapeCacheStatistics stats_;
};

ShapeCache& thread_shape_cache() {
  thread_local ShapeCache cache;
  return cache;
}

} // namespace

const TextShapeRun* find_cached_text_shape(const TextShapingRequest& request) {
  return thread_shape_cache().find(key_view(request));
}

void store_cached_text_shape(
    const TextShapingRequest& request,
    const TextShapeRun& run) {
  const ShapeKeyView view = key_view(request);
  thread_shape_cache().store(
      ShapeKey{
          .text = std::string(view.text),
          .font = request.font,
          .font_size = view.font_size,
          .scale = view.scale,
          .backend = view.backend,
          .direction = view.direction,
          .script = view.script,
          .language = std::string(view.language),
          .tab_width = view.tab_width,
      },
      run);
}

TextShapeCacheStatistics text_shape_cache_statistics() {
  return thread_shape_cache().statistics();
}

void set_text_shape_cache_capacity(std::size_t capacity) {
  thread_shape_cache().set_capacity(capacity);
}

void clear_text_shape_cache() {
  thread_shape_cache().clear();
}

} // namespace cgpui
