#include "dx12_runtime_recovery_loop_test_support.hpp"

#include "dx12_runtime_capture_test_support.hpp"
#include "dx12_runtime_recovery_scene_test_support.hpp"
#include "dx12_recovery_test_support.hpp"
#include "cgpui/app/window.hpp"
#include "cgpui/platform/platform_application.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/runtime_rendering.hpp"

#include <array>
#include <utility>

namespace cgpui_test {
namespace {
using namespace cgpui;

struct WindowObservation {
  RendererBackendInfo baseline;
  std::vector<std::uint8_t> glyph_pixels;
  std::size_t glyph_bytes = 0;
  float scale = 0;
  unsigned successes = 0;
  unsigned resizes_before_loss = 0;
  bool cold = false;
  bool warm = false;
  bool recovered = false;
  bool warm_control = false;
};

struct RecoveryLoop {
  PlatformApplication& application;
  bool child_first;
  std::array<WindowRuntimeId, 2> ids{};
  std::array<Dx12RuntimeCaptureCounts, 2> captures{};
  std::array<Dx12RuntimeSceneState, 2> scenes{};
  std::array<WindowObservation, 2> windows{};
  unsigned factory_count = 0;
  unsigned runtime_errors = 0;
  unsigned removals = 0;
  bool peer_warmup_requested = false;
  bool completed = false;
  int failure = 0;

  void fail(const char* reason, unsigned index = 0) {
    if (!failure) {
      failure = 1;
      std::cerr << "DX12 runtime " << (child_first ? "child" : "root")
                << "-first: window=" << index << " " << reason << '\n';
      for (unsigned i = 0; i != 2; ++i)
        std::cerr << "  window=" << i << " success=" << windows[i].successes
                  << " begin=" << captures[i].begin_attempts
                  << " begin-retry=" << captures[i].begin_retries
                  << " begin-pending=" << captures[i].begin_pending
                  << " begin-errors=" << captures[i].begin_errors
                  << " resize=" << captures[i].resize_attempts
                  << " pre-loss-resize=" << windows[i].resizes_before_loss
                  << " resize-retry=" << captures[i].resize_retries << '\n';
    }
    application.quit();
  }

  void request(WindowRuntime& runtime, unsigned index) {
    Window(runtime, ids[index]).request_render();
  }

  Result<std::unique_ptr<Renderer>> create(const RenderSurfaceDescriptor& descriptor) {
    const unsigned index = factory_count++;
    if (index >= captures.size())
      return std::unexpected(Error{.code = ErrorCode::renderer_initialization_failed,
          .message = "runtime unexpectedly recreated a renderer instead of recovering it"});
    return dx12_runtime_capture_renderer(descriptor, captures[index]);
  }

  void setup(AppContext& context) {
    ids[0] = context.runtime.root_window_runtime_id();
    context.runtime.set_error_callback([this](const Error& error) {
      ++runtime_errors;
      std::cerr << "runtime error: " << error.message << '\n';
      fail("runtime surfaced an error instead of completing native recovery");
    });
    context.runtime.set_after_frame_callback(
        [this](const WindowRuntimeContext& frame) { after_frame(frame); });
    auto child = context.runtime.try_open_window(
        WindowOptions{}.title("DX12 runtime recovery child").size(400, 260)
            .backdrop(WindowBackdrop::none).resizable(false),
        std::make_unique<Dx12RuntimeRecoveryScene>(ImageAssetId{9912}, scenes[1]));
    if (!child) { fail("pending child creation failed"); return; }
    ids[1] = child->runtime_id;
  }

  bool valid_frame(const WindowRuntimeContext& context, unsigned index) {
    const auto surface = context.platform_window.native_surface();
    const auto* native = std::get_if<Win32SurfaceHandle>(&surface);
    const auto info = context.renderer.backend_info();
    const auto* diagnostic = context.renderer.last_frame_diagnostic_snapshot();
    const auto& counts = captures[index];
    if (factory_count != 2 || !native || !native->hwnd || native->composition ||
        native->hwnd != counts.hwnd || captures[0].hwnd == captures[1].hwnd ||
        GetWindowThreadProcessId(static_cast<HWND>(native->hwnd), nullptr) != GetCurrentThreadId() ||
        info.backend != RendererBackend::dx12 || !info.adapter_luid || !info.device_generation ||
        scenes[index].upload_intents != 1 || counts.begin_errors || counts.capture_errors ||
        !diagnostic || !diagnostic->work.exact_match() || !diagnostic->dropped_resources.empty() ||
        diagnostic->upload_bytes.saturated || diagnostic->submitted_draws.saturated ||
        diagnostic->submitted_draws.solid_rect_count != 2 ||
        diagnostic->submitted_draws.image_count != 1 || diagnostic->submitted_draws.text_count != 1 ||
        diagnostic->submitted_draws.total_count != 4 ||
        (windows[index].cold && context.scale.value != windows[index].scale) ||
        !dx12_runtime_scene_pixels(context, scenes[index].epoch, windows[index].glyph_pixels)) {
      fail("native identity, once-upload scene, capture or diagnostics invalid", index);
      return false;
    }
    return true;
  }

