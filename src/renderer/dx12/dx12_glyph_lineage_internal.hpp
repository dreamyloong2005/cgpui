#pragma once
#include "cgpui/core/error.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>

namespace cgpui {
struct Dx12Texture;
struct Dx12GlyphFamily;

// No texture ownership, predecessor or native object is kept by this metadata.
struct Dx12GlyphSnapshot {
  std::weak_ptr<const Dx12Texture> source;
  std::shared_ptr<const Dx12GlyphFamily> family;
  std::size_t page_index = 0, prefix = 0;
  std::uint32_t width = 0, height = 0;
};
struct Dx12GlyphLineageStats {
  std::size_t families = 0, record_indices = 0, weak_snapshots = 0;
};

class Dx12GlyphLineage {
 public:
  Dx12GlyphLineage();
  ~Dx12GlyphLineage();
  Dx12GlyphLineage(const Dx12GlyphLineage&) = delete;
  Dx12GlyphLineage& operator=(const Dx12GlyphLineage&) = delete;
  Result<void> ensure_page(std::size_t page, std::uint32_t width, std::uint32_t height);
  Result<void> prepare_record(std::size_t page, std::size_t global_index);
  void commit_record(std::size_t page, std::size_t global_index) noexcept;
  Result<void> seal_snapshot(std::size_t page, const std::shared_ptr<const Dx12Texture>& source);
  std::optional<Dx12GlyphSnapshot> find_snapshot(
      const std::shared_ptr<const Dx12Texture>& source) const noexcept;
  // Borrowed numeric indices only; caller must not mutate lineage while using them.
  std::optional<std::span<const std::size_t>> record_indices(const Dx12GlyphSnapshot&) const noexcept;
  bool current(const Dx12GlyphSnapshot&) const noexcept;
  void prune_expired() noexcept;
  void clear() noexcept;
  Dx12GlyphLineageStats stats() const noexcept;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

class Dx12GlyphLineageSweep {
 public:
  explicit Dx12GlyphLineageSweep(Dx12GlyphLineage&) noexcept;
  ~Dx12GlyphLineageSweep();
  Dx12GlyphLineageSweep(const Dx12GlyphLineageSweep&) = delete;
  Dx12GlyphLineageSweep& operator=(const Dx12GlyphLineageSweep&) = delete;
 private:
  Dx12GlyphLineage& lineage_;
};
} // namespace cgpui
