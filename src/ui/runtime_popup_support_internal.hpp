#pragma once

namespace cgpui {

class PlatformApplication;
class Renderer;
struct WindowRuntimeRecord;

// Independent popups need both native window support and an opted-in
// renderer. The caller supplies the actual parent or newly created renderer.
[[nodiscard]] bool runtime_popup_windows_supported(
    const PlatformApplication& application, const Renderer* renderer);

// Ordinary child windows pass unchanged. Refused popups record an error;
// callers keep their existing native-window and view ownership handling.
[[nodiscard]] bool runtime_accept_popup_renderer(
    WindowRuntimeRecord& record, const PlatformApplication& application,
    const Renderer* renderer);

} // namespace cgpui
