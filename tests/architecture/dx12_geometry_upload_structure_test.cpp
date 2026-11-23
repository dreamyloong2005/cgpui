#include <algorithm>
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
  if (!file) throw std::runtime_error("missing geometry upload owner: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool before(const std::string& source, const char* first, const char* second) {
  const auto left = source.find(first), right = source.find(second);
  return left != std::string::npos && right != std::string::npos && left < right;
}
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_geometry_upload_internal.hpp",
                             "src/renderer/dx12/dx12_geometry_upload.cpp",
                             "src/renderer/dx12/dx12_geometry_upload_frame_internal.hpp",
                             "src/renderer/dx12/dx12_geometry_upload_frame.cpp"}) {
      const auto source = read(path);
      const auto limit = std::filesystem::path(path).extension() == ".cpp" ? 280 : 200;
      require(std::count(source.begin(), source.end(), '\n') <= limit,
              "geometry upload allocation and lifetime require focused private leaves");
      require(!has(source, "#include \"vulkan") && !has(source, "#include \"../vulkan") &&
                  !has(source, "CopyTextureRegion(") && !has(source, "DrawInstanced(") &&
                  !has(source, "DrawIndexedInstanced("),
              "geometry upload storage cannot absorb another backend, texture copies or draw recording");
    }
    const auto header = read("src/renderer/dx12/dx12_geometry_upload_internal.hpp");
    require(has(header, "class Dx12GeometryUploads") &&
                has(header, "Dx12UploadPages pages_") &&
                !has(header, "CreateCommittedResource(") && !has(header, "std::memcpy("),
            "the private geometry owner declares storage without inline native allocation/copy bodies");
    const auto adapter = read("src/renderer/dx12/dx12_geometry_upload.cpp");
    require(has(adapter, "pages_.allocate(device, size, 16)") &&
                has(adapter, "std::numeric_limits<UINT>::max()") && !has(adapter, "dx12_create_buffer(") &&
                !has(adapter, "->Map(") && !has(adapter, "->Unmap("),
            "geometry keeps 16-byte/UINT view policy while delegating native page ownership");
    for (const auto* path : {"src/renderer/dx12/dx12_indexed_draw.cpp",
                             "src/renderer/dx12/dx12_textured_draw.cpp",
                             "src/renderer/dx12/dx12_backdrop_blur_composite.cpp"}) {
      const auto source = read(path);
      require(has(source, "dx12_geometry_upload_frame_internal.hpp") &&
                  has(source, "dx12_upload_geometry(") && has(source, "->address") && has(source, "->size") &&
                  !has(source, "CreateCommittedResource(") && !has(source, "->Map(") &&
                  !has(source, "dx12_create_buffer("),
              "native geometry recorders must consume upload slices, not allocate/map per-draw buffers");
    }
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::unique_ptr<Dx12GeometryUploads> geometry_uploads") &&
                !has(state, "Dx12GeometryUploads::") && !has(state, "std::memcpy("),
            "state owns the private arena without absorbing its allocation or copy policy");
    const auto lifecycle = read("src/renderer/dx12/dx12_geometry_upload_frame.cpp");
    require(has(lifecycle, "state.submitted") && has(lifecycle, "state.failed") &&
                has(lifecycle, "state.recovery_pending") && has(lifecycle, "state.removal_reason") &&
                has(lifecycle, "!state.device") &&
                before(lifecycle, "if (auto valid = writable(state); !valid) return valid;",
                    "state.geometry_uploads->begin_frame()"),
            "geometry lifecycle must reject pending, failed and retired-device states before reuse");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    require(before(presentation, "if (auto idle = dx12_wait_idle(state); !idle) return idle;",
                   "dx12_begin_geometry_upload_frame(state)"),
            "presentation must prove GPU completion before rewinding geometry slices");
    require(before(read("src/renderer/dx12/dx12_native_retirement.cpp"),
                   "state.geometry_uploads->clear()", "state.device.Reset()"),
            "native retirement must release geometry pages before the old device");
    for (const auto& entry : std::filesystem::directory_iterator(root / "src/renderer/dx12")) {
      if (!entry.is_regular_file()) continue;
      const auto extension = entry.path().extension();
      if (extension != ".hpp" && extension != ".cpp") continue;
      require(!has(read(entry.path()), "dx12_upload_buffer("),
              "the replaced per-draw committed upload helper cannot survive as an unused alternate path");
    }
    for (const auto* path : {"src/renderer/dx12/dx12_texture_resources.cpp",
                             "src/renderer/dx12/dx12_capture.cpp"}) {
      const auto source = read(path);
      require(!has(source, "dx12_geometry_upload_internal.hpp") && !has(source, "geometry_uploads"),
              "texture staging and capture/readback remain outside the geometry-only arena");
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "dx12_geometry_upload_internal.hpp") && !has(source, "Dx12GeometryUploads"),
              "geometry arena allocation policy must not leak into public headers");
    }
    require(has(read("tests/architecture/mcpp.toml"), "dx12_geometry_upload_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_geometry_upload_structure_test|tests/architecture/dx12_geometry_upload_structure_test.cpp") &&
                has(read("tests/renderer-dx12/mcpp.toml"), "dx12_geometry_upload_budget_test.cpp"),
            "geometry upload native and structural checks need both mcpp and CI matrix registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
