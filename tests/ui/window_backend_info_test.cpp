#include "runtime_window_lifecycle_test_support.hpp"

namespace {
class InfoRenderer final : public cgpui::Renderer {
 public:
  explicit InfoRenderer(std::uint32_t vendor) : vendor_(vendor) {}
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    return std::unexpected(cgpui::Error{
        cgpui::ErrorCode::frame_acquisition_failed, "not painted"});
  }
  cgpui::RendererBackendInfo backend_info() const override {
    return {.backend = cgpui::RendererBackend::dx12, .adapter_name = "test GPU",
            .vendor_id = vendor_, .adapter_luid = 100 + vendor_,
            .selection = cgpui::RendererBackendSelection::dx12,
            .device_generation = 1};
  }
 private:
  std::uint32_t vendor_;
};
}

int main() {
  runtime_window_test::Application application;
  runtime_window_test::View view;
  std::vector<std::unique_ptr<InfoRenderer>> renderers;
  cgpui::WindowRuntime runtime(application, view,
      [&](const cgpui::RenderSurfaceDescriptor&) -> cgpui::Result<cgpui::Renderer*> {
        renderers.push_back(std::make_unique<InfoRenderer>(
            static_cast<std::uint32_t>(renderers.size()) + 1));
        return renderers.back().get();
      });
  if (cgpui::Window(runtime, {}).backend_info().backend !=
      cgpui::RendererBackend::unknown) return 1;
  int failure = 0;
  application.on_run = [&] {
    cgpui::Window root(runtime, runtime.root_window_runtime_id());
    const auto root_info = root.backend_info();
    if (root_info.backend != cgpui::RendererBackend::dx12 ||
        root_info.adapter_name != "test GPU" || root_info.vendor_id != 1 ||
        root_info.adapter_luid != 101 || root_info.device_generation != 1 ||
        root_info.selection != cgpui::RendererBackendSelection::dx12) {
      failure = 2;
      return;
    }
    auto child = runtime.try_open_window(cgpui::WindowOptions{}.title("child"));
    if (!child) { failure = 3; return; }
    const cgpui::Window window(runtime, child->runtime_id);
    if (window.backend_info().vendor_id != 2 || root.backend_info().vendor_id != 1) {
      failure = 4;
      return;
    }
    application.children.back()->emit(cgpui::WindowCloseRequested{});
    if (window.backend_info().backend != cgpui::RendererBackend::unknown) failure = 5;
    application.wake();
    if (window.backend_info().backend != cgpui::RendererBackend::unknown) failure = 6;
  };
  const auto result = runtime.run({}, {.request_initial_redraw = false});
  if (result != 0) return 7;
  if (cgpui::Window(runtime, runtime.root_window_runtime_id()).backend_info().backend !=
      cgpui::RendererBackend::unknown) return 8;
  return failure;
}
