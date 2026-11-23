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
  if (!file) throw std::runtime_error("missing texture upload owner: " + path.string());
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
    for (const auto* path : {"src/renderer/dx12/dx12_upload_pages_internal.hpp",
                             "src/renderer/dx12/dx12_upload_pages.cpp",
                             "src/renderer/dx12/dx12_texture_upload_internal.hpp",
                             "src/renderer/dx12/dx12_texture_upload.cpp",
                             "src/renderer/dx12/dx12_texture_upload_frame_internal.hpp",
                             "src/renderer/dx12/dx12_texture_upload_frame.cpp"}) {
      const auto source = read(path);
      const auto limit = std::filesystem::path(path).extension() == ".cpp" ? 280 : 200;
      require(std::count(source.begin(), source.end(), '\n') <= limit,
              "shared pages, texture footprints and lifecycle adapters need focused private owners");
      require(!has(source, "#include \"vulkan") && !has(source, "#include \"../vulkan"),
              "DX12 upload modules cannot depend on Vulkan internals");
    }
    const auto pages = read("src/renderer/dx12/dx12_upload_pages.cpp");
    require(has(pages, "Dx12UploadPages::allocate(") && has(pages, "->Map(") && has(pages, "->Unmap("),
            "the shared native page owner must own allocation and balanced persistent mapping");
    require(!has(pages, "std::memcpy(") && !has(pages, "GetCopyableFootprints(") &&
                !has(pages, "CopyTextureRegion(") && !has(pages, "DrawInstanced(") &&
                !has(pages, "DrawIndexedInstanced(") && !has(pages, "DXGI_FORMAT_R8"),
            "shared pages cannot absorb payload copy, texture format or native recording policy");
    const auto geometry = read("src/renderer/dx12/dx12_geometry_upload_internal.hpp");
    const auto texture = read("src/renderer/dx12/dx12_texture_upload_internal.hpp");
    require(has(geometry, "Dx12UploadPages pages_") && has(texture, "Dx12UploadPages pages_") &&
                !has(texture, "CreateCommittedResource(") && !has(texture, "std::memcpy("),
            "geometry and texture adapters must own separate instances of the shared page implementation");
    const auto adapter = read("src/renderer/dx12/dx12_texture_upload.cpp");
    require(has(adapter, "GetCopyableFootprints(") && has(adapter, "pages_.allocate(") &&
                has(adapter, "D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT") &&
                has(adapter, "D3D12_TEXTURE_DATA_PITCH_ALIGNMENT") &&
                has(adapter, "footprint.Offset += slice->offset") &&
                has(adapter, "std::memcpy(") && has(adapter, "RowPitch") &&
                !has(adapter, "dx12_create_buffer(") && !has(adapter, "->Map(") &&
                !has(adapter, "CopyTextureRegion("),
            "the texture adapter owns footprints and row copies but delegates page mapping and GPU recording");
    const auto recording = read("src/renderer/dx12/dx12_texture_resources.cpp");
    require(has(recording, "dx12_upload_texture(") && has(recording, "CopyTextureRegion(") &&
                has(recording, "payload_bytes") && !has(recording, "GetCopyableFootprints(") &&
                !has(recording, "dx12_create_buffer(") && !has(recording, "->Map(") &&
                !has(recording, "std::memcpy("),
            "texture realization must record a prepared slice and actual payload accounting, not staging allocation");
    const auto lifecycle = read("src/renderer/dx12/dx12_texture_upload_frame.cpp");
    require(has(lifecycle, "state.submitted") && has(lifecycle, "state.failed") &&
                has(lifecycle, "state.recovery_pending") && has(lifecycle, "state.removal_reason") &&
                has(lifecycle, "!state.device") &&
                before(lifecycle, "if (auto valid = writable(state); !valid) return valid;",
                       "state.texture_uploads->begin_frame()"),
            "texture staging lifecycle must reject unknown completion or retired-device states before mutation");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    const auto retirement = read("src/renderer/dx12/dx12_native_retirement.cpp");
    for (const auto* begin : {"dx12_begin_geometry_upload_frame(state)", "dx12_begin_texture_upload_frame(state)"})
      require(before(presentation, "if (auto idle = dx12_wait_idle(state); !idle) return idle;", begin),
              "both independent upload pools require completion proof before rewind");
    for (const auto* clear : {"state.geometry_uploads->clear()", "state.texture_uploads->clear()"})
      require(before(retirement, clear, "state.device.Reset()"),
              "both upload pools must release old-device pages before native device teardown");
    const auto state = read("src/renderer/dx12/dx12_state_internal.hpp");
    require(has(state, "std::unique_ptr<Dx12TextureUploads> texture_uploads") &&
                !has(state, "std::vector<Dx12Ptr<ID3D12Resource>> uploads"),
            "state must own the focused texture pool instead of an unstructured staging-resource list");
    for (const auto& entry : std::filesystem::directory_iterator(root / "src/renderer/dx12")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".cpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "state.uploads") && !has(source, "state->uploads") && !has(source, "state_->uploads"),
              "obsolete staging-resource ownership cannot survive alongside pooled uploads");
    }
    require(!has(read("src/renderer/dx12/dx12_capture.cpp"), "texture_uploads") &&
                !has(read("src/renderer/dx12/dx12_capture.cpp"), "Dx12UploadPages"),
            "capture/readback stays outside the upload-page pool");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "dx12_upload_pages_internal.hpp") && !has(source, "Dx12UploadPages") &&
                  !has(source, "dx12_texture_upload_internal.hpp") && !has(source, "Dx12TextureUploads"),
              "private upload storage must not leak into the public renderer API");
    }
    require(has(read("tests/architecture/mcpp.toml"), "dx12_texture_upload_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "dx12_texture_upload_structure_test|tests/architecture/dx12_texture_upload_structure_test.cpp") &&
                has(read("tests/renderer-dx12/mcpp.toml"), "dx12_texture_upload_budget_test.cpp"),
            "texture upload behavior and module checks need mcpp and CI-matrix registration");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
