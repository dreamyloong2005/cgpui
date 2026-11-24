#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <iostream>
#include <string>
namespace {
std::string read(const char* name, unsigned cap) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / name);
  if (!file) throw std::runtime_error(std::string("missing initialization owner: ") + name);
  std::string text{std::istreambuf_iterator<char>(file), {}};
  if (std::count(text.begin(), text.end(), '\n') > cap) throw std::runtime_error("initialization owner grew broad");
  return text;
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
}
int main() {
  try {
    const auto selection = read("src/renderer/renderer_automatic_selection.cpp", 65);
    require(!has(selection, "windows.h") && !has(selection, "getenv") && has(selection, "same_surface_reusable") &&
        has(selection, "failure.error.code") && has(selection, "RendererBackend::vulkan"), "neutral classified selection boundary differs");
    const auto rollback = read("src/renderer/dx12/dx12_initialization_rollback.cpp", 100);
    require(has(rollback, "state.operation_depth != 1") && has(rollback, "state.submitted") &&
        has(rollback, "state.composition_host->committed") && has(rollback, "window.instance") &&
        !has(rollback, "PostMessage") && !has(rollback, "->Commit("), "rollback must prove local unsubmitted ownership");
    require(rollback.find("state.composition_host.reset()") < rollback.find("dx12_release_native_objects(state)") &&
        rollback.find("dx12_release_native_objects(state)") < rollback.find("win32_surface_alpha_release_message"),
        "uncommitted graph/native references must retire before checked alpha release");
    const auto attempt = read("src/renderer/dx12/dx12_initialization_attempt.cpp", 65);
    require(has(attempt, "dx12_rollback_initialization") && has(attempt, "if (!rollback_candidate)") &&
        has(attempt, "dx12_initialization_fallback_allowed()"), "failure result alone cannot authorize fallback");
    const auto initialization = read("src/renderer/dx12/dx12_initialization.cpp", 75);
    require(initialization.find("dx12_initialization_fallback_allowed()") < initialization.find("dx12_configure_surface(") &&
        initialization.find("dx12_enroll_recovery_work(state)") < initialization.find("dx12_create_device(*state"),
        "invalid diagnostic settings precede mutation and in-flight attachment has a managed endpoint");
    require(initialization.find("dx12_validate_surface_descriptor(descriptor)") < initialization.find("dx12_enable_debug()"),
        "invalid surfaces must not initialize process-wide diagnostics");
    const auto adapters = read("src/renderer/dx12/dx12_adapter_selection.cpp", 90);
    require(has(adapters, "dx12_classify_initialization_failure(result) != Dx12InitializationFailureClass::backend_unavailable"),
        "terminal adapter errors must not be overwritten by enumeration exhaustion");
    const auto device = read("src/renderer/dx12/dx12_adapter_device.cpp", 90);
    require(has(device, "const HRESULT removal = state.device->GetDeviceRemovedReason()") &&
        has(device, "return FAILED(result) ? result : DXGI_ERROR_UNSUPPORTED"),
        "shader-model capability classification must retain actual failures/removal");
    require(has(attempt, "dx12_wrap_initialized_renderer(state)") &&
        !has(attempt, "dx12_wrap_initialized_renderer(std::move(state))"),
        "operation guard must retain a strong state owner during throwing facade allocation");
    const auto recovery = read("src/renderer/dx12/dx12_recovery.cpp", 150);
    require(!has(recovery, "select_automatic_renderer") && !has(recovery, "attempt_renderer_initialization") &&
        !has(recovery, "create_vulkan_renderer"), "runtime recovery must not switch backend");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
