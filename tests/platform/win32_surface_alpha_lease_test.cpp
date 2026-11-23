#include "cgpui/platform/platform.hpp"
#include "../../src/platform/win32_surface_alpha_protocol_internal.hpp"
#include "../../src/platform/win32/win32_window_backdrop_internal.hpp"

#include <dwmapi.h>

#include <iostream>
#include <thread>
#include <variant>

namespace {
using namespace cgpui;

int fail(int code, const char* message) {
  std::cerr << "win32 surface alpha lease: " << message << '\n';
  return code;
}

HWND handle(const PlatformWindow& window) {
  return static_cast<HWND>(std::get<Win32SurfaceHandle>(window.native_surface()).hwnd);
}

struct LeaseReply {
  HRESULT result = E_NOINTERFACE;
  UINT_PTR window_instance = 0;
  bool active = false;
};

LeaseReply send(HWND hwnd, UINT message, UINT_PTR instance = 0, UINT_PTR owner = 0) {
  LeaseReply reply;
  if (!GetPropW(hwnd, win32_surface_alpha_property)) return reply;
  if (message == win32_surface_alpha_query_message && !instance) {
    instance = static_cast<UINT_PTR>(SendMessageW(hwnd, win32_surface_alpha_instance_message, 0, 0));
  }
  reply.window_instance = instance;
  reply.result = static_cast<HRESULT>(SendMessageW(hwnd, message, owner, static_cast<LPARAM>(instance)));
  if (owner) reply.active = SUCCEEDED(static_cast<HRESULT>(SendMessageW(
      hwnd, win32_surface_alpha_status_message, owner, static_cast<LPARAM>(instance))));
  return reply;
}

bool reject_posted_bogus_tokens(HWND hwnd, UINT_PTR instance, UINT_PTR owner) {
  const auto before = send(hwnd, win32_surface_alpha_status_message, instance, owner);
  const auto query = send(hwnd, win32_surface_alpha_query_message);
  for (const UINT message : {win32_surface_alpha_instance_message, win32_surface_alpha_query_message,
                            win32_surface_alpha_acquire_message, win32_surface_alpha_release_message,
                            win32_surface_alpha_status_message, win32_surface_alpha_mark_opaque_message}) {
    for (const UINT_PTR cookie : {UINT_PTR{0}, owner, static_cast<UINT_PTR>(-1)}) {
      // Zero WPARAM specifically reaches the old +3 pointer-message handler:
      // the invalid LPARAM must remain a token, never a borrowed request address.
      if (!PostMessageW(hwnd, message, cookie, static_cast<LPARAM>(-1))) return false;
      MSG queued{};
      if (!PeekMessageW(&queued, hwnd, message, message, PM_REMOVE)) return false;
      DispatchMessageW(&queued);
      const auto observed = send(hwnd, win32_surface_alpha_status_message, instance, owner);
      if (observed.result != before.result || observed.active != before.active) return false;
    }
  }
  const auto after = send(hwnd, win32_surface_alpha_status_message, instance, owner);
  const auto after_query = send(hwnd, win32_surface_alpha_query_message);
  return before.result == after.result && before.active == after.active &&
      query.result == after_query.result && instance == after_query.window_instance;
}

DWORD material_type(HWND hwnd) {
  DWORD type = 0;
  if (FAILED(DwmGetWindowAttribute(hwnd, win32_system_backdrop_attribute, &type, sizeof(type)))) {
    return 0;
  }
  return type;
}
} // namespace

