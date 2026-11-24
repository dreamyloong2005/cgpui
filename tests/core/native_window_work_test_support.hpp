#pragma once
#include "../../src/core/native_window_work_internal.hpp"
#include <deque>
#include <functional>
#include <memory>

namespace cgpui_test {
struct Work final : cgpui::NativeWindowWork {
  unsigned calls = 0;
  std::function<cgpui::NativeWindowWorkResult()> action;
  std::function<void()> on_destroy;
  ~Work() override { if (on_destroy) on_destroy(); }
  cgpui::NativeWindowWorkResult run() noexcept override {
    ++calls;
    return action ? action() : cgpui::NativeWindowWorkResult::complete;
  }
};
struct Mailbox {
  std::deque<cgpui::NativeWindowWorkToken> messages;
  unsigned posts = 0;
  bool accepts = true;
  std::uintptr_t instance = 0;
  Mailbox() { open(); }
  ~Mailbox() { close(); }
  Mailbox(const Mailbox&) = delete;
  Mailbox& operator=(const Mailbox&) = delete;
  static bool enqueue(void* window, cgpui::NativeWindowWorkToken token) noexcept {
    auto& self = *static_cast<Mailbox*>(window);
    ++self.posts;
    if (!self.accepts) return false;
    try { self.messages.push_back(token); return true; } catch (...) { return false; }
  }
  void open() { instance = cgpui::native_window_work_open(this, enqueue); }
  void close() { cgpui::native_window_work_close(this, instance); instance = 0; }
  cgpui::NativeWindowWorkToken pop() {
    const auto token = messages.front(); messages.pop_front(); return token;
  }
  cgpui::NativeWindowWorkDispatch dispatch() {
    return cgpui::native_window_work_dispatch(this, pop());
  }
};
} // namespace cgpui_test
