#include "../support/frame_wait_test_support_internal.hpp"

#include <deque>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace {
using cgpui::Error;
using cgpui::ErrorCode;
using cgpui_test::TestFrameWaitPolicy;
using Clock = std::chrono::steady_clock;
using Milliseconds = std::chrono::milliseconds;

struct Script {
  Clock::time_point now{};
  Milliseconds yield_advance{1}, pump_advance{0}, begin_advance{0};
  std::deque<Error> errors;
  bool forever_pending = false, null_success = false;
  unsigned begins = 0, presents = 0;
  std::vector<Clock::time_point> begin_times;
  std::vector<Milliseconds> yields;
  std::vector<unsigned> pump_budgets;
  cgpui::RenderFrame* produced = nullptr;
};

class Frame final : public cgpui::RenderFrame {
 public:
  explicit Frame(Script& script) : script_(script) {}
  void clear(cgpui::Color) override {}
  void draw_rect(const cgpui::SolidRect&) override {}
  cgpui::Result<void> present() override { ++script_.presents; return {}; }
 private:
  Script& script_;
};

class Renderer final : public cgpui::Renderer {
 public:
  explicit Renderer(Script& script) : script_(script) {}
  cgpui::Result<void> resize(cgpui::Size, cgpui::DpiScale) override { return {}; }
  cgpui::Result<std::unique_ptr<cgpui::RenderFrame>> begin_frame() override {
    ++script_.begins;
    script_.begin_times.push_back(script_.now);
    script_.now += script_.begin_advance;
    if (!script_.errors.empty()) {
      auto error = script_.errors.front();
      script_.errors.pop_front();
      return std::unexpected(std::move(error));
    }
    if (script_.forever_pending)
      return std::unexpected(Error{ErrorCode::frame_not_ready, "still pending"});
    if (script_.null_success) return std::unique_ptr<cgpui::RenderFrame>{};
    auto frame = std::make_unique<Frame>(script_);
    script_.produced = frame.get();
    return frame;
  }
 private:
  Script& script_;
};

cgpui_test::detail::FrameWaitHooks hooks(Script& script) {
  return {
      .context = &script,
      .now = [](void* context) { return static_cast<Script*>(context)->now; },
      .yield = [](void* context, Milliseconds delay) {
        auto& state = *static_cast<Script*>(context);
        state.yields.push_back(delay);
        state.now += state.yield_advance;
      },
      .pump = [](void* context, unsigned budget) {
        auto& state = *static_cast<Script*>(context);
        state.pump_budgets.push_back(budget);
        state.now += state.pump_advance;
      },
  };
}

bool require(bool condition, const char* reason) {
  if (!condition) std::cerr << "test frame wait: " << reason << '\n';
  return condition;
}
auto wait(Renderer& renderer, Script& script, TestFrameWaitPolicy policy = {}) {
  return cgpui_test::detail::wait_for_test_frame_with_hooks(renderer, policy, hooks(script));
}
bool timed_out(const cgpui::Result<std::unique_ptr<cgpui::RenderFrame>>& result) {
  return !result && result.error().code == ErrorCode::frame_acquisition_failed &&
      result.error().message == "test frame acquisition exceeded its absolute three-second deadline";
}

bool immediate_result() {
  for (const bool null_success : {false, true}) {
    Script script;
    script.null_success = null_success;
    // An already-returned non-pending result is never discarded merely because
    // begin itself took longer than the retry budget; this is not preemption.
    script.begin_advance = Milliseconds(4000);
    Renderer renderer(script);
    auto frame = wait(renderer, script);
    if (!require(frame && frame->get() == script.produced && script.begins == 1 &&
        script.yields.empty() && script.pump_budgets.empty() && script.presents == 0,
        "immediate success/null-success changed or performed unrelated work")) return false;
  }
  return true;
}

bool pending_then_ready() {
  Script script;
  script.errors = {{ErrorCode::frame_not_ready, "first"}, {ErrorCode::frame_not_ready, "second"}};
  Renderer renderer(script);
  auto frame = wait(renderer, script, {.message_owner = std::this_thread::get_id()});
  return require(frame && frame->get() == script.produced && script.begins == 3 &&
      script.yields == std::vector<Milliseconds>{Milliseconds(1), Milliseconds(1)} &&
      script.pump_budgets == std::vector<unsigned>{32, 32} && script.presents == 0,
      "pending retries must yield nonzero, bound owner dispatch, and return the actual frame");
}

