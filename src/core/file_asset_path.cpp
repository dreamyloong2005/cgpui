#include "file_asset_source_internal.hpp"

#include <system_error>
#include <utility>

namespace cgpui::detail {
namespace {

namespace fs = std::filesystem;

bool valid_relative_asset_path(const fs::path& path, bool allow_empty) {
  if ((!allow_empty && path.empty()) || path.is_absolute() ||
      path.has_root_name() || path.has_root_directory()) {
    return false;
  }
  for (const fs::path& component : path) {
    if (component == "..") return false;
  }
  return true;
}

bool path_within_root(const fs::path& root, const fs::path& candidate) {
  auto root_part = root.begin();
  auto candidate_part = candidate.begin();
  for (; root_part != root.end(); ++root_part, ++candidate_part) {
    if (candidate_part == candidate.end() || *root_part != *candidate_part) {
      return false;
    }
  }
  return true;
}

} // namespace

Error asset_error(ErrorCode code, std::string message) {
  return Error{.code = code, .message = std::move(message)};
}

Result<std::filesystem::path> resolve_asset_path(
    const std::filesystem::path& root,
    std::string_view requested,
    bool allow_empty) {
  if (root.empty()) {
    return std::unexpected(asset_error(
        ErrorCode::asset_io_failed, "asset source root is unavailable"));
  }
  const fs::path relative{std::string(requested)};
  if (!valid_relative_asset_path(relative, allow_empty)) {
    return std::unexpected(asset_error(
        ErrorCode::invalid_argument,
        "asset path must be relative and stay within the source root"));
  }

  const fs::path candidate = (root / relative).lexically_normal();
  if (!path_within_root(root, candidate)) {
    return std::unexpected(asset_error(
        ErrorCode::invalid_argument,
        "asset path escapes the source root"));
  }

  std::error_code error;
  const bool exists = fs::exists(candidate, error);
  if (error) {
    return std::unexpected(asset_error(
        ErrorCode::asset_io_failed,
        "failed to inspect asset path: " + error.message()));
  }
  if (!exists) return candidate;

  const fs::path resolved = fs::canonical(candidate, error);
  if (error) {
    return std::unexpected(asset_error(
        ErrorCode::asset_io_failed,
        "failed to resolve asset path: " + error.message()));
  }
  if (!path_within_root(root, resolved)) {
    return std::unexpected(asset_error(
        ErrorCode::invalid_argument,
        "resolved asset path escapes the source root"));
  }
  return resolved;
}

} // namespace cgpui::detail
