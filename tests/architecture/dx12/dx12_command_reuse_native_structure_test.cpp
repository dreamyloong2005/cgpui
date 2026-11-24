#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(path);
  return {std::istreambuf_iterator<char>(file), {}};
}
std::string owner(const char* name) { return read(std::string("src/renderer/dx12/") + name); }
bool has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }
bool before(const std::string& s, const char* a, const char* b) {
  return has(s, a) && has(s, b) && s.find(a) < s.find(b);
}
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
}
int main() {
  try {
    for (const auto* file : {"dx12_command_reuse_signature.cpp", "dx12_command_reuse_selection.cpp",
        "dx12_command_bundle.cpp", "dx12_command_bundle_geometry.cpp", "dx12_frame_command_reuse.cpp",
        "dx12_command_reuse_proofs.cpp", "dx12_frame_submission.cpp", "dx12_frame_draw_body.cpp"}) {
      const auto source = owner(file);
      require(std::count(source.begin(), source.end(), '\n') <= 180, "reuse owner grew broad");
      require(!has(source, "vulkan") && !has(source, "WaitForSingleObject("), "reuse added backend/wait policy");
    }
    for (const auto* file : {"dx12_command_reuse_signature.cpp", "dx12_command_reuse_selection.cpp"}) {
      const auto source = owner(file);
      for (const auto* forbidden : {"CreateCommittedResource(", "dx12_prepare_texture(", "srv_pool->allocate", "try_emplace(", "CopyTextureRegion("})
        require(!has(source, forbidden), "read-ready signature/selection mutated native resources");
    }
    const auto bundle = owner("dx12_command_bundle.cpp");
    const auto begin = bundle.find("Result<void> Dx12CommandBundle::record(");
    const auto end = bundle.find("const Dx12ReuseScene& Dx12CommandBundle::scene()");
    require(begin != std::string::npos && end > begin, "focused bundle record boundary missing");
    const auto recording = bundle.substr(begin, end - begin);
    for (const auto* forbidden : {"RSSetScissorRects(", "RSSetViewports(", "ResourceBarrier(", "CopyTextureRegion(", "OMSetRenderTargets(", "ClearRenderTargetView(", "->Reset("})
      require(!has(recording, forbidden), "immutable BUNDLE contains forbidden/re-recording operation");
    require(has(recording, "D3D12_COMMAND_LIST_TYPE_BUNDLE") && has(recording, "impl_->list->Close()") &&
        before(bundle, "impl_->list.Reset()", "impl_->geometry.reset()") &&
        before(bundle, "impl_->geometry.reset()", "impl_->scene.pins.clear()"), "bundle native/storage/slot-pin ownership order missing");
    const auto scope = owner("dx12_glyph_recording_scope.cpp");
    require(before(scope, "open_ = true", "command_reuse->parent_reset(state_)") &&
        before(scope, "opened_after_reset()", "glyph_native->begin_recording("), "glyph successful P/open-before-allocation contract changed");
    const auto sync = owner("dx12_synchronization.cpp");
    require(before(sync, "if (completed < state.pending_fence_value) return {};", "command_reuse->completed(state)"),
        "reuse G must come from actual completed ticket, not event/false-submitted shortcut");
    const auto retire = owner("dx12_native_retirement.cpp");
    require(before(retire, "FAILED(state.removal_reason)", "state.submitted = false") &&
        before(retire, "state.commands.Reset()", "command_reuse->parent_released(state)") &&
        before(retire, "command_reuse->parent_released(state)", "dx12_reset_texture_resources(state)"), "cohort/removal/parent-first native cleanup broken");
    const auto submit = owner("dx12_frame_submission.cpp");
    require(before(submit, "command_reuse->prearm(state)", "ExecuteCommandLists(1, lists)") &&
        before(submit, "ExecuteCommandLists(1, lists)", "recording.executed()"), "prearm/real Execute ownership gap");
    const auto present = owner("dx12_presentation.cpp");
    require(before(present, "diagnostics.complete_successful_frame()", "command_reuse->publish_success(state)") &&
        before(present, "state.pixels = std::move(candidate_pixels)", "command_reuse->publish_success(state)"), "reuse published before original late public gates");
    require(before(owner("dx12_swapchain_resize.cpp"), "command_reuse->accept_surface(", "state.scale = scale") &&
        before(owner("dx12_frame.cpp"), "images_.has_mutations()", "images_.prepare(*state_, draws_)"), "scale-only or drained-mutation identity lost");
    require(has(owner("dx12_command_bundle_geometry_internal.hpp"), "Dx12GeometryUploads storage_") &&
        !has(owner("dx12_command_bundle_geometry.cpp"), "begin_frame("), "frozen geometry borrows/rewinds frame arena");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_glyph_unknown_completion_test.cpp", "dx12_glyph_reset_close_failure_test.cpp",
        "dx12_command_reuse_native_test.cpp", "dx12_command_reuse_native_failure_test.cpp",
        "dx12_command_reuse_unknown_completion_test.cpp", "dx12_command_reuse_scale_invalidation_test.cpp"})
      require(has(manifest, name), "new reuse gates must not replace accepted glyph gates");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
