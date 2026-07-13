#include "cgpui/ui/async_asset_loading.hpp"

int main() {
  cgpui::AsyncAssetLoadRequest request;
  cgpui::AsyncAssetLoadResult result{
      .key = {},
      .bytes = std::optional<cgpui::AssetBytes>{},
  };
  cgpui::AsyncAssetLoadCallback callback;
  return request.source == nullptr &&
          request.priority == cgpui::TaskPriority::normal &&
          result.bytes.has_value() && !result.bytes->has_value() && !callback
      ? 0
      : 1;
}
