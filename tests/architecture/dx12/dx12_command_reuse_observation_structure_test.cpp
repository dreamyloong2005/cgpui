#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }
bool before(const std::string& s, const char* a, const char* b) {
  return has(s, a) && has(s, b) && s.find(a) < s.find(b);
}
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
}
int main() {
  try {
    const auto emit = read("src/renderer/dx12/dx12_native_draw_recording.cpp");
    for (const auto* forbidden : {"RSSetScissorRects(", "RSSetViewports(", "ResourceBarrier(", "CopyTextureRegion(",
        "ClearRenderTargetView(", "OMSetRenderTargets(", "dx12_upload_", "dx12_prepare_texture(", "state.commands"})
      require(!has(emit, forbidden), "shared legal emitter absorbed resource/shell preparation");
    require(has(emit, "list.DrawIndexedInstanced(") && has(emit, "list.DrawInstanced(") &&
        has(emit, "list.SetGraphicsRootDescriptorTable(1, draw.srv)") &&
        has(emit, "list.SetGraphicsRootDescriptorTable(2, draw.sampler)"), "native emission/actual table forwarding missing");
    for (const auto* file : {"src/renderer/dx12/dx12_indexed_draw.cpp", "src/renderer/dx12/dx12_textured_draw.cpp"}) {
      const auto source = read(file);
      require(has(source, "dx12_emit_prepared_draw(state, *state.commands.Get(), prepared)") &&
          !has(source, "DrawInstanced(") && !has(source, "DrawIndexedInstanced("), "eligible DIRECT calls bypass shared native observation");
    }
    const auto submit = read("src/renderer/dx12/dx12_frame_submission.cpp");
    require(before(submit, "prepare_tail(state, target)", "ExecuteCommandLists(1, lists)") &&
        before(submit, "ExecuteCommandLists(1, lists)", "recording.executed()") &&
        before(submit, "recording.executed()", "producer_execute") &&
        before(submit, "recording.executed()", "ExecuteCommandLists(1, &tail)"), "two-call tail crossed immediate producer ownership marking");
    const auto tail = read("tests/renderer-dx12/dx12_command_reuse_queue_tail_test_support.cpp");
    require(has(tail, "submission_pending()") && has(tail, "target_ = &target") &&
        has(tail, "commands_->CopyTextureRegion(") && has(tail, "buffer_->Map(") &&
        !has(tail, "state.pixels") && !has(tail, "state.readback") && !has(tail, "ExecuteCommandLists("),
        "owned verifier must copy exact target through production forwarding, not replace public history");
    const auto observation = read("src/renderer/dx12/dx12_command_observation.cpp");
    require(has(observation, "event.list->GetType()") && has(observation, "impl_->used == impl_->capacity") &&
        has(observation, "verifier->submitted_ = true") && !has(observation, "state.submitted ="),
        "bounded actual-list observation cannot claim producer ownership or discard unknown work");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_command_reuse_native_test.cpp"), "native oracle discovery missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
