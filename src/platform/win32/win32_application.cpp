#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <shellapi.h>
#include <imm.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

constexpr UINT cgpui_wakeup_message = WM_APP + 1U;

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

std::string utf8_from_utf16(std::wstring_view value) {
  if (value.empty()) {
    return {};
  }

  const auto required = WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      nullptr,
      0,
      nullptr,
      nullptr);
  if (required <= 0) {
    return {};
  }

  std::string result(static_cast<std::size_t>(required), '\0');
  WideCharToMultiByte(
      CP_UTF8,
      0,
      value.data(),
      static_cast<int>(value.size()),
      result.data(),
      required,
      nullptr,
      nullptr);
  return result;
}

struct Win32TestDragDropPayload {
  float x = 0.0F;
  float y = 0.0F;
  const wchar_t* text = nullptr;
  const wchar_t* const* files = nullptr;
  std::size_t file_count = 0;
  std::uint32_t drop_effect = 0;
};

UINT test_drag_enter_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragEnter");
  return message;
}

UINT test_drag_update_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragUpdate");
  return message;
}

UINT test_drag_drop_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragDrop");
  return message;
}

UINT test_drag_exit_message() {
  static const UINT message =
      RegisterWindowMessageW(L"CGPUI.Win32.TestDragExit");
  return message;
}

DragDropPayload drag_payload_from_test_hook(
    const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return {};
  }

  DragDropPayload result;
  if (payload->files != nullptr && payload->file_count > 0) {
    result.kind = DragDropPayloadKind::files;
    result.files.reserve(payload->file_count);
    for (std::size_t index = 0; index < payload->file_count; ++index) {
      if (payload->files[index] != nullptr) {
        result.files.push_back(utf8_from_utf16(payload->files[index]));
      }
    }
    return result;
  }
  if (payload->text != nullptr && payload->text[0] != L'\0') {
    result.kind = DragDropPayloadKind::text;
    result.text = utf8_from_utf16(payload->text);
  }
  return result;
}

Point drag_position_from_test_hook(const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return {};
  }
  return Point{.x = payload->x, .y = payload->y};
}

DragDropAction drag_action_from_drop_effect(DWORD drop_effect) {
  if ((drop_effect & DROPEFFECT_MOVE) != 0) {
    return DragDropAction::move;
  }
  if ((drop_effect & DROPEFFECT_COPY) != 0) {
    return DragDropAction::copy;
  }
  return DragDropAction::none;
}

DragDropAction drag_action_from_test_hook(
    const Win32TestDragDropPayload* payload) {
  if (payload == nullptr) {
    return DragDropAction::none;
  }
  return drag_action_from_drop_effect(payload->drop_effect);
}

DWORD choose_ole_drop_effect(DWORD allowed_effect, DWORD key_state) {
  if ((key_state & MK_CONTROL) != 0 &&
      (allowed_effect & DROPEFFECT_COPY) != 0) {
    return DROPEFFECT_COPY;
  }
  if ((key_state & MK_SHIFT) != 0 &&
      (allowed_effect & DROPEFFECT_MOVE) != 0) {
    return DROPEFFECT_MOVE;
  }
  if ((allowed_effect & DROPEFFECT_COPY) != 0) {
    return DROPEFFECT_COPY;
  }
  if ((allowed_effect & DROPEFFECT_MOVE) != 0) {
    return DROPEFFECT_MOVE;
  }
  return DROPEFFECT_NONE;
}

