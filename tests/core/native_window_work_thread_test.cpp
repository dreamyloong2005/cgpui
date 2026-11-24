#include "native_window_work_test_support.hpp"
#include <thread>

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Mailbox box;
  auto work = std::make_shared<Work>();
  const auto owner = std::this_thread::get_id();
  bool wrong_owner = false;
  work->action = [&] { wrong_owner |= std::this_thread::get_id() != owner;
    return NativeWindowWorkResult::complete; };
  const auto token = native_window_work_register(&box, work);
  if (!token) return 1;
  NativeWindowWorkToken registered_elsewhere;
  bool posted = false, dispatched_elsewhere = false;
  std::thread sender([&] {
    registered_elsewhere = native_window_work_register(&box, work);
    native_window_work_close(&box, box.instance); // Only the endpoint owner can close it.
    posted = native_window_work_post(registered_elsewhere);
    dispatched_elsewhere = native_window_work_dispatch(&box, registered_elsewhere) != NativeWindowWorkDispatch::ignored;
  });
  sender.join();
  if (!registered_elsewhere || !posted || dispatched_elsewhere || work->calls ||
      box.messages.size() != 1) return 2;
  if (box.dispatch() != NativeWindowWorkDispatch::complete || work->calls != 1 || wrong_owner) return 3;
  std::thread cancel([&] { native_window_work_cancel(token); native_window_work_cancel(registered_elsewhere); });
  cancel.join();
  if (native_window_work_post(token) || native_window_work_post(registered_elsewhere)) return 4;
  return 0;
}
