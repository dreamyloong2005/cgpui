#pragma once

#include "cgpui/renderer/glyph_uploads.hpp"
#include "cgpui/renderer/image_uploads.hpp"

#include <cstddef>
#include <span>

namespace cgpui {

struct RendererFrameWork {
  std::size_t batch_count = 0;
  std::size_t command_count = 0;
  std::size_t upload_byte_count = 0;

  friend bool operator==(
      const RendererFrameWork&,
      const RendererFrameWork&) = default;
};

struct RendererFrameDiagnostics {
  RendererFrameWork planned_work;
  RendererFrameWork submitted_work;
  std::size_t pending_batch_count = 0;
  std::size_t unexpected_batch_count = 0;
  std::size_t pending_command_count = 0;
  std::size_t unexpected_command_count = 0;
  std::size_t pending_upload_byte_count = 0;
  std::size_t unexpected_upload_byte_count = 0;

  [[nodiscard]] bool exact_match() const;
};

[[nodiscard]] RendererFrameDiagnostics compare_renderer_frame_work(
    RendererFrameWork planned_work,
    RendererFrameWork submitted_work);

struct RendererUploadByteCounts {
  std::size_t glyph_atlas_byte_count = 0;
  std::size_t image_byte_count = 0;
  std::size_t total_byte_count = 0;
  bool saturated = false;
};

[[nodiscard]] RendererUploadByteCounts renderer_upload_byte_counts(
    std::span<const GlyphAtlasUploadBatch> glyph_uploads,
    std::span<const ImageUploadBatch> image_uploads);

} // namespace cgpui
