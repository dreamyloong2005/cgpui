#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace cgpui {

// Unlike a target preference, unknown is a valid report from custom renderers.
enum class RendererBackend { unknown, vulkan, metal, dx12 };
enum class RendererBackendSelection { automatic, vulkan, dx12 };

struct RendererBackendInfo {
  RendererBackend backend = RendererBackend::unknown;
  std::string adapter_name;
  std::uint32_t vendor_id = 0;
  std::optional<std::uint64_t> adapter_luid;
  bool software_adapter = false;
  RendererBackendSelection selection = RendererBackendSelection::automatic;
  std::string fallback_reason;
  std::uint64_t device_generation = 0;
};

} // namespace cgpui
