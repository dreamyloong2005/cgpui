#include "../../src/renderer/renderer_backend_selection_internal.hpp"
#include "../../src/renderer/renderer_automatic_selection_internal.hpp"
#include <stdexcept>
#include <iostream>
#include <string>

using namespace cgpui;
using namespace cgpui::detail;
namespace {
std::string calls;
Error failure{ErrorCode::renderer_initialization_failed, "DX12 failure"};
bool reusable = true, dx_success = false, vk_failure = false, null_dx = false, null_vk = false;
RendererBackend dx_backend = RendererBackend::dx12, vk_backend = RendererBackend::vulkan;
class Fake final : public Renderer {
 public:
  explicit Fake(RendererBackend value) : backend(value) {}
  RendererBackend backend;
  std::uint64_t generation = 7;
  Result<void> resize(Size, DpiScale) override {
    ++generation;
    return std::unexpected(Error{ErrorCode::renderer_recovery_pending, "pending resize"});
  }
  Result<std::unique_ptr<RenderFrame>> begin_frame() override {
    return std::unexpected(Error{ErrorCode::renderer_initialization_failed, "late runtime loss"});
  }
  RendererBackendInfo backend_info() const override {
    return {.backend = backend, .adapter_name = "actual adapter", .vendor_id = 91,
        .adapter_luid = 123, .software_adapter = true, .device_generation = generation};
  }
};
Result<std::unique_ptr<Renderer>> vulkan(const RenderSurfaceDescriptor&) {
  calls += 'v';
  if (vk_failure) return std::unexpected(Error{ErrorCode::unsupported_platform, "Vulkan failure"});
  if (null_vk) return std::unique_ptr<Renderer>{};
  return std::make_unique<Fake>(vk_backend);
}
Result<std::unique_ptr<Renderer>> explicit_dx12(const RenderSurfaceDescriptor&) {
  calls += 'e'; return std::unexpected(failure);
}
RendererInitializationAttempt attempt(const RenderSurfaceDescriptor&) {
  calls += 'd';
  if (null_dx) return std::unique_ptr<Renderer>{};
  if (dx_success) return std::make_unique<Fake>(dx_backend);
  return std::unexpected(RendererInitializationFailure{failure, reusable});
}
auto select(const char* preference = "auto") {
  calls.clear();
  return select_renderer_backend({}, preference, vulkan, explicit_dx12, attempt);
}
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
}
int main() {
  try {
    auto fallback = select();
    require(fallback && calls == "dv", "automatic fallback was not ordered or successful");
    auto info = (*fallback)->backend_info();
    require(info.backend == RendererBackend::vulkan && info.selection == RendererBackendSelection::automatic &&
        info.fallback_reason.find("DX12 failure") != std::string::npos && info.adapter_luid == 123 &&
        info.adapter_name == "actual adapter" && info.vendor_id == 91 && info.software_adapter &&
        info.device_generation == 7, "fallback metadata invented or lost backend facts");
    auto resized = (*fallback)->resize({}, {});
    auto frame = (*fallback)->begin_frame();
    require(!resized && resized.error().code == ErrorCode::renderer_recovery_pending && !frame &&
        frame.error().message == "late runtime loss" && calls == "dv" &&
        (*fallback)->backend_info().device_generation == 8, "runtime error retried a backend or cached metadata");
    dx_success = true;
    auto preferred = select("");
    require(preferred && calls == "d" && (*preferred)->backend_info().backend == RendererBackend::dx12 &&
        (*preferred)->backend_info().fallback_reason.empty(), "successful automatic DX12 fell back");
    dx_success = false; reusable = false;
    require(!select() && calls == "d", "unproven cleanup allowed fallback");
    reusable = true;
    for (auto code : {ErrorCode::invalid_argument, ErrorCode::frame_retry_required,
         ErrorCode::frame_not_ready, ErrorCode::renderer_recovery_pending, ErrorCode::frame_acquisition_failed}) {
      failure = {code, "must remain fatal"};
      auto result = select();
      require(!result && result.error().code == code && calls == "d", "invalid/runtime error fell back");
    }
    failure = {ErrorCode::renderer_initialization_failed, "DX12 failure"};
    require(!select("dx12") && calls == "e", "explicit DX12 used automatic policy");
    require(select("vulkan") && calls == "v", "explicit Vulkan attempted DX12");
    require(!select("bad") && calls.empty(), "invalid preference reached any factory");
    vk_failure = true;
    auto both = select();
    require(!both && calls == "dv" && both.error().code == ErrorCode::unsupported_platform &&
        both.error().message.find("DX12 failure") != std::string::npos &&
        both.error().message.find("Vulkan failure") != std::string::npos, "dual failure lost one error");
    vk_failure = false; null_dx = true;
    require(!select() && calls == "d", "empty DX12 success authorized fallback");
    null_dx = false; null_vk = true;
    require(!select() && calls == "dv", "empty Vulkan renderer accepted");
    null_vk = false; dx_success = true; dx_backend = RendererBackend::unknown;
    require(!select() && calls == "d", "preferred factory backend identity was fabricated");
    dx_success = false; vk_backend = RendererBackend::dx12;
    require(!select() && calls == "dv", "fallback backend identity was fabricated");
    calls.clear();
    require(!select_automatic_renderer({}, vulkan, nullptr) && calls.empty(), "missing attempt invoked fallback");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
