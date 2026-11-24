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
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
}
int main() {
  try {
    for (const auto* test : {"dx12_command_reuse_slot_retirement_test.cpp", "dx12_command_reuse_transient_overwrite_test.cpp",
        "dx12_command_reuse_late_publication_test.cpp", "dx12_command_reuse_cohort_orphan_test.cpp"}) {
      const auto source = read(std::string("tests/renderer-dx12/") + test);
      require(std::count(source.begin(), source.end(), '\n') <= 150, "supplement owner grew broad");
      for (const auto* fake : {"submitted = false", "failed = false", "can_release_entry", "publish_success(", "parent_reset_succeeded("})
        require(!has(source, fake), "native supplement cannot manufacture lifecycle proof or public success");
      require(has(read("tests/renderer-dx12/mcpp.toml"), test), "supplement missing additive test discovery");
    }
    const auto slot = read("tests/renderer-dx12/dx12_command_reuse_slot_retirement_test.cpp");
    require(has(slot, "ReuseFailListReset failure") && has(slot, "weak[0].expired()") &&
        has(slot, "srv_pool->allocate(") && has(slot, "reused != slots"), "slot control must check real lease identity across failure and actual reset");
    require(has(slot, "dx12_error(E_ACCESSDENIED, \"command list reset\")") &&
        has(slot, "denied.error().code != expected.code") && has(slot, "denied.error().message != expected.message") &&
        has(slot, "failure.allocator_calls != 1") && has(slot, "FAILED(failure.allocator_result)") &&
        has(slot, "failure.list_calls != 1") && has(slot, "failure.list_result != E_ACCESSDENIED") &&
        has(slot, "failure.close_calls != 0") && has(slot, "Dx12CommandEventKind::parent_reset) != resets") &&
        has(slot, "Dx12CommandEventKind::producer_execute) != executes"), "failed-Reset control must prove exact injected failure without P or submission");
    const auto support = read("tests/renderer-dx12/dx12_command_reuse_supplement_test_support.cpp");
    require(has(support, "++allocator_calls;") && has(support, "allocator_result = dx12_native_glyph_command_api().reset_allocator(allocator)") &&
        has(support, "++list_calls;") && has(support, "++close_calls;"), "Reset seam must retain actual forwarded HRESULT and call provenance");
    const auto poison = read("tests/renderer-dx12/dx12_command_reuse_transient_overwrite_test.cpp");
    require(has(poison, "dx12_upload_geometry(") && has(poison, "slice->resource->Map(") &&
        has(poison, "transient_ranges") && has(poison, "reuse_native_history(") && has(poison, "reuse_trace_pixels_ready("),
        "frozen-storage control requires actual address/byte overwrite AND same bundle/native pixels");
    const auto late = read("tests/renderer-dx12/dx12_command_reuse_late_publication_test.cpp");
    require(has(late, "AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR") && has(late, "only_sentinel(") &&
        has(late, "creates + 1") && has(late, "diagnostic_same("), "late failure must use actual debug rejection and next native fresh recording");
    const auto cohort = read("tests/renderer-dx12/dx12_command_reuse_cohort_orphan_test.cpp");
    require(has(cohort, "dx12_remove_native_device(before)") && has(cohort, "reuse_native_sentinel(") &&
        has(cohort, "peer->owner_alive = false") && has(cohort, "reuse_parent_before_bundle_release(") &&
        has(cohort, "dx12_recover_removed_device(") && has(cohort, "reuse_native_history("),
        "cohort control needs actual removal, release sentinels, orphan non-rebuild and new-epoch reuse");
    require(has(read("scripts/ci/architecture-header-targets.txt"), "dx12_command_reuse_supplement_structure_test|"),
        "supplement architecture registry missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
