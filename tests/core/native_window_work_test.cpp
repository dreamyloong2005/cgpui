#include "native_window_work_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  int unmanaged = 0;
  auto work = std::make_shared<Work>();
  if (native_window_work_register(&unmanaged, work)) return 1;
  Mailbox box;
  if (!box.instance || native_window_work_open(&box, Mailbox::enqueue)) return 2;
  const auto token = native_window_work_register(&box, work);
  if (!token || !native_window_work_post(token) || !native_window_work_post(token) ||
      box.posts != 1 || box.messages.size() != 1) return 3;
  if (native_window_work_dispatch(&unmanaged, token) != NativeWindowWorkDispatch::ignored ||
      box.dispatch() != NativeWindowWorkDispatch::complete || work->calls != 1 ||
      native_window_work_dispatch(&box, token) != NativeWindowWorkDispatch::ignored) return 4;
  if (!native_window_work_post(token)) return 5;
  native_window_work_cancel(token);
  if (box.dispatch() != NativeWindowWorkDispatch::ignored || native_window_work_post(token)) return 6;

  const auto expired = native_window_work_register(&box, work);
  work.reset();
  if (native_window_work_post(expired)) return 7; // Registration never owns the callback.
  work = std::make_shared<Work>();
  const auto stale = native_window_work_register(&box, work);
  if (!native_window_work_post(stale)) return 8;
  const auto old_instance = box.instance;
  box.close(); box.open(); // Reuse the exact native handle value, not its instance.
  const auto fresh = native_window_work_register(&box, work);
  if (!fresh || box.instance == old_instance || native_window_work_post(stale) ||
      box.dispatch() != NativeWindowWorkDispatch::ignored) return 9;
  box.accepts = false;
  if (native_window_work_post(fresh) || !box.messages.empty()) return 10;
  box.accepts = true;
  if (!native_window_work_post(fresh) || box.dispatch() != NativeWindowWorkDispatch::complete ||
      work->calls != 1) return 11; // Failed enqueue did not leave a phantom pending wake.
  native_window_work_close(&unmanaged, box.instance); // Wrong HWND cannot close this endpoint.
  if (!native_window_work_post(fresh)) return 12;
  box.close();
  if (box.dispatch() != NativeWindowWorkDispatch::ignored) return 13;
  return 0;
}