int main() {
  auto app = create_platform_application();
  if (!app) return fail(1, "platform unavailable");
  WindowDescriptor descriptor;
  descriptor.title = "Surface alpha lease contract";
  descriptor.size = {220, 120};
  auto first = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  auto second = (*app)->create_window(descriptor, [](const PlatformEvent&) {});
  if (!first || !second) return fail(2, "windows unavailable");
  const HWND hwnd = handle(**first), other = handle(**second);
  const auto query = send(hwnd, win32_surface_alpha_query_message);
  const auto other_query = send(other, win32_surface_alpha_query_message);
  if (!reject_posted_bogus_tokens(hwnd, query.window_instance, 11)) {
    return fail(16, "posted invalid wire tokens changed initial state");
  }
  if (query.result == E_NOINTERFACE || !query.window_instance ||
      query.window_instance == other_query.window_instance) {
    return fail(3, "protocol publication/unique window identity");
  }
  if (send(hwnd, win32_surface_alpha_acquire_message, other_query.window_instance, 11).result != E_INVALIDARG ||
      send(hwnd, win32_surface_alpha_acquire_message, query.window_instance, 0).result != E_INVALIDARG) {
    return fail(4, "stale identity and zero owner must reject");
  }
  if (send(other, win32_surface_alpha_mark_opaque_message,
            other_query.window_instance, 22).result != S_OK ||
      send(other, win32_surface_alpha_query_message).result != E_NOTIMPL ||
      send(other, win32_surface_alpha_acquire_message,
            other_query.window_instance, 22).result != E_NOTIMPL) {
    return fail(5, "opaque HWND history must permanently gate alpha");
  }
  auto client = descriptor.chrome;
  client.title_bar = WindowTitleBarMode::client;
  (*first)->apply_window_chrome(client);
  if (send(hwnd, win32_surface_alpha_query_message).result != E_NOTIMPL) {
    return fail(6, "client caption must not advertise ordinary alpha");
  }
  (*first)->apply_window_chrome(descriptor.chrome);
  if (query.result == E_NOTIMPL) {
    std::cout << "SKIP: OS/topology does not support ordinary alpha; identity/history gates passed\n";
    return 0;
  }
  const auto original_backdrop = (*first)->window_backdrop_state();
  const DWORD original_material = material_type(hwnd);
  const auto acquired = send(hwnd, win32_surface_alpha_acquire_message, query.window_instance, 11);
  if (FAILED(acquired.result) || !acquired.active) return fail(7, "attribute-backed lease acquire");
  if (!reject_posted_bogus_tokens(hwnd, query.window_instance, 11)) {
    return fail(20, "posted invalid wire tokens changed active owner");
  }
  if ((*first)->window_backdrop_state().requested != original_backdrop.requested ||
      (*first)->window_backdrop_state().applied != original_backdrop.applied ||
      (*first)->window_backdrop_state().mechanism != original_backdrop.mechanism ||
      material_type(hwnd) != original_material) {
    return fail(17, "alpha acquisition altered independent backdrop state");
  }
  // Backdrop none/material changes and the user's transparency-effects setting
  // may change DWM material/frame extension, but must not own the alpha lease.
  (*first)->apply_window_backdrop(WindowBackdrop::mica, true);
  auto appearance = (*app)->system_appearance();
  appearance.transparency_effects_enabled = false;
  win32_reevaluate_thread_window_backdrops(appearance);
  if ((*first)->window_backdrop_state().applied != WindowBackdrop::none ||
      !send(hwnd, win32_surface_alpha_status_message, query.window_instance, 11).active) {
    return fail(18, "effects preference took ownership of alpha lease");
  }
  (*first)->apply_window_backdrop(WindowBackdrop::none, true);
  if (!send(hwnd, win32_surface_alpha_status_message, query.window_instance, 11).active) {
    return fail(19, "backdrop none cleared independent alpha lease");
  }
  if (send(hwnd, win32_surface_alpha_acquire_message, query.window_instance, 33).result != HRESULT_FROM_WIN32(ERROR_BUSY) ||
      send(hwnd, win32_surface_alpha_mark_opaque_message,
            query.window_instance, 33).result != HRESULT_FROM_WIN32(ERROR_BUSY)) {
    return fail(8, "active owner must reject competing alpha and opaque owners");
  }
  if (send(hwnd, win32_surface_alpha_release_message, query.window_instance, 33).result != E_INVALIDARG ||
      !send(hwnd, win32_surface_alpha_status_message, query.window_instance, 11).active) {
    return fail(9, "foreign release must not touch active lease");
  }
  const LONG_PTR style = GetWindowLongPtrW(hwnd, GWL_STYLE);
  const auto rejected = (*first)->apply_window_chrome(client);
  if (rejected.applied.title_bar != WindowTitleBarMode::system || rejected.reason.empty() ||
      GetWindowLongPtrW(hwnd, GWL_STYLE) != style ||
      (*first)->request_display_state(PlatformWindowDisplayState::fullscreen)) {
    return fail(10, "unvalidated chrome/fullscreen must preserve active topology");
  }
  bool posted = false;
  std::thread releaser([&] {
    posted = PostMessageW(hwnd, win32_surface_alpha_release_message, 11,
                          static_cast<LPARAM>(query.window_instance)) != FALSE;
  });
  releaser.join();
  MSG message{};
  if (!posted || !PeekMessageW(&message, hwnd, win32_surface_alpha_release_message,
                               win32_surface_alpha_release_message, PM_REMOVE)) {
    return fail(11, "foreign-thread token release not queued");
  }
  DispatchMessageW(&message);
  if (send(hwnd, win32_surface_alpha_status_message, query.window_instance, 11).result != E_INVALIDARG)
    return fail(12, "queued release did not retire lease");
  if (FAILED(send(hwnd, win32_surface_alpha_acquire_message, query.window_instance, 44).result)) {
    return fail(13, "released window cannot reacquire alpha");
  }
  // A queued stale-owner token must never release a replacement owner.
  SendMessageW(hwnd, win32_surface_alpha_release_message, 11, query.window_instance);
  if (!send(hwnd, win32_surface_alpha_status_message, query.window_instance, 44).active) {
    return fail(14, "stale release affected a replacement owner");
  }
  (*first).reset();
  if (GetPropW(hwnd, win32_surface_alpha_property) != nullptr) return fail(15, "destroyed HWND retained lease property");
  return 0;
}
