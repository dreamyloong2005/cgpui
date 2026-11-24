#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error(std::string("missing image batch owner: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& source, const char* token) { return source.find(token) != std::string::npos; }
bool before(const std::string& source, const char* first, const char* second) {
  const auto a = source.find(first), b = source.find(second);
  return a != std::string::npos && b != std::string::npos && a < b;
}
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, unsigned maximum) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= maximum, "image batch owner exceeded focused cap");
}
}
int main() {
  try {
    const auto header = read("src/renderer/dx12/dx12_image_batch_internal.hpp");
    const auto request = read("src/renderer/dx12/dx12_image_request_internal.hpp");
    const auto queue = read("src/renderer/dx12/dx12_image_batch.cpp");
    const auto frame = read("src/renderer/dx12/dx12_frame.cpp");
    const auto recording = read("src/renderer/dx12/dx12_image_frame.cpp");
    const auto prepare = read("src/renderer/dx12/dx12_image_prepare.cpp");
    const auto submit = read("src/renderer/dx12/dx12_presentation.cpp");
    require(has(header, "std::vector<ImageAsset> uploads_;") && has(header, "std::vector<ImageAssetId> invalidations_;") &&
        !has(header, "return ") && has(request, "ImageDraw image;") && !has(request, "ID3D12") &&
        has(read("src/renderer/dx12/dx12_frame_internal.hpp"), "Dx12ImageBatch images_;"),
        "pending requests need frame-local owned values in focused declaration-only leaves");
    require(has(queue, "ImageAsset owned = image;") && has(queue, "pending.id == owned.id") &&
        has(queue, "*previous = std::move(owned)") && has(queue, "std::ranges::find(invalidations_, id)") &&
        !has(queue, "image_cache") && !has(queue, "state.") && !has(queue, "invalid_argument"),
        "recording copies/deduplicates final requests, not cache mutations or intermediate validation failures");
    require(has(recording, "images_.upload(image)") && has(recording, "images_.invalidate(id)") &&
        has(recording, "Dx12PendingImageDraw{renderer_to_device_pixels(image, state_->scale)}") &&
        has(recording, "RendererPrimitiveKind::image, command_count_++") && !has(recording, "image_cache"),
        "image recording preserves original slots/device scale without immediate cache resolution");
    require(before(frame, "if (error_)", "images_.prepare(*state_, draws_)") &&
        before(frame, "dx12_prepare_present(*state_, generation_)", "images_.prepare(*state_, draws_)") &&
        before(frame, "images_.prepare(*state_, draws_)", "dx12_submit_frame(") &&
        has(frame, "if (auto prepared = images_.prepare(*state_, draws_); !prepared) return prepared;"),
        "input/native preflight precedes CPU prefix, whose input errors do not poison or rebuild a live device");
    require(has(prepare, "uploads.swap(uploads_)") && has(prepare, "invalidations.swap(invalidations_)") &&
        before(prepare, "state.image_cache->invalidate(id)", "state.image_cache->upload(state, image)") &&
        before(prepare, "state.image_cache->upload(state, image)", "dx12_resolve_image_draw(state, *pending)") &&
        has(prepare, "command.payload = std::move(*resolved)") && !has(prepare, "command_index =") &&
        !has(prepare, "finish_frame") && !has(prepare, "CreateCommittedResource"),
        "focused prefix owns local raw bytes through exits and resolves payloads without reordering/aging/native work");
    const auto resolve = read("src/renderer/dx12/dx12_image_resolve.cpp");
    require(has(resolve, "state.image_cache->find(device.asset)") && has(resolve, "dx12_finite_textured_draw(draw)") &&
        !has(resolve, "renderer_to_device_pixels") &&
        before(submit, "dx12_require_resolved_images(draws)", "Dx12FrameDiagnostics diagnostics(") &&
        has(read("src/renderer/dx12/dx12_draw_dispatch.cpp"), "operator()(const Dx12PendingImageDraw&)"),
        "unresolved markers cannot be silently skipped/planned/submitted and resolved geometry is validated once");
    const auto completion = read("src/renderer/dx12/dx12_frame_completion.cpp");
    require(before(completion, "prepare_publication();", "image_cache->finish_frame();") &&
        before(completion, "image_cache->finish_frame();", "retained_image_payload();") &&
        has(read("src/renderer/dx12/dx12_image_cache.cpp"), "retention_.track(stored->second.texture)"),
        "immutable installation retains existing weak census and exactly-once post-success sampling");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* test : {"dx12_image_batch_test.cpp", "dx12_image_batch_validation_test.cpp", "dx12_image_batch_orphan_test.cpp"})
      require(has(manifest, test), "register focused batch CPU/native/orphan regressions");
    const auto shared = read("tests/renderer/mcpp.toml");
    for (const auto* test : {"renderer_image_batch_last_upload_test.cpp", "renderer_image_batch_invalidation_prefix_test.cpp",
                              "renderer_image_batch_abandonment_test.cpp"})
      require(has(shared, test), "register exact shared image contract regressions");
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_image_batch_structure_test|tests/architecture/dx12/dx12_image_batch_structure_test.cpp") &&
        has(read("tests/architecture/mcpp.toml"), "\"dx12/*_structure_test.cpp\""), "register batch structure boundary");
    for (const auto& entry : std::filesystem::recursive_directory_iterator(std::filesystem::path(CGPUI_SOURCE_ROOT) / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      std::ifstream input(entry.path()); const std::string source{std::istreambuf_iterator<char>(input), {}};
      require(!has(source, "Dx12ImageBatch") && !has(source, "Dx12PendingImageDraw"), "private requests leaked into public API");
    }
    limit("src/renderer/dx12/dx12_image_batch_internal.hpp", 35);
    limit("src/renderer/dx12/dx12_image_request_internal.hpp", 25);
    limit("src/renderer/dx12/dx12_image_resolve_internal.hpp", 25);
    limit("src/renderer/dx12/dx12_image_batch.cpp", 80);
    limit("src/renderer/dx12/dx12_image_prepare.cpp", 100);
    limit("src/renderer/dx12/dx12_image_resolve.cpp", 80);
    limit("src/renderer/dx12/dx12_image_frame.cpp", 60);
    limit("src/renderer/dx12/dx12_frame.cpp", 100);
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    limit("tests/renderer-dx12/dx12_image_batch_test.cpp", 120);
    limit("tests/renderer-dx12/dx12_image_batch_validation_test.cpp", 100);
    limit("tests/renderer-dx12/dx12_image_batch_orphan_test.cpp", 100);
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
