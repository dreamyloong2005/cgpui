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
  if (!file) throw std::runtime_error("missing SRV pool owner: " + path.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
bool before(const std::string& s, const char* a, const char* b) {
  const auto x = s.find(a), y = s.find(b);
  return x != std::string::npos && y != std::string::npos && x < y;
}
void cap(const char* name, std::size_t limit) {
  const auto s = read(std::string("src/renderer/dx12/") + name);
  require(std::count(s.begin(), s.end(), '\n') <= limit, "SRV private leaves exceeded focused budgets");
  require(!has(s, "vulkan") && !has(s, "DrawInstanced(") && !has(s, "CopyTextureRegion("),
          "SRV pool cannot absorb backend fallback, drawing or upload policy");
}
}
int main() {
  try {
    cap("dx12_srv_descriptor_pool_internal.hpp", 120); cap("dx12_srv_descriptor_pool.cpp", 220);
    cap("dx12_srv_descriptor_page_internal.hpp", 100); cap("dx12_srv_descriptor_page.cpp", 160);
    cap("dx12_srv_heap_api_internal.hpp", 60); cap("dx12_srv_heap_api.cpp", 60);
    const auto pool = read("src/renderer/dx12/dx12_srv_descriptor_pool.cpp");
    const auto page = read("src/renderer/dx12/dx12_srv_descriptor_page.cpp");
    require(has(pool, "page->available() >= count") && has(pool, "Dx12SrvAllocation(std::move(*page), count)") &&
        !has(pool, "begin_frame") && !has(pool, "memcpy") && !has(pool, "CreateCommittedResource("),
        "group allocation must use one stable page without bulk rewind or texture ownership");
    require(has(page, "free_(capacity)") && has(page, "free_[available_++] = slots[index]") &&
        !has(page, "free_.push_back") && has(page, "numeric_limits<SIZE_T>") && has(page, "numeric_limits<UINT64>"),
        "release must use upfront storage and native handle arithmetic must stay checked");
    const auto native = read("src/renderer/dx12/dx12_texture_realization_internal.hpp");
    const auto cpu = read("src/renderer/dx12/dx12_texture_internal.hpp");
    require(has(native, "Dx12SrvAllocation srv") && !has(cpu, "Dx12SrvAllocation") &&
        !has(cpu, "ID3D12DescriptorHeap") && !has(cpu, "ID3D12Resource"),
        "native leases must not escape into immutable CPU texture snapshots");
    const auto texture = read("src/renderer/dx12/dx12_texture_resources.cpp");
    require(has(texture, "srv_pool->allocate(*state.device.Get(), 1)") && !has(texture, "CreateDescriptorHeap(") &&
        before(texture, "CreateShaderResourceView(", "native.resource =") &&
        has(texture, "native.srv = std::move(*srv)"), "native image/glyph realization must publish resource+lease transactionally");
    const auto blur = read("src/renderer/dx12/dx12_backdrop_blur_targets.cpp");
    require(has(blur, "srv_pool->allocate(*state.device.Get(), 3)") && has(blur, "D3D12_DESCRIPTOR_HEAP_TYPE_RTV") &&
        !has(blur, "D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV") && !has(blur, "D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE"),
        "blur must allocate one three-slot group but keep its separate RTV heap");
    const auto draw = read("src/renderer/dx12/dx12_textured_draw.cpp");
    require(has(draw, "texture.srv.heap()") && has(draw, "texture.srv.gpu()") && has(draw, "renderer.samplers.Get()") &&
        (has(draw, "dx12_emit_prepared_draw(") && has(read("src/renderer/dx12/dx12_native_draw_recording.cpp"), "SetGraphicsRootDescriptorTable(2, draw.sampler)")), "each texture draw must bind its SRV page AND sampler tables");
    require(has(read("src/renderer/dx12/dx12_backdrop_blur_passes.cpp"), "targets.srvs.heap()") &&
        has(read("src/renderer/dx12/dx12_backdrop_blur_composite.cpp"), "SetGraphicsRootDescriptorTable(2, targets.targets[0].srv)"),
        "alpha blur must keep original and filtered views on the bound group page");
    const auto retirement = read("src/renderer/dx12/dx12_native_retirement.cpp");
    require(before(retirement, "dx12_reset_texture_resources(state)", "srv_pool->clear_after_retirement()") &&
        before(retirement, "backdrop.frame_targets.clear()", "srv_pool->clear_after_retirement()") &&
        before(retirement, "srv_pool->clear_after_retirement()", "state.device.Reset()"),
        "native owners must retire before the page catalog and native device");
    const auto presentation = read("src/renderer/dx12/dx12_presentation.cpp");
    require(before(presentation, "dx12_wait_idle(state)", "dx12_begin_texture_frame(state)") &&
        before(presentation, "dx12_wait_idle(state)", "dx12_begin_backdrop_blur_frame(state)"),
        "native owner pruning requires the existing completion proof");
    for (const auto* name : {"dx12_renderer.cpp", "dx12_state.cpp", "dx12_frame.cpp", "dx12_presentation.cpp"}) {
      const auto source = read(std::string("src/renderer/dx12/") + name);
      require(!has(source, "Dx12SrvAllocation::") && !has(source, "Dx12SrvDescriptorPool::") &&
          !has(source, "CreateDescriptorHeap("), "aggregate owners cannot absorb the allocator implementation");
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "Dx12SrvAllocation") && !has(source, "Dx12SrvDescriptorPool"), "SRV leases are not public API");
    }
    require(has(read("packages/renderer-dx12/mcpp.toml"), "../../src/renderer/dx12/*.cpp") &&
        has(read("tests/architecture/mcpp.toml"), "dx12/*_structure_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_srv_descriptor_pool_structure_test|"),
        "private pool sources and architecture test must retain mcpp/CI discovery");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* test : {"dx12_srv_heap_sharing_test.cpp", "dx12_srv_blur_sharing_test.cpp",
        "dx12_srv_pool_allocation_test.cpp", "dx12_srv_pool_growth_test.cpp", "dx12_srv_pool_failure_test.cpp",
        "dx12_srv_pool_warm_test.cpp", "dx12_srv_mixed_alpha_test.cpp", "dx12_srv_alpha_page_boundary_test.cpp",
        "dx12_srv_history_test.cpp", "dx12_srv_glyph_history_test.cpp", "dx12_srv_present_failure_test.cpp",
        "dx12_srv_pending_retirement_test.cpp", "dx12_srv_recovery_test.cpp", "dx12_srv_cohort_orphan_test.cpp",
        "dx12_srv_orphan_lifetime_test.cpp", "dx12_srv_realization_failure_test.cpp"})
      require(has(manifest, test), "focused SRV behavior test missing from mcpp discovery");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
