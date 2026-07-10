#pragma once

#include "cgpui/renderer/renderer_frame_diagnostics.hpp"

#include <cstddef>
#include <span>
#include <vector>

namespace cgpui {

struct RendererFrameResource {
  RendererPrimitiveKind primitive_kind = RendererPrimitiveKind::solid_rect;
  std::size_t command_index = 0;
  std::size_t resource_index = 0;

  friend bool operator==(
      const RendererFrameResource&,
      const RendererFrameResource&) = default;
};

enum class RendererDroppedResourceReason {
  unsupported_primitive,
  missing_submission_resource,
};

struct RendererDroppedResourceDiagnostic {
  RendererFrameResource resource;
  RendererDroppedResourceReason reason =
      RendererDroppedResourceReason::missing_submission_resource;

  friend bool operator==(
      const RendererDroppedResourceDiagnostic&,
      const RendererDroppedResourceDiagnostic&) = default;
};

struct RendererDroppedResourceDiagnostics {
  std::vector<RendererDroppedResourceDiagnostic> resources;
  RendererDrawCounts counts;
  std::size_t unsupported_primitive_count = 0;
  std::size_t missing_submission_resource_count = 0;

  [[nodiscard]] bool empty() const;
};

[[nodiscard]] RendererDroppedResourceDiagnostics
classify_renderer_dropped_resources(
    std::span<const RendererFrameResource> planned_resources,
    std::span<const RendererFrameResource> submitted_resources,
    std::span<const RendererPrimitiveKind> supported_primitive_kinds);

} // namespace cgpui
