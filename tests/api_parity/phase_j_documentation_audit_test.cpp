#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}
bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}
}  // namespace

int main() {
  const std::array paths{
      "docs/getting-started.md", "docs/architecture.md",
      "docs/platform-dependencies.md", "docs/examples.md",
      "docs/gpui-to-cpp23-migration.md", "docs/non-goals.md"};
  std::array<std::string, paths.size()> docs;
  for (std::size_t index = 0; index < paths.size(); ++index) {
    docs[index] = read_source(paths[index]);
    if (docs[index].empty()) return 1;
  }
  const std::string readme = read_source("README.md");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  if (readme.empty() || roadmap.empty() || ledger.empty()) return 2;
  if (!contains(docs[0], "xmake build -j 1 hello_window") ||
      !contains(docs[0], "CGPUI_LINUX_BACKEND") ||
      !contains(docs[1], "## Static Fast Path") ||
      !contains(docs[1], "## Dynamic Escape Hatch") ||
      !contains(docs[2], "Win32 + Vulkan") ||
      !contains(docs[2], "Wayland + Vulkan") ||
      !contains(docs[2], "X11/XCB + Vulkan")) return 3;
  if (!contains(docs[3], "gpui-official-example-matrix.json") ||
      !contains(docs[4], "Context<T>") ||
      !contains(docs[4], "std::expected") ||
      !contains(docs[5], "Web/WASM") ||
      !contains(docs[5], "Android") || !contains(docs[5], "iOS")) return 4;
  for (const auto* path : paths) if (!contains(readme, path)) return 5;
  if (!contains(roadmap, "- [x] Steps 823-828: Complete documentation") ||
      !contains(ledger, "\"phase_j_steps_823_828_status\": \"complete\"") ||
      !contains(ledger, "Phase J Step 829 reproducible packages")) return 6;
  return 0;
}
