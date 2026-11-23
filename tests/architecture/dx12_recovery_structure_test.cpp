#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing recovery owner: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
std::string compact(std::string source) {
  std::erase_if(source, [](unsigned char value) { return std::isspace(value); });
  return source;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_recovery.cpp",
                             "src/renderer/dx12/dx12_recovery_internal.hpp",
                             "src/renderer/dx12/dx12_frame_recovery.cpp",
                             "src/renderer/dx12/dx12_frame_recovery_internal.hpp",
                             "src/renderer/dx12/dx12_resize_recovery.cpp",
                             "src/renderer/dx12/dx12_native_retirement.cpp",
                             "src/renderer/dx12/dx12_native_retirement_internal.hpp",
                             "src/renderer/dx12/dx12_device.cpp",
                             "src/renderer/dx12/dx12_device_internal.hpp",
                             "src/renderer/dx12/dx12_adapter_device.cpp",
                             "src/renderer/dx12/dx12_adapter_selection.cpp",
                             "src/renderer/dx12/dx12_texture_realization_internal.hpp",
                             "src/renderer/dx12/dx12_texture_reset.cpp"}) {
      const auto source = read(path);
      const auto limit = std::filesystem::path(path).extension() == ".cpp" ? 150 : 200;
      require(std::count(source.begin(), source.end(), '\n') <= limit,
              "recovery, native cleanup, adapter and texture owners must remain focused");
    }
    const auto recovery = compact(read("src/renderer/dx12/dx12_recovery.cpp"));
    require(has(recovery, "dx12_create_device_on_adapter(") &&
                has(recovery, "dx12_release_native_objects(") &&
                has(recovery, "dx12_create_pipeline(") && has(recovery, "dx12_create_swapchain(") &&
                has(recovery, "dx12_check_debug_errors("),
            "recovery must delegate native teardown and each complete reconstruction stage");
    require(!has(recovery, "while(") && !has(recovery, "for(") &&
                !has(recovery, "create_vulkan_renderer(") && !has(recovery, "getenv("),
            "recovery cannot retry in a loop, switch backend or reselect through the environment");
    require(has(recovery, "state.recovery_pending") &&
                has(recovery, "state.info.device_generation=generation+1") &&
                recovery.find("state.info.device_generation=generation+1") > recovery.find("rebuild(state,luid)"),
            "pending recovery must outlive failures and only a complete rebuild advances generation");

    const auto pinned = compact(read("src/renderer/dx12/dx12_adapter_device.cpp"));
    require(has(pinned, "EnumAdapterByLuid(") && !has(pinned, "getenv(") &&
                !has(pinned, "EnumWarpAdapter(") && !has(pinned, "EnumAdapters1("),
            "reconstruction must use only the original adapter LUID");
    const auto device = compact(read("src/renderer/dx12/dx12_device.cpp"));
    const auto pinned_start = device.find("Result<void>dx12_create_device_on_adapter(");
    require(pinned_start != std::string::npos, "pinned device initialization is required");
    const auto pinned_body = device.substr(pinned_start);
    require(has(pinned_body, "dx12_select_pinned_adapter(") &&
                !has(pinned_body, "dx12_select_initial_adapter("),
            "the pinned initializer cannot call the initial selection policy");

    const auto purge = compact(read("src/renderer/dx12/dx12_native_retirement.cpp"));
    require(has(purge, "dx12_reset_texture_resources(") && has(purge, "state.device.Reset()") &&
                !has(purge, "image_cache") && !has(purge, "text_cache") &&
                !has(purge, "state.info=") && !has(purge, "state.hwnd=") &&
                !has(purge, "state.width=") && !has(purge, "state.height=") &&
                !has(purge, "state.scale=") && !has(purge, "make_unique<"),
            "native cleanup must preserve CPU caches/window identity without allocating replacements");
    const auto texture = read("src/renderer/dx12/dx12_texture_internal.hpp");
    require(!has(texture, "Dx12Ptr<") && !has(texture, "ID3D12") && !has(texture, "uploaded"),
            "immutable CPU snapshots held by finished frames cannot own old-device resources");
    const auto realization = read("src/renderer/dx12/dx12_texture_realization_internal.hpp");
    require(has(realization, "Dx12NativeTexture") && has(realization, "weak_ptr"),
            "native texture realizations require separate ownership without retaining expired CPU snapshots");
    const auto renderer = read("src/renderer/dx12/dx12_renderer.cpp");
    require(has(renderer, "dx12_prepare_frame(") && has(renderer, "dx12_resize_recovering(") &&
                has(read("src/renderer/dx12/dx12_frame.cpp"), "dx12_handle_frame_failure("),
            "renderer/frame entries must delegate the bounded recovery policy to private leaves");

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "dx12_recovery_internal.hpp") &&
                  !has(source, "dx12_recover_removed_device") && !has(source, "Dx12NativeTexture"),
              "recovery and native realization boundaries must stay private");
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
