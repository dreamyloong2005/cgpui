#include "dx12_glyph_lineage_internal.hpp"
#include "dx12_texture_internal.hpp"
#include <cassert>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace cgpui {
struct Dx12GlyphFamily {};
struct Dx12GlyphLineage::Impl {
  struct Page {
    std::shared_ptr<const Dx12GlyphFamily> family;
    std::uint32_t width, height;
    std::vector<std::size_t> indices;
  };
  std::vector<Page> pages;
  std::map<std::weak_ptr<const Dx12Texture>, Dx12GlyphSnapshot,
           std::owner_less<std::weak_ptr<const Dx12Texture>>> snapshots;
};
namespace {
constexpr std::size_t next_index_capacity(
    std::size_t capacity, std::size_t required, std::size_t maximum) noexcept {
  // Valid vector capacity and the caller's max_size guard bound both additions.
  const auto doubled = capacity > maximum - capacity ? maximum : capacity + capacity;
  return doubled < required ? required : doubled;
}
static_assert(next_index_capacity(0, 1, 7) == 1);
static_assert(next_index_capacity(2, 3, 7) == 4);
static_assert(next_index_capacity(4, 5, 7) == 7);
static_assert(next_index_capacity(std::numeric_limits<std::size_t>::max() / 2 + 1,
    std::numeric_limits<std::size_t>::max() / 2 + 2,
    std::numeric_limits<std::size_t>::max()) == std::numeric_limits<std::size_t>::max());
Error invalid_lineage() {
  return {.code = ErrorCode::invalid_argument, .message = "DX12 glyph lineage is inconsistent"};
}
}
Dx12GlyphLineage::Dx12GlyphLineage() : impl_(std::make_unique<Impl>()) {}
Dx12GlyphLineage::~Dx12GlyphLineage() = default;
Result<void> Dx12GlyphLineage::ensure_page(std::size_t index, std::uint32_t width, std::uint32_t height) {
  if (!width || !height || width > 16384 || height > 16384 || index > impl_->pages.size())
    return std::unexpected(invalid_lineage());
  if (index < impl_->pages.size()) {
    const auto& page = impl_->pages[index];
    if (page.width != width || page.height != height) return std::unexpected(invalid_lineage());
    return {};
  }
  impl_->pages.push_back({std::make_shared<const Dx12GlyphFamily>(), width, height, {}});
  return {};
}
Result<void> Dx12GlyphLineage::prepare_record(std::size_t index, std::size_t global) {
  if (index >= impl_->pages.size()) return std::unexpected(invalid_lineage());
  auto& indices = impl_->pages[index].indices;
  if ((!indices.empty() && global <= indices.back()) || indices.size() == indices.max_size())
    return std::unexpected(invalid_lineage());
  // Finish geometric allocation BEFORE CPU pixels/cursors change; commit stays nonallocating.
  if (indices.size() == indices.capacity())
    indices.reserve(next_index_capacity(indices.capacity(), indices.size() + 1, indices.max_size()));
  return {};
}
void Dx12GlyphLineage::commit_record(std::size_t index, std::size_t global) noexcept {
  auto& indices = impl_->pages[index].indices;
  assert(indices.size() < indices.capacity() && (indices.empty() || indices.back() < global));
  indices.push_back(global);
}
Result<void> Dx12GlyphLineage::seal_snapshot(
    std::size_t index, const std::shared_ptr<const Dx12Texture>& source) {
  if (!source || index >= impl_->pages.size()) return std::unexpected(invalid_lineage());
  const auto& page = impl_->pages[index];
  if (source->width != page.width || source->height != page.height || source->stride != page.width ||
      source->pixels.size() != std::size_t(page.width) * page.height) return std::unexpected(invalid_lineage());
  Dx12GlyphSnapshot value{source, page.family, index, page.indices.size(), page.width, page.height};
  const auto found = impl_->snapshots.find(value.source);
  if (found != impl_->snapshots.end()) {
    const auto& old = found->second;
    if (old.family != value.family || old.prefix != value.prefix ||
        old.width != value.width || old.height != value.height || old.page_index != index)
      return std::unexpected(invalid_lineage());
    return {};
  }
  auto identity = value.source;
  impl_->snapshots.emplace(std::move(identity), std::move(value)); // May throw; caller has not published.
  return {};
}
std::optional<Dx12GlyphSnapshot> Dx12GlyphLineage::find_snapshot(
    const std::shared_ptr<const Dx12Texture>& source) const noexcept {
  const auto found = impl_->snapshots.find(std::weak_ptr<const Dx12Texture>{source});
  if (found == impl_->snapshots.end()) return {};
  return found->second;
}
bool Dx12GlyphLineage::current(const Dx12GlyphSnapshot& snapshot) const noexcept {
  if (snapshot.page_index >= impl_->pages.size()) return false;
  const auto& page = impl_->pages[snapshot.page_index];
  return snapshot.family && snapshot.family == page.family && snapshot.width == page.width &&
      snapshot.height == page.height && snapshot.prefix <= page.indices.size();
}
std::optional<std::span<const std::size_t>> Dx12GlyphLineage::record_indices(
    const Dx12GlyphSnapshot& snapshot) const noexcept {
  if (!current(snapshot) || snapshot.source.expired()) return {};
  const auto found = impl_->snapshots.find(snapshot.source);
  if (found == impl_->snapshots.end()) return {};
  const auto& seal = found->second;
  if (seal.family != snapshot.family || seal.page_index != snapshot.page_index ||
      seal.width != snapshot.width || seal.height != snapshot.height || seal.prefix != snapshot.prefix)
    return {};
  return std::span<const std::size_t>{impl_->pages[snapshot.page_index].indices}.first(snapshot.prefix);
}
void Dx12GlyphLineage::prune_expired() noexcept {
  for (auto it = impl_->snapshots.begin(); it != impl_->snapshots.end();)
    if (it->first.expired()) it = impl_->snapshots.erase(it); else ++it;
}
void Dx12GlyphLineage::clear() noexcept { impl_->snapshots.clear(); impl_->pages.clear(); }
Dx12GlyphLineageStats Dx12GlyphLineage::stats() const noexcept {
  Dx12GlyphLineageStats result{impl_->pages.size(), 0, impl_->snapshots.size()};
  for (const auto& page : impl_->pages) result.record_indices += page.indices.size();
  return result;
}
Dx12GlyphLineageSweep::Dx12GlyphLineageSweep(Dx12GlyphLineage& lineage) noexcept : lineage_(lineage) {
  lineage_.prune_expired();
}
Dx12GlyphLineageSweep::~Dx12GlyphLineageSweep() { lineage_.prune_expired(); }
} // namespace cgpui
