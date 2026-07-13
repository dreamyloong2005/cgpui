#pragma once

#include "cgpui/core/asset_cache_key.hpp"
#include "cgpui/core/asset_source.hpp"
#include "cgpui/ui/runtime_handles.hpp"
#include "cgpui/ui/task_priority.hpp"

#include <functional>
#include <memory>

namespace cgpui {

class AsyncContextCapability;
struct WindowRuntimeContext;

struct AsyncAssetLoadRequest {
  std::shared_ptr<const AssetSource> source;
  AssetCacheKey key;
  TaskPriority priority = TaskPriority::normal;
};

struct AsyncAssetLoadResult {
  AssetCacheKey key;
  Result<std::optional<AssetBytes>> bytes;
};

using AsyncAssetLoadCallback = std::function<void(
    const WindowRuntimeContext&,
    AsyncAssetLoadResult)>;

[[nodiscard]] Result<TaskHandle> load_asset_async(
    const AsyncContextCapability& context,
    AsyncAssetLoadRequest request,
    AsyncAssetLoadCallback completion);

} // namespace cgpui