bool non_pending_is_never_retried() {
  for (const auto code : {ErrorCode::invalid_argument, ErrorCode::frame_retry_required,
                          ErrorCode::frame_acquisition_failed}) {
    for (const bool after_pending : {false, true}) {
      Script script;
      if (after_pending) script.errors.push_back({ErrorCode::frame_not_ready, "pending first"});
      script.errors.push_back({code, "original fatal / recovery / overlap message"});
      Renderer renderer(script);
      auto result = wait(renderer, script, {.message_owner = std::this_thread::get_id()});
      const unsigned retried = after_pending ? 1U : 0U;
      if (!require(!result && result.error().code == code &&
          result.error().message == "original fatal / recovery / overlap message" &&
          script.begins == retried + 1 && script.yields.size() == retried &&
          script.pump_budgets.size() == retried && script.presents == 0,
          "fatal/recovery/overlap must return once with unchanged code/message")) return false;
    }
  }
  return true;
}

bool absolute_deadline() {
  Script script;
  script.forever_pending = true;
  script.yield_advance = Milliseconds(1500);
  Renderer renderer(script);
  auto result = wait(renderer, script);
  return require(timed_out(result) && script.begins == 2 &&
      script.begin_times == std::vector<Clock::time_point>{Clock::time_point{},
          Clock::time_point{} + Milliseconds(1500)} &&
      script.yields == std::vector<Milliseconds>{Milliseconds(1), Milliseconds(1)} &&
      script.now == Clock::time_point{} + Milliseconds(3000) && script.pump_budgets.empty() &&
      script.presents == 0, "deadline was refreshed or another begin started at the deadline");
}

bool callbacks_can_exhaust_deadline() {
  for (const bool in_begin : {false, true}) {
    Script script;
    script.forever_pending = true;
    (in_begin ? script.begin_advance : script.pump_advance) = Milliseconds(3000);
    Renderer renderer(script);
    auto result = wait(renderer, script, {.message_owner = std::this_thread::get_id()});
    if (!require(timed_out(result) && script.begins == 1 && script.yields.empty() &&
        script.pump_budgets.size() == (in_begin ? 0U : 1U) && script.presents == 0,
        "expired begin/pump must not yield or start another frame attempt")) return false;
  }
  return true;
}

bool worker_never_pumps_owner_messages() {
  Script script;
  script.errors.push_back({ErrorCode::frame_not_ready, "pending on worker"});
  Renderer renderer(script);
  const TestFrameWaitPolicy owner{.message_owner = std::this_thread::get_id()};
  bool result_ok = false;
  std::thread worker([&] {
    auto frame = wait(renderer, script, owner);
    result_ok = frame && frame->get() == script.produced;
  });
  worker.join();
  return require(result_ok && script.begins == 2 && script.pump_budgets.empty() &&
      script.yields == std::vector<Milliseconds>{Milliseconds(1)} && script.presents == 0,
      "worker carrying another thread's policy pumped window messages or busy-waited");
}

bool default_overload_does_not_transform_errors() {
  // Exercise the ordinary facade without sleeping; all pending timing cases
  // above use the private controlled-clock seam, never native renderer faults.
  Script script;
  script.errors.push_back({ErrorCode::frame_retry_required, "preserve recovery"});
  Renderer renderer(script);
  auto result = cgpui_test::wait_for_test_frame(renderer);
  return require(!result && result.error().code == ErrorCode::frame_retry_required &&
      result.error().message == "preserve recovery" && script.begins == 1 && script.presents == 0,
      "simple overload swallowed or retried a recovery contract");
}
} // namespace

int main() {
  if (!immediate_result()) return 1;
  if (!pending_then_ready()) return 2;
  if (!non_pending_is_never_retried()) return 3;
  if (!absolute_deadline()) return 4;
  if (!callbacks_can_exhaust_deadline()) return 5;
  if (!worker_never_pumps_owner_messages()) return 6;
  if (!default_overload_does_not_transform_errors()) return 7;
  return 0;
}
