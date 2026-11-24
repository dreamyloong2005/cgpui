#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(std::string("missing resize owner: ") + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* text) { return source.find(text) != std::string::npos; }
void require(bool ok, const char* reason) { if (!ok) throw std::runtime_error(reason); }
bool before(const std::string& source, const char* first, const char* second) {
  const auto a = source.find(first), b = source.find(second);
  return a != std::string::npos && b != std::string::npos && a < b;
}
}
int main() {
  try {
    const auto header = read("src/ui/runtime_surface_resizes_internal.hpp");
    const auto owner = read("src/ui/runtime_surface_resizes.cpp");
    require(std::count(header.begin(), header.end(), '\n') <= 50 &&
        std::count(owner.begin(), owner.end(), '\n') <= 140,
        "resize coalescing must remain a focused private module");
    require(has(header, "class RuntimeSurfaceResizes") && !has(header, "Renderer*") &&
        !has(header, "PlatformWindow*") && !has(header, "HANDLE") && !has(header, "renderer.resize("),
        "resize owner must not retain native/renderer pointers or inline policy");
    require(has(owner, "pending->applying") && has(owner, "pending->cancelled") &&
        has(owner, "revision != pending->revision") && has(owner, "found->second == pending") &&
        has(owner, "resize violated begin-only frame_not_ready contract"),
        "nested desired extents, cancellation and illegal admission must stay explicit");
    for (const auto* file : {"src/ui/runtime_renderer_resize_results.cpp", "src/ui/runtime_event_windows.cpp"})
      require(has(read(file), "surface_resizes_.request("), "root and child must share the resize owner");
    for (const auto* file : {"src/ui/runtime_renderer_frame_results.cpp", "src/ui/runtime_window_frame_results.cpp"})
      require(before(read(file), "surface_resizes_.retry(", "render_view("),
          "pending resize must complete before any new frame render");
    const auto readiness = read("src/ui/runtime_frame_readiness.cpp");
    require(has(readiness, "ErrorCode::renderer_recovery_pending") &&
        has(readiness, "surface_resizes_.clear_all()"),
        "pending recovery must reuse delayed window-bound wake and cancellation");
    for (const auto* file : {"src/ui/runtime_window_records.cpp", "src/ui/runtime_window_shutdown.cpp",
        "src/ui/runtime_window_resource_cleanup.cpp"})
      require(has(read(file), "surface_resizes_.clear("), "window retirement must cancel pending resize");
    for (const auto* file : {"src/ui/runtime_renderer_frame_results.cpp", "src/ui/runtime_window_frame_results.cpp"})
      require(has(read(file), "surface_resizes_.has_pending("), "old-frame success must preserve a newer pending resize");
    require(has(read("include/cgpui/core/error.hpp"), "renderer_recovery_pending,") &&
        has(read("include/cgpui/renderer/renderer_frame.hpp"), "requested size/scale is not yet applied"),
        "unfinished recovery must have a distinct neutral public contract");
    const auto manifest = read("tests/ui/mcpp.toml");
    for (const auto* test : {"runtime_surface_resizes_test.cpp",
        "window_runtime_recovery_pending_frame_test.cpp", "window_runtime_recovery_pending_resize_test.cpp",
        "window_runtime_recovery_pending_lifetime_test.cpp"})
      require(has(manifest, test), "resize/recovery behavior test must be discovered");
    require(has(read("tests/architecture/mcpp.toml"), "runtime_surface_resizes_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "runtime_surface_resizes_structure_test|tests/architecture/runtime_surface_resizes_structure_test.cpp"),
        "resize structure guard must be registered in mcpp and CI");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
