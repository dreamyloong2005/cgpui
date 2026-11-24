#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing glyph fallback control: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
bool before(const std::string& s, const char* a, const char* b) {
  const auto x = s.find(a), y = s.find(b);
  return x != std::string::npos && y != std::string::npos && x < y;
}
}
int main() {
  try {
    for (const auto* name : {"dx12_glyph_fallback_controls_test_support.hpp",
        "dx12_glyph_fallback_history_test_support.cpp", "dx12_glyph_fallback_submit_test_support.cpp",
        "dx12_glyph_missing_lineage_test.cpp", "dx12_glyph_family_fallback_test.cpp",
        "dx12_glyph_invalid_seal_test.cpp"}) {
      const auto source = read(std::string("tests/renderer-dx12/") + name);
      require(std::count(source.begin(), source.end(), '\n') <= 150, "fallback controls must remain focused leaves");
      for (const auto* forbidden : {"const_cast", "#define private", "friend class", "friend struct",
          "CopyResource(", "CopyTextureRegion(", "observer.record(", "observer->record("}) {
        require(!has(source, forbidden), "fallback fixture cannot fabricate ownership/access or native copy observations");
      }
    }
    const auto submit = read("tests/renderer-dx12/dx12_glyph_fallback_submit_test_support.cpp");
    require(has(submit, "dx12_prepare_glyph_target(state, snapshot, native, inputs)") &&
        has(submit, "dx12_stage_glyph_target(state, *snapshot, *prepared)") &&
        has(submit, "dx12_record_glyph_target(state, native, *prepared)") &&
        before(submit, "state.commands->Reset(", "scope.opened_after_reset()") &&
        before(submit, "ExecuteCommandLists(1, lists)", "scope.executed()") &&
        has(submit, "dx12_wait_idle(state)") &&
        submit.rfind("dx12_wait_idle(state)") > submit.find("scope.executed()") &&
        before(submit, "dx12_commit_texture_uploads(state)", "dx12_check_debug_errors(state)"),
        "private fallback boundary must forward actual prepare/copy/Execute/completion/debug flow");
    const auto history = read("tests/renderer-dx12/dx12_glyph_fallback_history_test_support.cpp");
    require(has(history, "dx12_validate_glyph_record(") && has(history, "seal_snapshot(page, snapshot)") &&
        has(history, "std::make_shared<const cgpui::Dx12Texture>") &&
        before(history, "glyph_readback(state, resource)", "dx12_check_debug_errors(state)"),
        "synthetic history must use real validation/seals and guarded shared full-page readback");
    const auto readback = read("tests/renderer-dx12/dx12_glyph_readback_test_support.cpp");
    require(before(readback, "job->buffer->Unmap(", "dx12_check_debug_errors(state)"),
        "validated shared readback revision is a required dependency, not a fixture-local replacement");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_glyph_fallback_history_test_support.cpp",
        "dx12_glyph_fallback_submit_test_support.cpp", "dx12_glyph_missing_lineage_test.cpp",
        "dx12_glyph_family_fallback_test.cpp", "dx12_glyph_invalid_seal_test.cpp",
        "dx12_glyph_readback_debug_gate_test.cpp", "dx12_glyph_dirty_transport_test.cpp",
        "dx12_glyph_dense_fallback_test.cpp", "dx12_glyph_lineage_test.cpp", "dx12_srv_pool_allocation_test.cpp"})
      require(has(manifest, name), "new controls and old validation/oracles must remain registered");
    require(has(read("tests/architecture/mcpp.toml"), "dx12/*_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_glyph_fallback_controls_structure_test|"),
        "fallback structure requires existing mcpp discovery plus explicit CI row");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
