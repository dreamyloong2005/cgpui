#include "cgpui/platform/platform.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace cgpui {
namespace {

std::wstring widen(std::string_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = MultiByteToWideChar(
      CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  if (required <= 0) {
    return {};
  }

  std::wstring result(static_cast<std::size_t>(required), L'\0');
  MultiByteToWideChar(
      CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), required);
  return result;
}

class Win32Window final : public PlatformWindow {
 public:
  Win32Window(HINSTANCE instance, PlatformEventCallback callback, WindowState state)
      : instance_(instance), callback_(std::move(callback)), state_(state) {}

  ~Win32Window() override {
    if (hwnd_ != nullptr) {
      SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
      DestroyWindow(hwnd_);
      hwnd_ = nullptr;
    }
  }

  void attach(HWND hwnd) { hwnd_ = hwnd; }

  void detach() { hwnd_ = nullptr; }

  [[nodiscard]] NativeSurfaceHandle native_surface() const override {
    return Win32SurfaceHandle{.hinstance = instance_, .hwnd = hwnd_};
  }

  [[nodiscard]] WindowState state() const override { return state_; }

  void request_redraw() override { InvalidateRect(hwnd_, nullptr, FALSE); }

  void request_close() override {
    if (hwnd_ != nullptr) {
      PostMessageW(hwnd_, WM_CLOSE, 0, 0);
    }
  }

  void set_title(std::string_view title) override {
    const auto wide_title = widen(title);
    SetWindowTextW(hwnd_, wide_title.c_str());
  }

  void update_size() {
    const auto dpi = static_cast<float>(GetDpiForWindow(hwnd_));
    update_size_for_dpi(dpi);
  }

  void update_size_for_dpi(float dpi) {
    RECT rect{};
    GetClientRect(hwnd_, &rect);
    const auto width = static_cast<float>(rect.right - rect.left);
    const auto height = static_cast<float>(rect.bottom - rect.top);
    state_.framebuffer_size = Size{width, height};
    state_.scale = DpiScale{dpi / 96.0F};
    callback_(WindowResized{.size = state_.framebuffer_size, .scale = state_.scale});
  }

  void dpi_changed(WPARAM wparam, LPARAM lparam) {
    if (lparam != 0) {
      const auto* rect = reinterpret_cast<const RECT*>(lparam);
      SetWindowPos(
          hwnd_,
          nullptr,
          rect->left,
          rect->top,
          rect->right - rect->left,
          rect->bottom - rect->top,
          SWP_NOZORDER | SWP_NOACTIVATE);
    }
    update_size_for_dpi(static_cast<float>(HIWORD(wparam)));
  }

  void close_requested() {
    state_.close_requested = true;
    callback_(WindowCloseRequested{});
  }

  void redraw_requested() { callback_(WindowRedrawRequested{}); }

  void pointer_moved(LPARAM lparam) {
    callback_(PointerMoved{.position = Point{
        static_cast<float>(GET_X_LPARAM(lparam)),
        static_cast<float>(GET_Y_LPARAM(lparam))}});
  }

  void pointer_button(MouseButton button, bool pressed, LPARAM lparam) {
    callback_(PointerButton{
        .button = button,
        .pressed = pressed,
        .position = Point{
            static_cast<float>(GET_X_LPARAM(lparam)),
            static_cast<float>(GET_Y_LPARAM(lparam))}});
  }

  void pointer_scrolled(WPARAM wparam, LPARAM lparam) {
    POINT point{
        .x = GET_X_LPARAM(lparam),
        .y = GET_Y_LPARAM(lparam),
    };
    ScreenToClient(hwnd_, &point);
    callback_(PointerScrolled{
        .delta = Point{
            0.0F,
            static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) /
                static_cast<float>(WHEEL_DELTA)},
        .position = Point{
            static_cast<float>(point.x),
            static_cast<float>(point.y)}});
  }

  void key_event(WPARAM wparam, KeyAction action) {
    callback_(KeyboardKey{.key_code = static_cast<std::uint32_t>(wparam), .action = action});
  }

