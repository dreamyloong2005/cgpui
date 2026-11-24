#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
namespace {
std::string read(const char* name) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / name);
  if (!input) throw std::runtime_error(name);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& value, const char* token) { return value.find(token) != value.npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
}
int main() {
  try {
    for (const char* file : {"dx12_dred_breadcrumb_summary_internal.hpp", "dx12_dred_breadcrumb_model_internal.hpp",
        "dx12_dred_memory_reader_internal.hpp", "dx12_dred_breadcrumb_copy_internal.hpp",
        "dx12_dred_breadcrumb_reads.cpp", "dx12_dred_breadcrumb_copy.cpp", "dx12_dred_process_reader.cpp",
        "dx12_dred_breadcrumb_native_internal.hpp", "dx12_dred_breadcrumb_native.cpp",
        "dx12_dred_breadcrumb_format_internal.hpp", "dx12_dred_breadcrumb_format.cpp"}) {
      const auto text = read((std::string("src/renderer/dx12/") + file).c_str());
      require(std::count(text.begin(), text.end(), '\n') <= 120, "breadcrumb leaf exceeds its focused cap");
      for (const char* banned : {"pCommandListDebugName", "pCommandQueueDebugName", "GetPageFaultAllocationOutput",
          "state.failed", "state.removal_reason", "state.recovery_pending", "std::vector", "std::string"})
        require(!has(text, banned), "diagnostic reader crossed names/pagefault/recovery/dynamic-storage boundary");
    }
    const auto reader = read("src/renderer/dx12/dx12_dred_process_reader.cpp");
    require(has(reader, "ReadProcessMemory(GetCurrentProcess()") && !has(reader, "memcpy"),
        "native source reads must use the self-process guarded memory API");
    const auto copy = read("src/renderer/dx12/dx12_dred_breadcrumb_copy.cpp");
    require(has(copy, "copy.node(current, first)") && has(copy, "copy.node(current, last)") &&
        has(copy, "completed != candidate.completed") && has(copy, "visited") &&
        has(copy, "std::max(oldest, around)") && has(copy, "report.nodes[summary.nodes++] = candidate"),
        "copier requires bounded progress window, repeat detection and complete-sample publication");
    const auto reads = read("src/renderer/dx12/dx12_dred_breadcrumb_reads.cpp");
    require(has(reads, "!result.success || result.transferred != destination.size()") &&
        has(reads, "max_calls = 512, max_bytes = 65536") && has(reads, "% dx12_dred_history_capacity") &&
        has(reads, "numeric_limits<std::uintptr_t>::max() - address"), "exact reads need byte/call/overflow/ring guards");
    const auto model = read("src/renderer/dx12/dx12_dred_breadcrumb_model_internal.hpp");
    require(!has(model, "uintptr_t") && !has(model, "ID3D12") && !has(model, "const char*"),
        "owned report cannot retain borrowed addresses/names/COM pointers");
    const auto native = read("src/renderer/dx12/dx12_dred_breadcrumb_native.cpp");
    require(has(native, "FAILED(queried) || !dred") && has(native, "GetAutoBreadcrumbsOutput(&output)") &&
        has(native, "dx12_sample_dred_breadcrumbs(") && !has(native, "output.pHeadAutoBreadcrumbNode->"),
        "native adapter must gate output query and pass only integer addresses to sampler");
    const auto site = read("src/renderer/dx12/dx12_dred_site.cpp");
    require(site.find("if (reason >= 0)") < site.find("api.query_interface()") &&
        has(site, "report.breadcrumbs = api.breadcrumb_summary()"), "live devices never request borrowed output");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    for (const char* test : {"dx12_dred_breadcrumb_copy_test.cpp", "dx12_dred_breadcrumb_failure_test.cpp",
        "dx12_dred_breadcrumb_ring_test.cpp", "dx12_dred_process_reader_test.cpp", "dx12_dred_breadcrumb_api_test.cpp",
        "dx12_dred_breadcrumb_site_test.cpp", "dx12_dred_breadcrumb_format_test.cpp",
        "dx12_dred_breadcrumb_test_support.cpp", "dx12_dred_breadcrumb_reference_test_support.cpp"})
      require(has(manifest, test), "register every breadcrumb behavior/support leaf");
    require(has(read("scripts/ci/architecture-header-targets.txt"),
        "dx12_dred_breadcrumb_structure_test|tests/architecture/dx12/dx12_dred_breadcrumb_structure_test.cpp"),
        "structure test requires exact CI inventory registration");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
