#pragma once
#include "dx12_glyph_native_internal.hpp"
#include "dx12_glyph_upload_plan_internal.hpp"
#include "dx12_texture_upload_internal.hpp"
#include <optional>

namespace cgpui {
struct Dx12State;
struct Dx12NativeTexture;
// Borrowed immutable inputs under existing serialization; not a public cache mutation hook.
struct Dx12GlyphTargetInputs {
  const Dx12GlyphLineage* lineage = nullptr;
  std::span<const GlyphUploadRecord> history;
  std::span<const GlyphAtlasPage> pages;
};
struct Dx12GlyphPreparedTarget {
  std::optional<Dx12GlyphSnapshot> snapshot;
  Dx12GlyphNativeBase base;
  std::optional<Dx12GlyphUploadPlan> regions;
  std::vector<Dx12TextureUploadSlice> staging;
};
Result<Dx12GlyphPreparedTarget> dx12_prepare_glyph_target(
    Dx12State&, const Dx12TextureHandle&, Dx12NativeTexture&);
Result<Dx12GlyphPreparedTarget> dx12_prepare_glyph_target(
    Dx12State&, const Dx12TextureHandle&, Dx12NativeTexture&, const Dx12GlyphTargetInputs&);
Result<void> dx12_stage_glyph_target(Dx12State&, const Dx12Texture&, Dx12GlyphPreparedTarget&);
void dx12_record_glyph_target(Dx12State&, Dx12NativeTexture&, Dx12GlyphPreparedTarget&) noexcept;
} // namespace cgpui
