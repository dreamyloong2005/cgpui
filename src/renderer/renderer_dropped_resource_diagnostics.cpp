#include "cgpui/renderer/renderer_dropped_resource_diagnostics.hpp"

#include <algorithm>

namespace cgpui {
namespace {

bool renderer_primitive_is_supported(
    RendererPrimitiveKind primitive_kind,
    std::span<const RendererPrimitiveKind> supported_primitive_kinds) {
  return std::ranges::find(supported_primitive_kinds, primitive_kind) !=
         supported_primitive_kinds.end();
}

} // namespace

bool RendererDroppedResourceDiagnostics::empty() const {
  return resources.empty();
}

RendererDroppedResourceDiagnostics classify_renderer_dropped_resources(
    std::span<const RendererFrameResource> planned_resources,
    std::span<const RendererFrameResource> submitted_resources,
    std::span<const RendererPrimitiveKind> supported_primitive_kinds) {
  RendererDroppedResourceDiagnostics diagnostics;
  diagnostics.resources.reserve(planned_resources.size());
  std::size_t submitted_index = 0;
  for (const RendererFrameResource planned_resource : planned_resources) {
    const auto remaining = submitted_resources.subspan(submitted_index);
    const auto match = std::ranges::find(remaining, planned_resource);
    if (match != remaining.end()) {
      submitted_index +=
          static_cast<std::size_t>(match - remaining.begin()) + 1;
      continue;
    }

    const bool supported = renderer_primitive_is_supported(
        planned_resource.primitive_kind,
        supported_primitive_kinds);
    const RendererDroppedResourceReason reason =
        supported
            ? RendererDroppedResourceReason::missing_submission_resource
            : RendererDroppedResourceReason::unsupported_primitive;
    diagnostics.resources.push_back(RendererDroppedResourceDiagnostic{
        .resource = planned_resource,
        .reason = reason,
    });
    renderer_add_draw_count(diagnostics.counts, planned_resource.primitive_kind);
    if (supported) {
      ++diagnostics.missing_submission_resource_count;
    } else {
      ++diagnostics.unsupported_primitive_count;
    }
  }
  return diagnostics;
}

} // namespace cgpui