  void before_removal(const WindowRuntimeContext& context, unsigned index) {
    auto& observed = windows[index];
    const auto info = context.renderer.backend_info();
    const auto& uploads = context.renderer.last_frame_diagnostic_snapshot()->upload_bytes;
    if (captures[index].begin_retries || captures[index].resize_retries) {
      fail("unexpected retry before the single removal", index); return;
    }
    if (!observed.cold) {
      if (uploads.image_byte_count != 16 || uploads.glyph_atlas_byte_count == 0) {
        fail("cold native image/glyph upload payload absent", index); return;
      }
      observed.baseline = info;
      observed.glyph_bytes = uploads.glyph_atlas_byte_count;
      observed.scale = context.scale.value;
      observed.cold = true;
      request(context.runtime, index);
    } else {
      if (info.device_generation != observed.baseline.device_generation ||
          info.adapter_luid != observed.baseline.adapter_luid || uploads.total_byte_count != 0) {
        fail("baseline warm-cache frame changed generation or uploaded again", index); return;
      }
      observed.warm = true;
    }
    if (!peer_warmup_requested) {
      peer_warmup_requested = true;
      request(context.runtime, 1 - index);
    }
    if (!windows[0].warm || !windows[1].warm) return;
    if (windows[0].baseline.adapter_luid != windows[1].baseline.adapter_luid) {
      fail("native windows did not select the same adapter"); return;
    }
    for (unsigned i = 0; i != 2; ++i) {
      scenes[i].epoch = 1;
      windows[i].resizes_before_loss = captures[i].resize_attempts;
    }
    ++removals;
    if (!dx12_remove_native_device(windows[0].baseline)) {
      fail("real native RemoveDevice was not verified"); return;
    }
    // Never retry begin/present in the fixture: the runtime must schedule it.
    request(context.runtime, child_first ? 1 : 0);
  }

  void after_removal(const WindowRuntimeContext& context, unsigned index) {
    auto& observed = windows[index];
    const auto info = context.renderer.backend_info();
    const auto& uploads = context.renderer.last_frame_diagnostic_snapshot()->upload_bytes;
    if (info.device_generation != observed.baseline.device_generation + 1 ||
        info.adapter_luid != observed.baseline.adapter_luid || captures[index].begin_retries != 1 ||
        captures[index].resize_retries != 0) {
      fail("fresh generation / actual typed begin retry absent (inspect resize sequence)", index);
      return;
    }
    if (!observed.recovered) {
      // Consume FIRST fresh-frame diagnostics before any extra WM_PAINT cache hit.
      if (uploads.image_byte_count != 16 || uploads.glyph_atlas_byte_count != observed.glyph_bytes) {
        fail("retained image/glyphs did not reupload their original payload", index); return;
      }
      observed.recovered = true;
      request(context.runtime, index);
      if (!windows[1 - index].recovered) request(context.runtime, 1 - index);
    } else {
      if (uploads.image_byte_count || uploads.glyph_atlas_byte_count || uploads.total_byte_count) {
        fail("next recovered warm-cache frame reuploaded textures", index); return;
      }
      observed.warm_control = true;
    }
    if (!windows[0].warm_control || !windows[1].warm_control) return;
    if (!dx12_recovered_native_device_clean(info)) {
      fail("recovered device contains native debug errors", index); return;
    }
    completed = true;
    application.quit();
  }

  void after_frame(const WindowRuntimeContext& context) {
    if (failure || completed) { application.quit(); return; }
    const unsigned index = context.window_runtime_id == ids[0] ? 0 : 1;
    if (context.window_runtime_id != ids[index]) { fail("unknown runtime window"); return; }
    ++windows[index].successes;
    if (!valid_frame(context, index)) return;
    if (removals == 0) before_removal(context, index);
    else after_removal(context, index);
  }

  int finish(int result) {
    if (result || failure || runtime_errors || !completed || factory_count != 2 || removals != 1)
      return dx12_fail(3, "native runtime did not complete all recovery phases");
    for (unsigned i = 0; i != 2; ++i) {
      if (captures[i].begin_retries != 1 || captures[i].begin_errors || captures[i].capture_errors ||
          captures[i].resize_retries || scenes[i].upload_intents != 1 ||
          !windows[i].recovered || !windows[i].warm_control)
        return dx12_fail(4, "post-run externally owned retry/cache/factory invariants differ");
      std::cout << "PACING_RUNTIME window=" << i << " attempts=" << captures[i].begin_attempts
                << " pending=" << captures[i].begin_pending << " successes=" << windows[i].successes
                << " recovery-retry=" << captures[i].begin_retries << '\n';
    }
    return 0;
  }
};
} // namespace

int dx12_runtime_recovery_case(bool child_first) {
  Dx12RecoveryDeadline deadline;
  if (!deadline.done) return dx12_fail(1, "runtime recovery watchdog unavailable");
  auto application = cgpui::create_platform_application();
  if (!application) return dx12_fail(2, application.error().message.c_str());
  RecoveryLoop loop{.application = **application, .child_first = child_first};
  Dx12RuntimeRecoveryScene root(cgpui::ImageAssetId{9911}, loop.scenes[0]);
  cgpui::AppRunnerOptions options;
  options.window.title = "DX12 runtime recovery root";
  options.window.size = {400, 260};
  options.window.chrome.resizable = false;
  options.window.backdrop = cgpui::WindowBackdrop::none;
  options.setup_context = [&](cgpui::AppContext& context) { loop.setup(context); };
  const int result = cgpui::run_app(**application, root,
      [&](const cgpui::RenderSurfaceDescriptor& descriptor) { return loop.create(descriptor); },
      std::move(options));
  return loop.finish(result); // No runtime-owned renderer pointers survive run_app.
}

} // namespace cgpui_test