DragDropPayload drag_payload_from_ole_data_object(IDataObject* data_object) {
  if (data_object == nullptr) {
    return {};
  }

  FORMATETC text_format{
      .cfFormat = CF_UNICODETEXT,
      .ptd = nullptr,
      .dwAspect = DVASPECT_CONTENT,
      .lindex = -1,
      .tymed = TYMED_HGLOBAL,
  };
  STGMEDIUM text_storage{};
  if (SUCCEEDED(data_object->GetData(&text_format, &text_storage))) {
    DragDropPayload payload;
    if (text_storage.tymed == TYMED_HGLOBAL &&
        text_storage.hGlobal != nullptr) {
      const auto* text =
          static_cast<const wchar_t*>(GlobalLock(text_storage.hGlobal));
      if (text != nullptr && text[0] != L'\0') {
        payload.kind = DragDropPayloadKind::text;
        payload.text = utf8_from_utf16(text);
      }
      if (text != nullptr) {
        GlobalUnlock(text_storage.hGlobal);
      }
    }
    ReleaseStgMedium(&text_storage);
    if (payload.kind != DragDropPayloadKind::none) {
      return payload;
    }
  }

  FORMATETC file_format{
      .cfFormat = CF_HDROP,
      .ptd = nullptr,
      .dwAspect = DVASPECT_CONTENT,
      .lindex = -1,
      .tymed = TYMED_HGLOBAL,
  };
  STGMEDIUM file_storage{};
  if (SUCCEEDED(data_object->GetData(&file_format, &file_storage))) {
    DragDropPayload payload;
    if (file_storage.tymed == TYMED_HGLOBAL &&
        file_storage.hGlobal != nullptr) {
      auto* drop_handle = reinterpret_cast<HDROP>(file_storage.hGlobal);
      const UINT file_count =
          DragQueryFileW(drop_handle, 0xFFFFFFFFU, nullptr, 0);
      if (file_count > 0) {
        payload.kind = DragDropPayloadKind::files;
        payload.files.reserve(file_count);
        for (UINT index = 0; index < file_count; ++index) {
          const UINT length = DragQueryFileW(drop_handle, index, nullptr, 0);
          std::wstring file_path(static_cast<std::size_t>(length) + 1U, L'\0');
          DragQueryFileW(
              drop_handle,
              index,
              file_path.data(),
              static_cast<UINT>(file_path.size()));
          file_path.resize(length);
          if (!file_path.empty()) {
            payload.files.push_back(utf8_from_utf16(file_path));
          }
        }
      }
    }
    ReleaseStgMedium(&file_storage);
    return payload;
  }

  return {};
}

struct Win32OleDropTargetRegistrationState {
  HWND hwnd = nullptr;
  bool registered = false;
  HRESULT last_registration_result = S_FALSE;
  HRESULT last_revocation_result = S_FALSE;
};

class Win32Window;

class Win32OleDropTarget final : public IDropTarget {
 public:
  explicit Win32OleDropTarget(Win32Window& owner) : owner_(&owner) {}

