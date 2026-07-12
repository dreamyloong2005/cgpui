#pragma once

#include "cgpui/ui/element_animation_sequence.hpp"

#include <string>
#include <unordered_map>

namespace cgpui {
namespace detail {

struct ScopedElementAnimationKey {
  std::uint64_t scope_id = 0;
  std::string value;

  friend bool operator==(
      const ScopedElementAnimationKey&,
      const ScopedElementAnimationKey&) = default;
};

struct ScopedElementAnimationKeyHash {
  std::size_t operator()(const ScopedElementAnimationKey& key) const;
};

struct ElementAnimationRecord {
  AnimationOptions options;
  std::uint64_t started_ms = 0;
  std::uint64_t seen_generation = 0;
  std::size_t stage_index = 0;
  std::uint64_t iteration = 0;
  bool repeating = false;
  bool complete = false;
};

struct ElementAnimationFrame {
  std::uint64_t now_ms = 0;
  std::uint64_t generation = 0;
};

[[nodiscard]] std::uint64_t normalized_animation_tick_interval(
    AnimationOptions options);

} // namespace detail

class ElementAnimationStateStore::Impl {
 public:
  std::unordered_map<
      detail::ScopedElementAnimationKey,
      detail::ElementAnimationRecord,
      detail::ScopedElementAnimationKeyHash>
      records;
  std::unordered_map<std::uint64_t, detail::ElementAnimationFrame> frames;
  std::uint64_t active_scope_id = 0;
};

} // namespace cgpui
