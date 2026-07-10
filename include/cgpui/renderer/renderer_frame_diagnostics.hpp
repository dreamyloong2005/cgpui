#pragma once

#include <cstddef>

namespace cgpui {

struct RendererFrameWork {
  std::size_t batch_count = 0;
  std::size_t command_count = 0;

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

  [[nodiscard]] bool exact_match() const;
};

[[nodiscard]] RendererFrameDiagnostics compare_renderer_frame_work(
    RendererFrameWork planned_work,
    RendererFrameWork submitted_work);

} // namespace cgpui