  void focus_changed(bool focused) { callback_(WindowFocused{.focused = focused}); }

 private:
  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
};

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

  switch (message) {
    case WM_NCCREATE: {
      const auto* create = reinterpret_cast<CREATESTRUCTW*>(lparam);
      auto* created_window = static_cast<Win32Window*>(create->lpCreateParams);
      created_window->attach(hwnd);
      SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(created_window));
      return TRUE;
    }
    case WM_SIZE:
      if (window != nullptr) {
        window->update_size();
      }
      return 0;
    case WM_DPICHANGED:
      if (window != nullptr) {
        window->dpi_changed(wparam, lparam);
      }
      return 0;
    case WM_SETFOCUS:
      if (window != nullptr) {
        window->focus_changed(true);
      }
      return 0;
    case WM_KILLFOCUS:
      if (window != nullptr) {
        window->focus_changed(false);
      }
      return 0;
    case WM_CLOSE:
      if (window != nullptr) {
        window->close_requested();
      }
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT paint{};
      BeginPaint(hwnd, &paint);
      EndPaint(hwnd, &paint);
      if (window != nullptr) {
        window->redraw_requested();
      }
      return 0;
    }
    case WM_NCDESTROY:
      if (window != nullptr) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
        window->detach();
      }
      return DefWindowProcW(hwnd, message, wparam, lparam);
    case WM_MOUSEMOVE:
      if (window != nullptr) {
        window->pointer_moved(lparam);
      }
      return 0;
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(MouseButton::left, message == WM_LBUTTONDOWN, lparam);
      }
      return 0;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(MouseButton::right, message == WM_RBUTTONDOWN, lparam);
      }
      return 0;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      if (window != nullptr) {
        window->pointer_button(MouseButton::middle, message == WM_MBUTTONDOWN, lparam);
      }
      return 0;
    case WM_MOUSEWHEEL:
      if (window != nullptr) {
        window->pointer_scrolled(wparam, lparam);
      }
      return 0;
    case WM_KEYDOWN:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::pressed);
      }
      return 0;
    case WM_KEYUP:
      if (window != nullptr) {
        window->key_event(wparam, KeyAction::released);
      }
      return 0;
    default:
      return DefWindowProcW(hwnd, message, wparam, lparam);
  }
}

class Win32Application final : public PlatformApplication {
 public:
  Win32Application() : instance_(GetModuleHandleW(nullptr)) {}

  Result<std::unique_ptr<PlatformWindow>> create_window(
      const WindowDescriptor& descriptor,
      PlatformEventCallback callback) override {
    const wchar_t* class_name = L"CGPUIWindow";

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(WNDCLASSEXW);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance_;
    window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
    window_class.lpszClassName = class_name;

    if (RegisterClassExW(&window_class) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
      return std::unexpected(Error{
          .code = ErrorCode::platform_initialization_failed,
          .message = "RegisterClassExW failed"});
    }

    auto state = WindowState{
        .framebuffer_size = descriptor.size,
        .scale = DpiScale{1.0F},
        .close_requested = false};
    auto window = std::make_unique<Win32Window>(instance_, std::move(callback), state);

    const auto title = widen(descriptor.title);
    HWND hwnd = CreateWindowExW(
        0,
        class_name,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        static_cast<int>(descriptor.size.width),
        static_cast<int>(descriptor.size.height),
        nullptr,
        nullptr,
        instance_,
        window.get());
    if (hwnd == nullptr) {
      return std::unexpected(Error{
          .code = ErrorCode::window_creation_failed,
          .message = "CreateWindowExW failed"});
    }

    ShowWindow(hwnd, SW_SHOW);
    window->update_size();
    return window;
  }

  int run() override {
    MSG message{};
    while (running_ && GetMessageW(&message, nullptr, 0, 0) > 0) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    return 0;
  }

  void quit() override {
    running_ = false;
    PostQuitMessage(0);
  }

 private:
  HINSTANCE instance_ = nullptr;
  bool running_ = true;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<Win32Application>();
}

} // namespace cgpui
