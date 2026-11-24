#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& name) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / name);
  if (!input) throw std::runtime_error("missing retained-image owner: " + name);
  return {std::istreambuf_iterator<char>(input), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void limit(const char* path, unsigned lines) {
  const auto value = read(path);
  require(std::count(value.begin(), value.end(), '\n') <= lines, "retained-image module exceeded focused cap");
}
}
int main() {
  try {
    const auto public_leaf = read("include/cgpui/renderer/renderer_frame_diagnostic_snapshot.hpp");
    require(has(public_leaf, "std::optional<std::size_t> retained_image_byte_count;") &&
        has(public_leaf, "bool retained_image_byte_count_saturated = false;") &&
        !has(public_leaf, "Dx12") && !has(public_leaf, "return "),
        "neutral payload availability/saturation belong in the existing data-only snapshot leaf");
    const auto header = read("src/renderer/dx12/dx12_image_retention_internal.hpp");
    const auto tracker = read("src/renderer/dx12/dx12_image_retention.cpp");
    require(has(header, "std::vector<std::weak_ptr<const Dx12Texture>> images_") && !has(header, "return ") &&
        has(tracker, "known.owner_before(image)") && has(tracker, "image.owner_before(known)") &&
        has(tracker, "image->pixels.size()") && has(tracker, "observed.lock()") &&
        has(tracker, "std::erase_if(images_") && has(tracker, "prune();"),
        "private measurement must use weak lifetime identity and tight payload rather than id/use_count");
    require(has(tracker, "catch (const std::bad_alloc&)") && has(tracker, "catch (const std::length_error&)") &&
        has(tracker, "if (!complete_) return {.byte_count = std::nullopt};") &&
        !has(tracker, "catch (...)") && !has(tracker, "use_count("),
        "failed bookkeeping must stay unavailable, not become a known undercount or blanket failure suppression");
    const auto cache = read("src/renderer/dx12/dx12_image_cache.cpp");
    require(has(cache, "retention_.track(stored->second.texture)") &&
        cache.find("insert_or_assign(") < cache.find("retention_.track(stored->second.texture)") &&
        has(read("src/renderer/dx12/dx12_image_cache_internal.hpp"), "Dx12ImageRetention retention_") &&
        !has(read("src/renderer/dx12/dx12_texture.cpp"), "retention_") &&
        !has(read("src/renderer/dx12/dx12_native_retirement.cpp"), "image_cache"),
        "only successful image-cache mutation registers; generic glyph and native-retirement paths do not own CPU census");
    const auto completion = read("src/renderer/dx12/dx12_frame_completion.cpp");
    const auto start = completion.find("void Dx12FrameDiagnostics::complete_successful_frame()");
    require(start != std::string::npos, "successful completion needs its focused implementation");
    const auto body = completion.substr(start);
    require(body.find("if (published_) return;") < body.find("prepare_publication();") &&
        body.find("prepare_publication();") < body.find("image_cache->finish_frame();") &&
        body.find("image_cache->finish_frame();") < body.find("retained_image_payload();") &&
        body.find("retained_image_payload();") < body.find("publish_prepared();") &&
        has(completion, "std::is_nothrow_assignable_v"),
        "exactly-once post-aging sampling follows candidate allocations and precedes nonthrowing publication");
    const auto ledger = read("src/renderer/dx12/dx12_frame_diagnostics.cpp");
    const auto submit = read("src/renderer/dx12/dx12_presentation.cpp");
    require(has(ledger, "Dx12FrameDiagnostics::publish()") && !has(ledger, "image_cache->finish_frame") &&
        !has(read("src/renderer/dx12/dx12_frame.cpp"), "image_cache->finish_frame") &&
        has(submit, "diagnostics.complete_successful_frame();") &&
        submit.find("diagnostics.complete_successful_frame();") > submit.find("dx12_commit_composition(state)") &&
        !has(submit, "diagnostics.publish();"),
        "CPU ledger publication is not native success; production completion follows all native fallible stages");
    const auto equal = read("tests/renderer-dx12/dx12_diagnostics_support.hpp");
    require(has(equal, "a->retained_image_byte_count == b.retained_image_byte_count") &&
        has(equal, "a->retained_image_byte_count_saturated == b.retained_image_byte_count_saturated"),
        "historical equality must include retention availability, count and saturation");
    const auto manifest = read("tests/renderer-dx12/mcpp.toml");
    const auto windows = manifest.find("[target.'cfg(os = \"windows\")'.test]");
    for (const auto* name : {"dx12_image_retention_test.cpp", "dx12_image_retention_cache_test.cpp",
        "dx12_image_retention_completion_test.cpp", "dx12_retained_image_equality_test.cpp",
        "dx12_retained_image_snapshots_test.cpp", "dx12_retained_image_uploads_test.cpp",
        "dx12_retained_image_history_test.cpp", "dx12_retained_image_recovery_test.cpp"}) {
      require(windows != std::string::npos && manifest.find(name, windows) != std::string::npos,
          "register each focused CPU/native retention regression");
      const auto source = read(std::string("tests/renderer-dx12/") + name);
      require(std::count(source.begin(), source.end(), '\n') <= 140, "retention regression became a broad test file");
    }
    require(has(manifest, "\"dx12_image_retention_test_support.cpp\"") &&
        has(read("tests/renderer/mcpp.toml"), "\"renderer_retained_image_diagnostics_test.cpp\"") &&
        has(read("scripts/ci/architecture-header-targets.txt"),
            "dx12_image_retention_structure_test|tests/architecture/dx12/dx12_image_retention_structure_test.cpp"),
        "register shared support, neutral contract and exact structure CI entry");
    limit("src/renderer/dx12/dx12_image_retention_internal.hpp", 45);
    limit("src/renderer/dx12/dx12_image_retention.cpp", 80);
    limit("src/renderer/dx12/dx12_frame_completion.cpp", 80);
    limit("tests/renderer-dx12/dx12_image_retention_test_support.hpp", 35);
    limit("tests/renderer-dx12/dx12_image_retention_test_support.cpp", 65);
    limit("src/renderer/dx12/dx12_renderer.cpp", 100);
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
