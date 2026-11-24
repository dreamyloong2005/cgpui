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
  if (!file) throw std::runtime_error("missing glyph CPU owner: " + name);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& text, const char* word) { return text.find(word) != std::string::npos; }
void require(bool value, const char* reason) { if (!value) throw std::runtime_error(reason); }
bool before(const std::string& text, const char* first, const char* second) {
  const auto a = text.find(first), b = text.find(second);
  return a != std::string::npos && b != std::string::npos && a < b;
}
std::string section(const std::string& text, const char* first, const char* last) {
  const auto begin = text.find(first), end = text.find(last);
  require(begin != std::string::npos && end != std::string::npos && begin < end, "missing lineage method boundary");
  return text.substr(begin, end - begin);
}
}
int main() {
  try {
    for (const auto* leaf : {"dx12_glyph_lineage", "dx12_glyph_record_validation",
                            "dx12_glyph_suffix", "dx12_glyph_upload_plan"}) {
      for (const auto* ext : {"_internal.hpp", ".cpp"}) {
        const auto source = read(std::string("src/renderer/dx12/") + leaf + ext);
        require(std::count(source.begin(), source.end(), '\n') <= 170, "glyph CPU leaf grew broad");
        for (const auto* native : {"ID3D12", "DXGI_", "CopyTextureRegion(", "CreateCommittedResource("})
          require(!has(source, native), "CPU lineage/planner must not own native objects or commands");
      }
    }
    const auto lineage = read("src/renderer/dx12/dx12_glyph_lineage.cpp");
    const auto preparation = section(lineage, "Result<void> Dx12GlyphLineage::prepare_record(",
        "void Dx12GlyphLineage::commit_record(");
    const auto commit = section(lineage, "void Dx12GlyphLineage::commit_record(",
        "Result<void> Dx12GlyphLineage::seal_snapshot(");
    require(has(lineage, "std::owner_less<std::weak_ptr<const Dx12Texture>>") &&
        has(lineage, "std::vector<std::size_t> indices;") && has(lineage, "it->first.expired()") &&
        !has(lineage, "source.lock("), "lineage must own weak identity and numeric mapping only");
    require(before(preparation, "indices.max_size()", "indices.reserve(") &&
        before(preparation, "indices.reserve(", "return {};") && !has(preparation, "indices.push_back(") &&
        has(commit, ") noexcept") && before(commit, "indices.size() < indices.capacity()", "indices.push_back(global)") &&
        !has(commit, ".reserve(") && !has(commit, ".resize("),
        "prepare must reserve checked capacity before success; noexcept commit only appends preallocated mapping");
    const auto atlas = read("src/renderer/dx12/dx12_text_atlas.cpp");
    require(before(atlas, "cache.lineage.prepare_record(", "copy_upload(upload, *layout, page)") &&
        before(atlas, "copy_upload(upload, *layout, page)", "cache.lineage.commit_record(") &&
        before(atlas, "cache.lineage.commit_record(", "++cache.processed_uploads") &&
        before(atlas, "cache.lineage.seal_snapshot(", "page.texture = std::move(*snapshot)") &&
        before(atlas, "page.texture = std::move(*snapshot)", "page.dirty = false"),
        "CPU publication/record progress must follow successful preparation and sealing");
    require(has(atlas, "Dx12GlyphLineageSweep sweep(cache.lineage)") &&
        has(read("src/renderer/dx12/dx12_text_draws.cpp"), "Dx12GlyphLineageSweep sweep(cache.lineage)"),
        "error/abandon/cache-hit sweeps cannot depend on Present");
    const auto planner = read("src/renderer/dx12/dx12_glyph_upload_plan.cpp");
    require(before(planner, "for (const auto& index : suffix->records)", "GlyphAtlasProductionResourceState production_state;") &&
        has(planner, "history.subspan(index.global_index, 1)") && has(planner, "std::move(batch)") &&
        !has(planner, "std::vector<GlyphUploadRecord>"), "fresh state and borrowed one-record planning required");
    const auto registry = read("tests/renderer-dx12/mcpp.toml");
    for (const auto* test : {"dx12_glyph_lineage_test.cpp", "dx12_glyph_lineage_growth_test.cpp",
                            "dx12_glyph_suffix_test.cpp",
                            "dx12_glyph_publication_test.cpp", "dx12_glyph_publication_failure_test.cpp",
                            "dx12_srv_heap_sharing_test.cpp",
                            "dx12_srv_blur_sharing_test.cpp", "dx12_srv_recovery_test.cpp"})
      require(has(registry, test), "new glyph cases must not replace existing SRV registrations");
    require(has(read("scripts/ci/architecture-header-targets.txt"), "dx12_glyph_cpu_lineage_structure_test|") &&
        has(read("tests/architecture/mcpp.toml"), "dx12/*_structure_test.cpp") &&
        has(read("packages/renderer-dx12/mcpp.toml"), "../../src/renderer/dx12/*.cpp"),
        "owning mcpp and CI discovery missing");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
