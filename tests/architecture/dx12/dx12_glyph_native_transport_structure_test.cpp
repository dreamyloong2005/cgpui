#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::string& name) {
  std::ifstream file(root / name);
  if (!file) throw std::runtime_error("missing glyph native owner: " + name);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
bool before(const std::string& s, const char* a, const char* b) {
  const auto x = s.find(a), y = s.find(b);
  return x != std::string::npos && y != std::string::npos && x < y;
}
std::string owner(const char* file) { return read(std::string("src/renderer/dx12/") + file); }
}
int main() {
  try {
    for (const auto* file : {"dx12_glyph_native_internal.hpp", "dx12_glyph_native.cpp",
        "dx12_glyph_command_dependencies_internal.hpp", "dx12_glyph_command_dependencies.cpp",
        "dx12_glyph_recording_scope_internal.hpp", "dx12_glyph_recording_scope.cpp",
        "dx12_glyph_command_api_internal.hpp", "dx12_glyph_command_api.cpp",
        "dx12_glyph_native_recording_internal.hpp", "dx12_glyph_native_recording.cpp",
        "dx12_glyph_target_preparation.cpp", "dx12_texture_native_pruning.cpp",
        "dx12_texture_region_upload_internal.hpp", "dx12_texture_region_upload.cpp",
        "dx12_texture_copy_observer_internal.hpp", "dx12_texture_copy_observer.cpp"}) {
      const auto s = owner(file);
      require(std::count(s.begin(), s.end(), '\n') <= 180, "native glyph leaf exceeded focused budget");
      require(!has(s, "#include \"vulkan") && !has(s, "WaitForSingleObject("), "glyph modules cannot own fallback/pacing");
    }
    const auto deps = owner("dx12_glyph_command_dependencies.cpp");
    require(has(deps, "std::vector<std::unique_ptr<Pin>>") && has(deps, "Map::node_type") &&
        has(deps, "native_textures.extract(expired)") && has(deps, "owner->identity = nullptr") &&
        !has(deps, "insert(std::move") && !has(deps, "Dx12TextureHandle"), "dependencies must transport sole node lease without CPU pins/reinsertion");
    const auto native = owner("dx12_glyph_native.cpp");
    const auto complete = native.substr(native.find("void Dx12GlyphNative::commit_completed()"),
        native.find("void Dx12GlyphNative::abandon_unexecuted()") - native.find("void Dx12GlyphNative::commit_completed()"));
    require(has(complete, "candidate.prefix > slot.prefix") && !has(complete, ".clear()") &&
        !has(complete, "parent_reset_or_release()"), "G commits maximum prefix but cannot retire dependencies");
    require(!has(native, "Dx12SrvAllocation") && !has(native, "Dx12TextureHandle"), "resource-only catalog cannot own CPU snapshots or SRV leases");
    const auto prepare = owner("dx12_texture_resources.cpp");
    require(before(prepare, "renderer.pending_uploads.reserve(", "dx12_record_glyph_target(") &&
        before(prepare, "dx12_stage_glyph_target(", "dx12_record_glyph_target(") &&
        has(prepare, "if (glyph) renderer.glyph_native->pin(native)"), "pending marker/all staging and exact-hit pins precede target references");
    const auto presentation = owner("dx12_presentation.cpp");
    const auto submission = owner("dx12_frame_submission.cpp");
    require(before(presentation, "Dx12GlyphRecordingScope recording(state)", "dx12_begin_texture_frame(state)") &&
        before(presentation, "dx12_begin_texture_frame(state)", "recording.reset_and_begin()") &&
        before(presentation, "image_acquisition)", "recording.reset_and_begin()") &&
        before(presentation, "recording.reset_and_begin()", "auto& target =") &&
        has(presentation, "if (auto prepared = recording.reset_and_begin(); !prepared) return prepared;") &&
        before(submission, "ExecuteCommandLists(", "recording.executed()") &&
        before(presentation, "dx12_execute_frame(state, recording, target)", "RendererFrameTimingStage::queue_submission") &&
        !has(presentation, "commands.reset_allocator(") && !has(presentation, "commands.reset_list("),
        "presentation keeps scope/Execute lifetime and calls focused preparation before target commands");
    const auto reset = owner("dx12_texture_reset.cpp");
    require(before(reset, "glyph_native->reset_after_parent_release()", "renderer.native_textures.clear()"),
            "glyph node/slot owner must retire before ordinary map/pool");
    const auto scope = owner("dx12_glyph_recording_scope.cpp");
    const auto begin_at = scope.find("Result<void> Dx12GlyphRecordingScope::reset_and_begin()");
    const auto open_at = scope.find("void Dx12GlyphRecordingScope::opened_after_reset()");
    require(begin_at != std::string::npos && open_at != std::string::npos && begin_at < open_at,
            "native recording preparation needs its focused scope owner");
    const auto begin = scope.substr(begin_at, open_at - begin_at);
    require(before(begin, "commands.reset_allocator(", "commands.reset_list(") &&
        before(begin, "if (FAILED(result)) return std::unexpected(dx12_error(result, \"allocator reset\"));", "commands.reset_list(") &&
        before(begin, "if (FAILED(result)) return std::unexpected(dx12_error(result, \"command list reset\"));", "opened_after_reset()") &&
        before(begin, "opened_after_reset()", "glyph_native->begin_recording(") &&
        before(begin, "glyph_native->begin_recording(", "dx12_begin_texture_upload_frame(state_)") &&
        before(begin, "dx12_begin_texture_upload_frame(state_)", "dx12_observe_glyph_lifetime(state_)") &&
        has(begin, "if (auto prepared = dx12_begin_texture_upload_frame(state_); !prepared) return prepared;"),
        "Reset failures retain owners; successful P opens the guard before fallible metadata and pool rewind");
    const auto opened = scope.substr(open_at, scope.find("HRESULT Dx12GlyphRecordingScope::close()") - open_at);
    require(before(opened, "open_ = true", "glyph_native->parent_reset()") &&
        has(scope, "if (attempted_close_) return cleanup_close_") && has(scope, "abandon_unexecuted()"),
        "error/exception guard must mark open before preparation, preserve dependencies and avoid repeated Close");
    const auto region = owner("dx12_texture_region_upload.cpp");
    require(has(region, "upload_region(*state.device.Get(), view)") && !has(region, "CreateCommittedResource(") &&
        !has(region, "std::vector<std::uint8_t>"), "region views must reuse existing pool without extra alpha copy");
    require(has(owner("dx12_texture_upload_internal.hpp"), "struct Dx12TextureSourceView {") &&
        !has(owner("dx12_texture_region_upload_internal.hpp"), "struct Dx12TextureSourceView") &&
        !has(owner("dx12_texture_upload.cpp"), "dx12_texture_region_upload_internal.hpp"),
        "generic source view must belong to upload owner, not reverse-depend on glyph adapter");
    require(has(owner("dx12_texture_upload_frame.cpp"), "dx12_validate_texture_upload_state(state)") &&
        has(region, "dx12_validate_texture_upload_state(state)") && !has(region, "state.submitted"),
        "full/region uploads must share one private live-idle gate");
    const auto copies = owner("dx12_glyph_native_recording.cpp");
    require(has(copies, "CopyResource(") && has(copies, "CopyTextureRegion(") &&
        has(copies, "observer->record("), "native copy observation must follow real forwarding operations");
    const auto registry = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* test : {"dx12_glyph_dirty_transport_test.cpp", "dx12_glyph_cold_order_test.cpp",
        "dx12_glyph_node_lifetime_test.cpp", "dx12_glyph_recording_scope_test.cpp",
        "dx12_glyph_unknown_completion_test.cpp", "dx12_glyph_native_recovery_test.cpp",
        "dx12_glyph_region_staging_test.cpp", "dx12_glyph_pending_failure_test.cpp",
        "dx12_glyph_later_region_failure_test.cpp", "dx12_glyph_fallback_sampling_test.cpp",
        "dx12_glyph_observer_capacity_test.cpp", "dx12_glyph_reset_close_failure_test.cpp",
        "dx12_srv_heap_sharing_test.cpp", "dx12_srv_recovery_test.cpp"})
      require(has(registry, test), "glyph/native SRV registry entry missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
