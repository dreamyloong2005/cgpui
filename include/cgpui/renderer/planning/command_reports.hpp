#pragma once

#include "cgpui/renderer/renderer_frame_reports.hpp"

namespace cgpui {

RendererCommandReport build_renderer_command_report(
    std::span<const RendererCommandStreamItem> commands);
RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
RendererCommandReport build_renderer_command_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    std::span<const ImageDraw> image_draws,
    GlyphCache& glyph_cache);
RendererFrameReport build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws,
    GlyphCache& glyph_cache);
RendererFrameReport build_renderer_frame_report(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets,
    GlyphCache& glyph_cache);
std::vector<RendererCommandBatch> build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws);
std::vector<RendererCommandBatch> build_renderer_command_batches(
    std::span<const SolidRect> rects,
    std::span<const RoundedRectDraw> rounded_rects,
    std::span<const TextDraw> text_draws,
    std::span<const TextSelectionDraw> selections,
    std::span<const TextCaretDraw> carets);

} // namespace cgpui