  HRESULT STDMETHODCALLTYPE QueryInterface(
      REFIID interface_id,
      void** object) override;
  ULONG STDMETHODCALLTYPE AddRef() override;
  ULONG STDMETHODCALLTYPE Release() override;
  HRESULT STDMETHODCALLTYPE DragEnter(
      IDataObject* data_object,
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragOver(
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;
  HRESULT STDMETHODCALLTYPE DragLeave() override;
  HRESULT STDMETHODCALLTYPE Drop(
      IDataObject* data_object,
      DWORD key_state,
      POINTL point,
      DWORD* effect) override;

 private:
  std::atomic_ulong reference_count_{1};
  Win32Window* owner_ = nullptr;
};

class Win32UiaAccessibilityAdapter {
 public:
  void update(PlatformAccessibilityTreeUpdate update) {
    last_update_ = std::move(update);
    root_element_id_ = last_update_.root_element_id;
    node_count_ = last_update_.node_count;
    focused_node_count_ = last_update_.focused_node_count;
    text_input_node_count_ = 0;
    for (const PlatformAccessibilityNodeUpdate& node : last_update_.nodes) {
      if (node.role == PlatformAccessibilityRole::text_input) {
        text_input_node_count_ += 1;
      }
    }
  }

  [[nodiscard]] std::uint64_t root_element_id() const {
    return root_element_id_;
  }

  [[nodiscard]] std::size_t node_count() const {
    return node_count_;
  }

  [[nodiscard]] std::size_t focused_node_count() const {
    return focused_node_count_;
  }

  [[nodiscard]] std::size_t text_input_node_count() const {
    return text_input_node_count_;
  }

 private:
  PlatformAccessibilityTreeUpdate last_update_;
  std::uint64_t root_element_id_ = 0;
  std::size_t node_count_ = 0;
  std::size_t focused_node_count_ = 0;
  std::size_t text_input_node_count_ = 0;
};

class Win32NativeMenuState {
 public:
  PlatformMenuInstallationResult install_native_menu(NativeMenuModel menu) {
    model_ = std::move(menu);
    last_menu_installation_ = PlatformMenuInstallationResult{
        .supported = false,
        .backend = "win32",
        .menu_count = model_.items.size(),
        .item_count = native_menu_item_count(model_),
        .accelerator_count = native_menu_accelerator_count(model_),
    };
    return last_menu_installation_;
  }

  [[nodiscard]] const PlatformMenuInstallationResult& last_menu_installation()
      const {
    return last_menu_installation_;
  }

 private:
  NativeMenuModel model_;
  PlatformMenuInstallationResult last_menu_installation_;
};

KeyboardModifiers current_modifiers() {
  return KeyboardModifiers{
      .shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0,
      .control = (GetKeyState(VK_CONTROL) & 0x8000) != 0,
      .alt = (GetKeyState(VK_MENU) & 0x8000) != 0,
      .super = (GetKeyState(VK_LWIN) & 0x8000) != 0 ||
          (GetKeyState(VK_RWIN) & 0x8000) != 0,
  };
}

const wchar_t* cursor_id_for(CursorShape cursor_shape) {
  switch (cursor_shape) {
    case CursorShape::pointing_hand:
      return MAKEINTRESOURCEW(32649);
    case CursorShape::text:
      return MAKEINTRESOURCEW(32513);
    case CursorShape::crosshair:
      return MAKEINTRESOURCEW(32515);
    case CursorShape::resize_left_right:
      return MAKEINTRESOURCEW(32644);
    case CursorShape::resize_up_down:
      return MAKEINTRESOURCEW(32645);
    case CursorShape::not_allowed:
      return MAKEINTRESOURCEW(32648);
    case CursorShape::default_arrow:
      return MAKEINTRESOURCEW(32512);
  }
  return MAKEINTRESOURCEW(32512);
}

class Win32Window final : public PlatformWindow {
 public:
  Win32Window(HINSTANCE instance, PlatformEventCallback callback, WindowState state)
      : instance_(instance),
        callback_(std::move(callback)),
        state_(state),
        ole_drop_target_(std::make_unique<Win32OleDropTarget>(*this)) {}

  ~Win32Window() override {
    if (hwnd_ != nullptr) {
      revoke_drop_target();
      SetWindowLongPtrW(hwnd_, GWLP_USERDATA, 0);
      DestroyWindow(hwnd_);
      hwnd_ = nullptr;
    }
  }

  void attach(HWND hwnd) {
    hwnd_ = hwnd;
    register_drop_target(hwnd);
  }

  void detach() {
    revoke_drop_target();
    hwnd_ = nullptr;
  }

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

  void set_cursor(CursorShape cursor_shape) override {
    HCURSOR cursor = LoadCursorW(nullptr, cursor_id_for(cursor_shape));
    if (cursor == nullptr) {
      cursor = LoadCursorW(nullptr, cursor_id_for(CursorShape::default_arrow));
    }
    current_cursor_ = cursor;
    if (hwnd_ != nullptr) {
      SetClassLongPtrW(hwnd_, GCLP_HCURSOR, reinterpret_cast<LONG_PTR>(cursor));
      SetCursor(cursor);
    }
  }

  void set_ime_text_input_placement(
      std::optional<ImeTextInputPlacement> placement) override {
    state_.ime_text_input_placement = placement;
    apply_ime_text_input_placement();
  }

  void update_accessibility_tree(
      PlatformAccessibilityTreeUpdate update) override {
    uia_accessibility_.update(std::move(update));
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

  void wakeup_requested() { callback_(WindowWakeupRequested{}); }

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
    callback_(KeyboardKey{
        .key_code = static_cast<std::uint32_t>(wparam),
        .action = action,
        .modifiers = current_modifiers()});
  }

  void text_input(WPARAM wparam) {
    const wchar_t character = static_cast<wchar_t>(wparam);
    auto text = utf8_from_utf16(std::wstring_view(&character, 1));
    if (!text.empty()) {
      callback_(TextInput{
          .text = std::move(text),
          .modifiers = current_modifiers()});
    }
  }

  void focus_changed(bool focused) { callback_(WindowFocused{.focused = focused}); }

  void ime_start_composition() { apply_ime_text_input_placement(); }

  void drag_entered(const Win32TestDragDropPayload* payload) {
    callback_(DragEntered{
        .position = drag_position_from_test_hook(payload),
        .payload = drag_payload_from_test_hook(payload),
        .action = drag_action_from_test_hook(payload)});
  }

  void drag_updated(const Win32TestDragDropPayload* payload) {
    callback_(DragUpdated{
        .position = drag_position_from_test_hook(payload),
        .payload = drag_payload_from_test_hook(payload),
        .action = drag_action_from_test_hook(payload)});
  }

  void drag_dropped(const Win32TestDragDropPayload* payload) {
    callback_(DragDropped{
        .position = drag_position_from_test_hook(payload),
        .payload = drag_payload_from_test_hook(payload),
        .action = drag_action_from_test_hook(payload)});
  }

  void drag_exited(const Win32TestDragDropPayload* payload) {
    callback_(DragExited{
        .position = drag_position_from_test_hook(payload),
        .payload = {},
        .action = DragDropAction::none});
  }

  void ole_drag_entered(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) {
    const DWORD selected_effect =
        choose_ole_drop_effect(
            effect != nullptr ? *effect : DROPEFFECT_NONE,
            key_state);
    if (effect != nullptr) {
      *effect = selected_effect;
    }
    last_ole_drag_position_ = client_position_from_screen(point);
    last_ole_drag_payload_ = drag_payload_from_ole_data_object(data_object);
    callback_(DragEntered{
        .position = last_ole_drag_position_,
        .payload = last_ole_drag_payload_,
        .action = drag_action_from_drop_effect(selected_effect)});
  }

  void ole_drag_updated(POINTL point, DWORD key_state, DWORD* effect) {
    const DWORD selected_effect =
        choose_ole_drop_effect(
            effect != nullptr ? *effect : DROPEFFECT_NONE,
            key_state);
    if (effect != nullptr) {
      *effect = selected_effect;
    }
    last_ole_drag_position_ = client_position_from_screen(point);
    callback_(DragUpdated{
        .position = last_ole_drag_position_,
        .payload = last_ole_drag_payload_,
        .action = drag_action_from_drop_effect(selected_effect)});
  }

  void ole_drag_exited() {
    callback_(DragExited{
        .position = last_ole_drag_position_,
        .payload = {},
        .action = DragDropAction::none});
    last_ole_drag_payload_ = {};
  }

  void ole_drag_dropped(
      IDataObject* data_object,
      POINTL point,
      DWORD key_state,
      DWORD* effect) {
    const DWORD selected_effect =
        choose_ole_drop_effect(
            effect != nullptr ? *effect : DROPEFFECT_NONE,
            key_state);
    if (effect != nullptr) {
      *effect = selected_effect;
    }
    last_ole_drag_position_ = client_position_from_screen(point);
    last_ole_drag_payload_ = drag_payload_from_ole_data_object(data_object);
    callback_(DragDropped{
        .position = last_ole_drag_position_,
        .payload = last_ole_drag_payload_,
        .action = drag_action_from_drop_effect(selected_effect)});
    last_ole_drag_payload_ = {};
  }

 private:
  void register_drop_target(HWND hwnd) {
    if (ole_drop_target_ == nullptr || hwnd == nullptr ||
        ole_drop_target_registration_.registered) {
      return;
    }
    ole_drop_target_registration_.hwnd = hwnd;
    ole_drop_target_registration_.last_registration_result =
        RegisterDragDrop(hwnd, ole_drop_target_.get());
    ole_drop_target_registration_.registered =
        SUCCEEDED(ole_drop_target_registration_.last_registration_result);
  }

  void revoke_drop_target() {
    if (!ole_drop_target_registration_.registered ||
        ole_drop_target_registration_.hwnd == nullptr) {
      return;
    }
    ole_drop_target_registration_.last_revocation_result =
        RevokeDragDrop(ole_drop_target_registration_.hwnd);
    ole_drop_target_registration_.registered = false;
    ole_drop_target_registration_.hwnd = nullptr;
  }

  Point client_position_from_screen(POINTL point) const {
    POINT screen_point{
        .x = static_cast<LONG>(point.x),
        .y = static_cast<LONG>(point.y),
    };
    if (hwnd_ != nullptr) {
      ScreenToClient(hwnd_, &screen_point);
    }
    return Point{
        .x = static_cast<float>(screen_point.x),
        .y = static_cast<float>(screen_point.y)};
  }

  void apply_ime_text_input_placement() {
    if (hwnd_ == nullptr || !state_.ime_text_input_placement.has_value()) {
      return;
    }

    const ImeTextInputPlacement& placement =
        *state_.ime_text_input_placement;
    const POINT point{
        .x = static_cast<LONG>(placement.rect.origin.x),
        .y = static_cast<LONG>(placement.rect.origin.y),
    };
    const auto height = static_cast<LONG>(placement.rect.size.height);
    const RECT area{
        .left = point.x,
        .top = point.y,
        .right = point.x + static_cast<LONG>(placement.rect.size.width),
        .bottom = point.y + height,
    };

    HIMC context = ImmGetContext(hwnd_);
    if (context == nullptr) {
      return;
    }

    COMPOSITIONFORM composition{};
    composition.dwStyle = CFS_POINT;
    composition.ptCurrentPos = point;
    ImmSetCompositionWindow(context, &composition);

    CANDIDATEFORM candidate{};
    candidate.dwIndex = 0;
    candidate.dwStyle = CFS_EXCLUDE;
    candidate.ptCurrentPos = point;
    candidate.rcArea = area;
    ImmSetCandidateWindow(context, &candidate);

    ImmReleaseContext(hwnd_, context);
  }

  HINSTANCE instance_ = nullptr;
  HWND hwnd_ = nullptr;
  HCURSOR current_cursor_ = nullptr;
  PlatformEventCallback callback_;
  WindowState state_;
  Win32UiaAccessibilityAdapter uia_accessibility_;
  std::unique_ptr<Win32OleDropTarget> ole_drop_target_;
  Win32OleDropTargetRegistrationState ole_drop_target_registration_;
  Point last_ole_drag_position_{};
  DragDropPayload last_ole_drag_payload_;
};

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::QueryInterface(
    REFIID interface_id,
    void** object) {
  if (object == nullptr) {
    return E_POINTER;
  }
  if (IsEqualIID(interface_id, IID_IUnknown) ||
      IsEqualIID(interface_id, IID_IDropTarget)) {
    *object = static_cast<IDropTarget*>(this);
    AddRef();
    return S_OK;
  }
  *object = nullptr;
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Win32OleDropTarget::AddRef() {
  return static_cast<ULONG>(reference_count_.fetch_add(1) + 1U);
}

ULONG STDMETHODCALLTYPE Win32OleDropTarget::Release() {
  const auto count = reference_count_.fetch_sub(1) - 1U;
  return static_cast<ULONG>(count);
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragEnter(
    IDataObject* data_object,
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_entered(data_object, point, key_state, effect);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragOver(
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_updated(point, key_state, effect);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::DragLeave() {
  if (owner_ != nullptr) {
    owner_->ole_drag_exited();
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE Win32OleDropTarget::Drop(
    IDataObject* data_object,
    DWORD key_state,
    POINTL point,
    DWORD* effect) {
  if (owner_ != nullptr) {
    owner_->ole_drag_dropped(data_object, point, key_state, effect);
  }
  return S_OK;
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  auto* window = reinterpret_cast<Win32Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
  const auto* drag_payload =
      reinterpret_cast<const Win32TestDragDropPayload*>(lparam);

  if (message == test_drag_enter_message()) {
    if (window != nullptr) {
      window->drag_entered(drag_payload);
    }
    return 0;
  }
  if (message == test_drag_update_message()) {
    if (window != nullptr) {
      window->drag_updated(drag_payload);
    }
    return 0;
  }
  if (message == test_drag_drop_message()) {
    if (window != nullptr) {
      window->drag_dropped(drag_payload);
    }
    return 0;
  }
  if (message == test_drag_exit_message()) {
    if (window != nullptr) {
      window->drag_exited(drag_payload);
    }
    return 0;
  }

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
    case WM_IME_STARTCOMPOSITION:
      if (window != nullptr) {
        window->ime_start_composition();
      }
      return DefWindowProcW(hwnd, message, wparam, lparam);
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
    case WM_CHAR:
      if (window != nullptr) {
        window->text_input(wparam);
      }
      return 0;
    default:
      return DefWindowProcW(hwnd, message, wparam, lparam);
  }
}

class Win32Application final : public PlatformApplication {
 public:
  Win32Application() : instance_(GetModuleHandleW(nullptr)) {
    ole_initialization_result_ = OleInitialize(nullptr);
    ole_initialized_ = SUCCEEDED(ole_initialization_result_);
  }

  ~Win32Application() override {
    if (ole_initialized_) {
      OleUninitialize();
    }
  }

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
        .close_requested = false,
        .ime_text_input_support = ImeTextInputSupport::available};
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
    windows_.push_back(window.get());
    return window;
  }

  int run() override {
    running_thread_id_ = GetCurrentThreadId();
    MSG message{};
    while (running_ && GetMessageW(&message, nullptr, 0, 0) > 0) {
      if (message.message == cgpui_wakeup_message) {
        dispatch_wakeup();
        continue;
      }
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    running_thread_id_ = 0;
    windows_.clear();
    return 0;
  }

  void request_wakeup() override {
    if (running_thread_id_ != 0) {
      PostThreadMessageW(running_thread_id_, cgpui_wakeup_message, 0, 0);
    }
  }

  PlatformMenuInstallationResult install_native_menu(
      NativeMenuModel menu) override {
    last_menu_installation_ =
        native_menu_state_.install_native_menu(std::move(menu));
    return last_menu_installation_;
  }

  void quit() override {
    running_ = false;
    PostQuitMessage(0);
  }

  [[nodiscard]] std::vector<FontFaceDescriptor> discover_font_records()
      const override {
    return {
        FontFaceDescriptor{
            .font = FontDescriptor{.family = "Segoe UI"},
            .postscript_name = "SegoeUI",
            .source = FontSource::platform,
            .path = "win32://Segoe UI",
        },
    };
  }

 private:
  void dispatch_wakeup() {
    for (Win32Window* window : windows_) {
      if (window != nullptr) {
        window->wakeup_requested();
      }
    }
  }

  HINSTANCE instance_ = nullptr;
  HRESULT ole_initialization_result_ = S_FALSE;
  bool ole_initialized_ = false;
  Win32NativeMenuState native_menu_state_;
  PlatformMenuInstallationResult last_menu_installation_;
  DWORD running_thread_id_ = 0;
  bool running_ = true;
  std::vector<Win32Window*> windows_;
};

} // namespace

Result<std::unique_ptr<PlatformApplication>> create_platform_application() {
  return std::make_unique<Win32Application>();
}

} // namespace cgpui
