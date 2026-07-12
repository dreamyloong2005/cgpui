#pragma once

#include "cgpui/core/error.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

struct AssetBytes {
  std::vector<std::uint8_t> bytes;
};

class AssetSource {
 public:
  virtual ~AssetSource();

  [[nodiscard]] virtual Result<std::optional<AssetBytes>> load(
      std::string_view path) const = 0;
  [[nodiscard]] virtual Result<std::vector<std::string>> list(
      std::string_view path) const = 0;
};

struct FileAssetSourceOptions {
  std::size_t max_asset_bytes = 64U * 1024U * 1024U;
};

class FileAssetSource final : public AssetSource {
 public:
  explicit FileAssetSource(
      std::filesystem::path root,
      FileAssetSourceOptions options = {});

  [[nodiscard]] Result<std::optional<AssetBytes>> load(
      std::string_view path) const override;
  [[nodiscard]] Result<std::vector<std::string>> list(
      std::string_view path) const override;

  [[nodiscard]] const std::filesystem::path& root() const;
  [[nodiscard]] FileAssetSourceOptions options() const;

 private:
  std::filesystem::path root_;
  FileAssetSourceOptions options_;
};

} // namespace cgpui
