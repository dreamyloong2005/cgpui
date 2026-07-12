#pragma once

#include "cgpui/core/asset_source.hpp"

namespace cgpui::detail {

[[nodiscard]] Error asset_error(ErrorCode code, std::string message);
[[nodiscard]] Result<std::filesystem::path> resolve_asset_path(
    const std::filesystem::path& root,
    std::string_view requested,
    bool allow_empty);

} // namespace cgpui::detail
