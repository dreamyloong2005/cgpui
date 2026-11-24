#include "native_window_work_test_support.hpp"

using namespace cgpui;
using namespace cgpui_test;
int main() {
  Mailbox box;
  auto work = std::make_shared<Work>();
  const auto token = native_window_work_register(&box, work);
  bool wrong = false;
  work->action = [&] {
    if (work->calls == 1) {
      wrong |= !native_window_work_post(token);
      wrong |= box.dispatch() != NativeWindowWorkDispatch::deferred; // Nested pump.
      wrong |= work->calls != 1;
    }
    return NativeWindowWorkResult::complete;
  };
  if (!native_window_work_post(token) || box.dispatch() != NativeWindowWorkDispatch::complete ||
      wrong || box.messages.size() != 1 || work->calls != 1) return 1;
  if (box.dispatch() != NativeWindowWorkDispatch::complete || work->calls != 2 ||
      !box.messages.empty()) return 2; // Only the explicit nested wake was requeued.

  work->action = [] { return NativeWindowWorkResult::deferred; };
  if (!native_window_work_post(token) || box.dispatch() != NativeWindowWorkDispatch::deferred ||
      !box.messages.empty()) return 3; // Busy does not create a message hotloop.
  work->action = [&] {
    wrong |= !native_window_work_post(token); // Outer operation can request while run is active.
    return NativeWindowWorkResult::deferred;
  };
  if (!native_window_work_post(token) || box.dispatch() != NativeWindowWorkDispatch::deferred ||
      wrong || box.messages.size() != 1) return 4;
  work->action = [&] {
    native_window_work_cancel(token); // No registry lock is held during run.
    box.close();
    return NativeWindowWorkResult::complete;
  };
  if (box.dispatch() != NativeWindowWorkDispatch::complete || !box.messages.empty() ||
      native_window_work_post(token)) return 5;

  Mailbox last_box;
  auto last_owner = std::make_shared<Work>(), other = std::make_shared<Work>();
  const auto last_token = native_window_work_register(&last_box, last_owner);
  bool destroyed = false;
  last_owner->action = [&] { last_owner.reset(); return NativeWindowWorkResult::complete; };
  last_owner->on_destroy = [&] {
    destroyed = true;
    const auto added = native_window_work_register(&last_box, other);
    wrong |= !bool(added); // Last strong release may reenter the registry too.
    native_window_work_cancel(added);
  };
  if (!native_window_work_post(last_token) || last_box.dispatch() != NativeWindowWorkDispatch::complete ||
      !destroyed || wrong || native_window_work_post(last_token)) return 6;
  return 0;
}
