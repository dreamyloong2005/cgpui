#include "cgpui/core/asset_source.hpp"
#include "file_asset_source_internal.hpp"

#include <algorithm>
#include <fstream>
#include <limits>
#include <system_error>
#include <utility>

namespace cgpui {
namespace {

namespace fs = std::filesystem;

} // namespace

AssetSource::~AssetSource() = default;

FileAssetSource::FileAssetSource(
    std::filesystem::path root,
    FileAssetSourceOptions options)
    : options_(options) {
  if (root.empty()) return;
  std::error_code error;
  const fs::path absolute = fs::absolute(std::move(root), error);
  if (error) return;
  root_ = fs::weakly_canonical(absolute, error);
  if (error) root_ = absolute.lexically_normal();
}

Result<std::optional<AssetBytes>> FileAssetSource::load(
    std::string_view path) const {
  const Result<fs::path> resolved =
      detail::resolve_asset_path(root_, path, false);
  if (!resolved) return std::unexpected(resolved.error());

  std::error_code error;
  const fs::file_status status = fs::status(*resolved, error);
  if (error == std::errc::no_such_file_or_directory) return std::nullopt;
  if (error) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_io_failed,
        "failed to inspect asset file: " + error.message()));
  }
  if (!fs::exists(status)) return std::nullopt;
  if (!fs::is_regular_file(status)) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_io_failed, "asset path is not a regular file"));
  }

  const std::uintmax_t file_size = fs::file_size(*resolved, error);
  if (error || file_size > std::numeric_limits<std::size_t>::max() ||
      file_size > static_cast<std::uintmax_t>(
                      std::numeric_limits<std::streamsize>::max())) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_io_failed, "failed to determine asset file size"));
  }
  if (file_size > options_.max_asset_bytes) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_too_large, "asset file exceeds configured byte limit"));
  }

  AssetBytes asset;
  asset.bytes.resize(static_cast<std::size_t>(file_size));
  std::ifstream input(*resolved, std::ios::binary);
  if (!input) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_io_failed, "failed to open asset file"));
  }
  if (!asset.bytes.empty()) {
    input.read(
        reinterpret_cast<char*>(asset.bytes.data()),
        static_cast<std::streamsize>(asset.bytes.size()));
    if (!input || input.gcount() != static_cast<std::streamsize>(asset.bytes.size())) {
      return std::unexpected(detail::asset_error(
          ErrorCode::asset_io_failed, "failed to read complete asset file"));
    }
  }
  return std::optional<AssetBytes>{std::move(asset)};
}

Result<std::vector<std::string>> FileAssetSource::list(
    std::string_view path) const {
  const Result<fs::path> resolved =
      detail::resolve_asset_path(root_, path, true);
  if (!resolved) return std::unexpected(resolved.error());

  std::error_code error;
  fs::directory_iterator entry(*resolved, error);
  if (error) {
    return std::unexpected(detail::asset_error(
        ErrorCode::asset_io_failed,
        "failed to list asset directory: " + error.message()));
  }

  std::vector<std::string> entries;
  const fs::directory_iterator end;
  while (entry != end) {
    entries.push_back(entry->path().filename().generic_string());
    entry.increment(error);
    if (error) {
      return std::unexpected(detail::asset_error(
          ErrorCode::asset_io_failed,
          "failed while listing asset directory: " + error.message()));
    }
  }
  std::ranges::sort(entries);
  return entries;
}

const std::filesystem::path& FileAssetSource::root() const {
  return root_;
}

FileAssetSourceOptions FileAssetSource::options() const {
  return options_;
}

} // namespace cgpui
