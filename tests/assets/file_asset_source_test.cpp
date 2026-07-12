#include "cgpui/core/asset_source.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

class TempAssetTree {
 public:
  TempAssetTree()
      : root_(std::filesystem::temp_directory_path() /
              "cgpui-file-asset-source-test") {
    std::error_code error;
    std::filesystem::remove_all(root_, error);
    std::filesystem::create_directories(root_ / "nested", error);
    write("alpha.bin", {0x00, 0x41, 0xff});
    write("nested/beta.bin", {1, 2, 3, 4});
    write("too-large.bin", std::vector<unsigned char>(9, 7));
    write("empty.bin", {});
  }

  ~TempAssetTree() {
    std::error_code error;
    std::filesystem::remove_all(root_, error);
  }

  const std::filesystem::path& root() const { return root_; }

 private:
  void write(
      const std::filesystem::path& relative,
      const std::vector<unsigned char>& bytes) {
    std::ofstream output(root_ / relative, std::ios::binary);
    output.write(
        reinterpret_cast<const char*>(bytes.data()),
        static_cast<std::streamsize>(bytes.size()));
  }

  std::filesystem::path root_;
};

int test_loads_binary_empty_and_missing_assets() {
  TempAssetTree tree;
  const cgpui::FileAssetSource source(
      tree.root(), cgpui::FileAssetSourceOptions{.max_asset_bytes = 8});

  const auto alpha = source.load("alpha.bin");
  const auto nested = source.load("nested/beta.bin");
  const auto empty = source.load("empty.bin");
  const auto missing = source.load("missing.bin");
  if (!alpha || !alpha->has_value() ||
      (*alpha)->bytes != std::vector<std::uint8_t>({0x00, 0x41, 0xff})) {
    return 11;
  }
  if (!nested || !nested->has_value() || (*nested)->bytes.size() != 4) {
    return 12;
  }
  if (!empty || !empty->has_value() || !(*empty)->bytes.empty()) return 13;
  if (!missing) return 141;
  if (missing->has_value()) return 142;
  return 0;
}

int test_rejects_escape_absolute_directory_and_oversize_paths() {
  TempAssetTree tree;
  const cgpui::FileAssetSource source(
      tree.root(), cgpui::FileAssetSourceOptions{.max_asset_bytes = 8});

  const auto traversal = source.load("../outside.bin");
  const auto absolute = source.load(tree.root().string());
  const auto directory = source.load("nested");
  const auto oversized = source.load("too-large.bin");
  const auto missing_root = cgpui::FileAssetSource({}).load("asset.bin");
  if (traversal || traversal.error().code != cgpui::ErrorCode::invalid_argument ||
      absolute || absolute.error().code != cgpui::ErrorCode::invalid_argument ||
      directory || directory.error().code != cgpui::ErrorCode::asset_io_failed ||
      oversized || oversized.error().code != cgpui::ErrorCode::asset_too_large ||
      missing_root ||
      missing_root.error().code != cgpui::ErrorCode::asset_io_failed) {
    return 2;
  }
  return 0;
}

int test_lists_entries_stably_and_rejects_invalid_directories() {
  TempAssetTree tree;
  const cgpui::FileAssetSource source(tree.root());
  const auto root_entries = source.list("");
  const auto nested_entries = source.list("nested");
  const auto missing = source.list("missing");
  const std::vector<std::string> expected{
      "alpha.bin", "empty.bin", "nested", "too-large.bin"};
  if (!root_entries || *root_entries != expected || !nested_entries ||
      *nested_entries != std::vector<std::string>{"beta.bin"} ||
      missing || missing.error().code != cgpui::ErrorCode::asset_io_failed) {
    return 3;
  }
  return 0;
}

} // namespace

int main() {
  if (const int result = test_loads_binary_empty_and_missing_assets()) {
    return result;
  }
  if (const int result =
          test_rejects_escape_absolute_directory_and_oversize_paths()) {
    return result;
  }
  return test_lists_entries_stably_and_rejects_invalid_directories();
}
