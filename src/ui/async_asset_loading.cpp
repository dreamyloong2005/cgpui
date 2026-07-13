#include "cgpui/ui/async_asset_loading.hpp"

#include "cgpui/ui/async_context.hpp"

#include <exception>
#include <optional>
#include <utility>

namespace cgpui {
namespace {

struct AsyncAssetLoadState {
  std::optional<AsyncAssetLoadResult> result;
};

Result<std::optional<AssetBytes>> load_asset_bytes(
    const AssetSource& source,
    std::string_view path) {
  try {
    return source.load(path);
  } catch (const std::exception& error) {
    return std::unexpected(Error{
        .code = ErrorCode::asset_io_failed,
        .message = std::string{"asset source load threw: "} + error.what(),
    });
  } catch (...) {
    return std::unexpected(Error{
        .code = ErrorCode::asset_io_failed,
        .message = "asset source load threw an unknown exception",
    });
  }
}

} // namespace

Result<TaskHandle> load_asset_async(
    const AsyncContextCapability& context,
    AsyncAssetLoadRequest request,
    AsyncAssetLoadCallback completion) {
  if (!request.source) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "async asset load source is required",
    });
  }
  if (!completion) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "async asset load completion callback is required",
    });
  }
  if (!context.valid()) {
    return std::unexpected(Error{
        .code = ErrorCode::invalid_argument,
        .message = "async asset load context is required",
    });
  }
  Result<AssetCacheKey> normalized = make_asset_cache_key(
      request.key.source_id, request.key.path, request.key.kind,
      request.key.revision);
  if (!normalized) return std::unexpected(normalized.error());
  auto state = std::make_shared<AsyncAssetLoadState>();
  const AssetCacheKey key = std::move(*normalized);
  const std::string path = key.path;
  return context.try_spawn_background_task(
      request.priority,
      [source = std::move(request.source), key, path, state](
          TaskCancellationToken token) mutable {
        if (token.cancellation_requested()) return;
        state->result.emplace(AsyncAssetLoadResult{
            .key = std::move(key),
            .bytes = load_asset_bytes(*source, path),
        });
      },
      [state, completion = std::move(completion)](
          const WindowRuntimeContext& completion_context) mutable {
        if (!state->result) return;
        completion(completion_context, std::move(*state->result));
      });
}

} // namespace cgpui
