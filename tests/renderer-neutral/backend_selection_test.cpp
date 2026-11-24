#include "../../src/renderer/renderer_backend_selection_internal.hpp"

#include <array>
#include <string_view>
#include <utility>

namespace {
int vulkan_calls = 0;
int dx12_calls = 0;
bool fail_vulkan = false;
bool fail_dx12 = false;

class FakeRenderer final : public cgpui::Renderer {
 public:
  explicit FakeRenderer(cgpui::RendererBackend backend) : backend_(backend) {}
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unexpected(cgpui::Error{
        cgpui::ErrorCode::frame_acquisition_failed, "fake frame"});
  }
  cgpui::RendererBackendInfo backend_info() const override {
    return {.backend = backend_, .adapter_name = "test adapter", .vendor_id = 42,
            .adapter_luid = 123, .software_adapter = true,
            .device_generation = 7};
  }
 private:
  cgpui::RendererBackend backend_;
};

cgpui::Result<std::unique_ptr<cgpui::Renderer>> vulkan_factory(
    const cgpui::RenderSurfaceDescriptor&) {
  ++vulkan_calls;
  if (fail_vulkan) return std::unexpected(cgpui::Error{
      cgpui::ErrorCode::renderer_initialization_failed, "Vulkan original error"});
  return std::make_unique<FakeRenderer>(cgpui::RendererBackend::vulkan);
}

cgpui::Result<std::unique_ptr<cgpui::Renderer>> dx12_factory(
    const cgpui::RenderSurfaceDescriptor&) {
  ++dx12_calls;
  if (fail_dx12) return std::unexpected(cgpui::Error{
      cgpui::ErrorCode::unsupported_platform, "DX12 original error"});
  return std::make_unique<FakeRenderer>(cgpui::RendererBackend::dx12);
}

cgpui::detail::RendererInitializationAttempt automatic_attempt(
    const cgpui::RenderSurfaceDescriptor& descriptor) {
  auto result = dx12_factory(descriptor);
  if (!result) return std::unexpected(cgpui::detail::RendererInitializationFailure{result.error()});
  return std::move(*result);
}

auto select(std::string_view preference) {
  return cgpui::detail::select_renderer_backend(
      {}, preference, vulkan_factory, dx12_factory, automatic_attempt);
}
} // namespace

int main() {
  using enum cgpui::RendererBackendSelection;
  for (const auto preference : {"", "auto", "vulkan", "dx12"}) {
    const auto parsed = cgpui::detail::parse_renderer_backend_selection(preference);
    const auto expected = std::string_view(preference) == "vulkan" ? vulkan
        : std::string_view(preference) == "dx12" ? dx12 : automatic;
    if (!parsed || *parsed != expected) return 1;
  }
  for (const auto preference : {"invalid", "DX12", " vulkan", "metal"}) {
    const auto result = select(preference);
    if (result || result.error().code != cgpui::ErrorCode::invalid_argument ||
        result.error().message.find("auto, vulkan, or dx12") == std::string::npos ||
        vulkan_calls != 0 || dx12_calls != 0) return 2;
  }
  for (const auto preference : {"", "auto", "vulkan", "dx12"}) {
    vulkan_calls = dx12_calls = 0;
    const auto result = select(preference);
    const bool explicit_dx12 = std::string_view(preference) == "dx12";
    const bool use_dx12 = std::string_view(preference) != "vulkan";
    if (!result || vulkan_calls != (use_dx12 ? 0 : 1) ||
        dx12_calls != (use_dx12 ? 1 : 0)) return 3;
    const auto info = (*result)->backend_info();
    const auto expected_selection = explicit_dx12 ? dx12
        : std::string_view(preference) == "vulkan" ? vulkan : automatic;
    if (info.backend != (use_dx12 ? cgpui::RendererBackend::dx12
                                    : cgpui::RendererBackend::vulkan) ||
        info.selection != expected_selection || info.adapter_name != "test adapter" ||
        info.vendor_id != 42 || info.adapter_luid != 123 || !info.software_adapter ||
        info.device_generation != 7 || !info.fallback_reason.empty()) return 4;
    const auto frame = (*result)->begin_frame();
    if (frame || frame.error().message != "fake frame" ||
        !(*result)->resize({}, {})) return 5;
  }
  fail_dx12 = fail_vulkan = true;
  for (const auto preference : {"", "auto", "vulkan", "dx12"}) {
    vulkan_calls = dx12_calls = 0;
    const auto result = select(preference);
    const bool use_dx12 = std::string_view(preference) != "vulkan";
    const auto expected_code = use_dx12 ? cgpui::ErrorCode::unsupported_platform
        : cgpui::ErrorCode::renderer_initialization_failed;
    const auto expected_message = use_dx12 ? "DX12 original error"
                                               : "Vulkan original error";
    if (result || result.error().code != expected_code ||
        result.error().message != expected_message ||
        vulkan_calls != (use_dx12 ? 0 : 1) ||
        dx12_calls != (use_dx12 ? 1 : 0)) return 6;
  }
  const auto missing = cgpui::detail::select_renderer_backend({}, "dx12", vulkan_factory, nullptr);
  if (missing || missing.error().code != cgpui::ErrorCode::renderer_initialization_failed)
    return 7;
  return 0;
}
