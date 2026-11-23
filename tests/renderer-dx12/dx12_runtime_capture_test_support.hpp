#pragma once

#include "cgpui/renderer/renderer_frame.hpp"

namespace cgpui_test {

// External observations outlive the runtime-owned native renderer.
struct Dx12RuntimeCaptureCounts {
  void* hwnd = nullptr;
  unsigned begin_attempts = 0;
  unsigned begin_retries = 0;
  unsigned begin_pending = 0;
  unsigned begin_errors = 0;
  unsigned resize_attempts = 0;
  unsigned resize_retries = 0;
  unsigned capture_errors = 0;
};

cgpui::Result<std::unique_ptr<cgpui::Renderer>> dx12_runtime_capture_renderer(
    const cgpui::RenderSurfaceDescriptor& descriptor,
    Dx12RuntimeCaptureCounts& counts);

} // namespace cgpui_test
