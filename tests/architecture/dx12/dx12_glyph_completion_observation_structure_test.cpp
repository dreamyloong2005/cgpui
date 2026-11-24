#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::string& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing glyph completion leaf: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
std::string owner(const char* file) { return read(std::string("src/renderer/dx12/") + file); }
}
int main() {
  try {
    for (const auto* leaf : {"dx12_glyph_work_observer_internal.hpp", "dx12_glyph_work_observer.cpp",
        "dx12_glyph_native.cpp", "dx12_glyph_target_preparation.cpp", "dx12_glyph_native_recording.cpp"}) {
      const auto source = owner(leaf);
      require(std::count(source.begin(), source.end(), '\n') <= 180, "completion leaf exceeds focused budget");
    }
    const auto planner = owner("dx12_glyph_upload_plan.cpp");
    require(has(planner, "const auto* planned_alpha = batch.alpha.data()") &&
        has(planner, "std::move(batch), source.alpha.data(), planned_alpha") &&
        has(planner, "history.subspan(index.global_index, 1)"), "planner witness must use actual single-record buffers");
    const auto work = owner("dx12_glyph_work_observer.cpp");
    require(has(work, "renderer.native_textures") && has(work, "observe_owned_textures(*observer)") &&
        has(work, "state.texture_uploads->stats()") && has(work, "state.srv_pool->stats().live_slots") &&
        has(work, "textures_[i] == resource") && !has(work, "std::vector") &&
        !has(work, "new ") && !has(work, "std::function"), "bounded observation must enumerate actual owners/stats without callbacks/allocation");
    const auto dependencies = owner("dx12_glyph_command_dependencies.cpp");
    const auto native = owner("dx12_glyph_native.cpp");
    require(has(dependencies, "pin->detached.mapped().resource.Get()") &&
        has(dependencies, "impl_->resources") && has(native, "candidate.resource.Get()") &&
        has(native, "slot.resource.Get()"), "scoped ownership union must include detached nodes/catalog/candidates/dependencies");
    const auto prepare = owner("dx12_glyph_target_preparation.cpp");
    require(has(prepare, "cache->glyphs.upload_records()") && has(prepare, "const Dx12GlyphTargetInputs& inputs") &&
        has(prepare, "inputs.lineage->find_snapshot(texture)") && has(prepare, "inputs.history, inputs.pages") &&
        has(prepare, "plan->dirty_bytes < plan->full_bytes"), "private defensive control must share actual production dispatch");
    const auto public_cache = read("include/cgpui/renderer/glyph_cache.hpp");
    require(!has(public_cache, "GlyphCacheTestAccess"), "defensive native fixture cannot add a public-header friend hook");
    const auto fixture = read("tests/renderer-dx12/dx12_glyph_defensive_history_test_support.cpp");
    require(!has(fixture, "const_cast") && has(fixture, "dx12_validate_glyph_record(") &&
        has(fixture, "lineage.seal_snapshot(") && has(fixture, "history.inputs()") &&
        has(fixture, "ExecuteCommandLists(") && has(fixture, "dx12_wait_idle(state)"),
        "synthetic coherent history needs real shared validation/dispatch and native completion");
    require(has(fixture, "return dx12_check_debug_errors(state)") &&
        has(read("tests/renderer-dx12/dx12_glyph_dense_fallback_test.cpp"), "dx12_check_debug_errors(*state)") &&
        has(read("tests/renderer-dx12/dx12_glyph_exact_pin_failure_test.cpp"), "dx12_check_debug_errors(*state)"),
        "raw-submit and never-executed Reset/Close controls must have final debug acceptance gates");
    const auto registry = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* test : {"dx12_glyph_exact_pin_failure_test.cpp", "dx12_glyph_dense_fallback_test.cpp",
                            "dx12_glyph_forward_work_ledger_test.cpp", "dx12_glyph_dirty_transport_test.cpp"})
      require(has(registry, test), "completion test must add to, not replace, frozen native coverage");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
