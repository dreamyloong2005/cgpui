#include "cgpui/platform/platform.hpp"
#include "cgpui/renderer/renderer.hpp"
#include "metal_renderer_internal.hpp"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <chrono>
#include <thread>
#include <variant>

int main() {
  @autoreleasepool {
    const cgpui::RenderSurfaceDescriptor wrong_surface{
        .native_surface = cgpui::Win32SurfaceHandle{},
        .framebuffer_size = cgpui::Size{64.0F, 64.0F},
        .scale = cgpui::DpiScale{1.0F}};
    if (cgpui::create_renderer(wrong_surface)) return 1;

    const cgpui::RenderSurfaceDescriptor empty_surface{
        .native_surface = cgpui::MetalSurfaceHandle{},
        .framebuffer_size = cgpui::Size{64.0F, 64.0F},
        .scale = cgpui::DpiScale{1.0F}};
    if (cgpui::create_renderer(empty_surface)) return 2;

    cgpui::MetalFramePacingState pacing;
    if (!pacing.try_acquire() || !pacing.try_acquire() ||
        !pacing.try_acquire() || pacing.try_acquire() ||
        pacing.in_flight_count() != 3) return 3;
    pacing.release();
    if (!pacing.try_acquire() || pacing.in_flight_count() != 3) return 4;
    pacing.release();
    pacing.release();
    pacing.release();

    auto application = cgpui::create_platform_application();
    if (!application) return 5;
    auto window = (*application)->create_window(
        cgpui::WindowDescriptor{
            .title = "CGPUI Metal Bootstrap Test",
            .size = cgpui::Size{128.0F, 96.0F}},
        [](const cgpui::PlatformEvent&) {});
    if (!window) return 6;

    const cgpui::WindowState state = (*window)->state();
    cgpui::RenderSurfaceDescriptor descriptor{
        .native_surface = (*window)->native_surface(),
        .framebuffer_size = state.framebuffer_size,
        .scale = state.scale};
    auto metal_state = cgpui::create_metal_renderer_state(descriptor);
    if (!metal_state || (*metal_state)->device == nil ||
        (*metal_state)->command_queue == nil ||
        (*metal_state)->next_frame_id.load() != 1) {
      return 7;
    }
    auto renderer = cgpui::make_metal_renderer(*metal_state);
    if (renderer->last_frame_diagnostic_snapshot() != nullptr) return 8;

    auto* layer = (__bridge CAMetalLayer*)
        std::get<cgpui::MetalSurfaceHandle>(descriptor.native_surface).layer;
    if (layer == nil || layer.device == nil ||
        layer.pixelFormat != MTLPixelFormatBGRA8Unorm ||
        layer.maximumDrawableCount != 3) return 9;

    cgpui::RenderSurfaceDescriptor zero_sized_descriptor = descriptor;
    zero_sized_descriptor.framebuffer_size = cgpui::Size{};
    if (cgpui::create_metal_renderer_state(zero_sized_descriptor)) return 10;

    if (!renderer->resize(cgpui::Size{160.0F, 120.0F},
                          cgpui::DpiScale{1.5F}) ||
        layer.drawableSize.width != 160.0 ||
        layer.drawableSize.height != 120.0 ||
        layer.contentsScale != 1.5) return 11;

    auto frame = renderer->begin_frame();
    if (!frame || (*metal_state)->next_frame_id.load() != 2 ||
        (*metal_state)->pacing.in_flight_count() != 1) return 12;
    (*frame)->clear(cgpui::Color{.r = 0.25F, .g = 0.5F, .b = 0.75F, .a = 1.0F});
    if (!(*frame)->present()) return 13;

    const auto completion_deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds{5};
    while ((*metal_state)->pacing.in_flight_count() != 0 &&
           std::chrono::steady_clock::now() < completion_deadline) {
      std::this_thread::sleep_for(std::chrono::milliseconds{1});
    }
    if ((*metal_state)->pacing.in_flight_count() != 0) return 14;

    const cgpui::RendererFrameDiagnosticSnapshot* snapshot =
        renderer->last_frame_diagnostic_snapshot();
    if (snapshot == nullptr || !snapshot->work.exact_match() ||
        snapshot->work.submitted_work.command_count != 1 ||
        snapshot->work.submitted_work.batch_count != 1 ||
        snapshot->work.submitted_work.draw_count != 0 ||
        snapshot->timings.image_acquisition_nanoseconds == 0 ||
        snapshot->timings.total_nanoseconds == 0) return 15;
  }
  return 0;
}
