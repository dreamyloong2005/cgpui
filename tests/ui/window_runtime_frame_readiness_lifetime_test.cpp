#include "runtime_frame_readiness_test_support.hpp"

using namespace readiness_test;
using cgpui::ErrorCode;

namespace {
int child_close() {
  Fixture f;
  const int result = f.run([&] {
    const auto child = f.open_child();
    if (!child) return;
    child->outcomes = {ErrorCode::frame_not_ready};
    f.paint(child);
    if (!f.require(f.errors == 0, "pending treated as fatal")) return;
    f.app.emit(child, cgpui::WindowCloseRequested{});
    f.app.advance(20);
    f.require(f.app.pump() && child->destroyed && child->redraws == 0 &&
        child->begins == 1 && f.app.root->begins == 0,
        "closed child pending callback touched old child or fell back to root");
    const auto replacement = f.open_child();
    if (replacement) {
      f.paint(replacement);
      f.require(replacement->presents == 1, "old pending polluted replacement child");
    }
  });
  return result || f.failure ? 1 : 0;
}

int repeated_run(bool close_root) {
  Fixture f;
  std::shared_ptr<State> old_root;
  const int first = f.run([&] {
    old_root = f.app.root;
    old_root->outcomes = {ErrorCode::frame_not_ready};
    f.paint(old_root);
    if (!f.require(f.errors == 0, "pending treated as fatal")) return;
    if (close_root) f.app.emit(old_root, cgpui::WindowCloseRequested{});
  });
  if (!f.require(first == 0 && old_root && old_root->destroyed,
                 "first runtime session did not tear down normally")) return 1;
  const int second = f.run([&] {
    f.app.advance(2);
    f.require(f.app.pump() && f.app.root->begins == 0,
              "old root timer escaped into a repeated run");
    f.app.root->outcomes = {ErrorCode::frame_not_ready};
    f.paint(f.app.root);
    f.app.advance(2);
    f.require(f.app.pump() && f.app.root->begins == 2 && f.app.root->presents == 1 &&
        old_root->begins == 1, "new session readiness token was cancelled by stale work");
  });
  return second || f.failure ? 1 : 0;
}
} // namespace

int main() {
  if (child_close()) return 1;
  if (repeated_run(false)) return 2;
  if (repeated_run(true)) return 3;
  return 0;
}
