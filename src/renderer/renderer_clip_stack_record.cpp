#include "cgpui/renderer/renderer_types.hpp"

namespace cgpui {

bool operator==(
    const RendererClipStackRecord& lhs,
    const RendererClipStackRecord& rhs) {
  const auto same_rect = [](Rect left, Rect right) {
    return left.origin.x == right.origin.x && left.origin.y == right.origin.y &&
           left.size.width == right.size.width &&
           left.size.height == right.size.height;
  };
  const auto same_optional_rect =
      [&](const std::optional<Rect>& left, const std::optional<Rect>& right) {
        if (left.has_value() != right.has_value()) {
          return false;
        }
        return !left.has_value() || same_rect(*left, *right);
      };
  if (lhs.full_depth != rhs.full_depth || lhs.truncated != rhs.truncated ||
      !same_optional_rect(lhs.current_clip_rect, rhs.current_clip_rect) ||
      lhs.clips.size() != rhs.clips.size()) {
    return false;
  }
  for (std::size_t index = 0; index < lhs.clips.size(); ++index) {
    if (!same_rect(lhs.clips[index], rhs.clips[index])) {
      return false;
    }
  }
  return true;
}

bool RendererClipStackRecord::empty() const { return full_depth == 0; }

RendererClipStackRecord renderer_clip_stack_record(
    std::span<const Rect> clips) {
  RendererClipStackRecord record;
  record.full_depth = clips.size();
  if (clips.empty()) {
    return record;
  }

  record.current_clip_rect = clips.back();
  record.truncated = clips.size() > kRendererClipStackMaxDepth;
  const std::size_t first_kept =
      record.truncated ? clips.size() - kRendererClipStackMaxDepth : 0;
  record.clips.assign(clips.begin() + first_kept, clips.end());
  return record;
}

} // namespace cgpui
