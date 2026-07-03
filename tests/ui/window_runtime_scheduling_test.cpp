#include "window_runtime_test_support.hpp"

namespace {
RuntimeFixture* deferred_callback_fixture = nullptr;
class TimerApiView;
TimerApiView* timer_api_view = nullptr;
cgpui::WindowRuntime* timer_api_runtime = nullptr;
class AnimationApiView;
AnimationApiView* animation_api_view = nullptr;
cgpui::WindowRuntime* animation_api_runtime = nullptr;
class AsyncTaskApiView;
AsyncTaskApiView* async_task_api_view = nullptr;
cgpui::WindowRuntime* async_task_api_runtime = nullptr;
class ThreadedAsyncExecutorView;
ThreadedAsyncExecutorView* threaded_async_executor_view = nullptr;
cgpui::WindowRuntime* threaded_async_executor_runtime = nullptr;
RuntimeFixture* platform_wakeup_fixture = nullptr;
cgpui::WindowRuntime* platform_wakeup_runtime = nullptr;
class PlatformWakeupQueueView;
PlatformWakeupQueueView* platform_wakeup_view = nullptr;

void dispatch_deferred_callback_sequence() {
  auto& callback = deferred_callback_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 68,
      .action = cgpui::KeyAction::pressed});
}

class DeferredCallbackView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (!std::holds_alternative<cgpui::KeyboardKey>(event)) {
      return cgpui::EventResult::unhandled();
    }

    event_count += 1;
    context.defer(
        [this](const cgpui::WindowRuntimeContext& deferred_context) {
          deferred_count += 1;
          first_deferred_order = next_deferred_order++;
          first_deferred_saw_dispatch =
              deferred_context.last_event_dispatch.has_value();
          first_deferred_event_kind =
              deferred_context.last_event_dispatch.has_value()
                  ? deferred_context.last_event_dispatch->event_kind
                  : cgpui::EventKind::unknown;
          deferred_context.request_render();
        });
    context.defer(
        [this](const cgpui::WindowRuntimeContext& deferred_context) {
          deferred_count += 1;
          second_deferred_order = next_deferred_order++;
          second_deferred_saw_invalidation =
              deferred_context.runtime.invalidation_state().render;
        });
    return cgpui::EventResult::consumed_event();
  }

  int paint_count = 0;
  int event_count = 0;
  int deferred_count = 0;
  int next_deferred_order = 1;
  int first_deferred_order = 0;
  int second_deferred_order = 0;
  bool after_event_saw_no_deferred = false;
  bool first_deferred_saw_dispatch = false;
  cgpui::EventKind first_deferred_event_kind = cgpui::EventKind::unknown;
  bool second_deferred_saw_invalidation = false;
};

int test_deferred_callbacks_run_after_event_before_redraw_fifo() {
  RuntimeFixture fixture;
  DeferredCallbackView view;
  deferred_callback_fixture = &fixture;
  fixture.app.on_run = &dispatch_deferred_callback_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext&,
          const cgpui::EventDispatchRecord&) {
        view.after_event_saw_no_deferred = view.deferred_count == 0;
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  deferred_callback_fixture = nullptr;

  if (result != 0) {
    return 363;
  }
  if (view.event_count != 1 || view.deferred_count != 2 ||
      !view.after_event_saw_no_deferred) {
    return 364;
  }
  if (view.first_deferred_order != 1 || view.second_deferred_order != 2) {
    return 365;
  }
  if (!view.first_deferred_saw_dispatch ||
      view.first_deferred_event_kind != cgpui::EventKind::keyboard_key) {
    return 366;
  }
  if (!view.second_deferred_saw_invalidation) {
    return 367;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 ||
      view.paint_count != 1) {
    return 368;
  }

  return 0;
}

class TimerApiView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  int timer_order = 0;
  int one_shot_count = 0;
  int one_shot_order = 0;
  int repeating_count = 0;
  int repeating_last_order = 0;
  int paint_count = 0;
  bool repeating_second_saw_invalidation = false;
};

void dispatch_timer_api_sequence() {
  timer_api_runtime->advance_time(4);
  if (timer_api_view->one_shot_count != 0 ||
      timer_api_view->repeating_count != 0) {
    return;
  }

  timer_api_runtime->advance_time(1);
  if (timer_api_view->one_shot_count != 0 ||
      timer_api_view->repeating_count != 1 ||
      timer_api_view->repeating_last_order != 1) {
    return;
  }

  timer_api_runtime->advance_time(5);
  if (timer_api_view->one_shot_count != 1 ||
      timer_api_view->repeating_count != 2 ||
      timer_api_view->one_shot_order != 2 ||
      timer_api_view->repeating_last_order != 3 ||
      !timer_api_view->repeating_second_saw_invalidation) {
    return;
  }

  timer_api_runtime->advance_time(10);
}

int test_one_shot_and_repeating_timers_tick_deterministically() {
  RuntimeFixture fixture;
  TimerApiView view;
  timer_api_view = &view;
  fixture.app.on_run = &dispatch_timer_api_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  timer_api_runtime = &runtime;

  const cgpui::TimerId one_shot = runtime.schedule_timer(
      10,
      [&](const cgpui::WindowRuntimeContext& context) {
        view.one_shot_count += 1;
        view.one_shot_order = ++view.timer_order;
        context.request_render();
      });
  const cgpui::TimerId repeating = runtime.schedule_repeating_timer(
      5,
      [&](const cgpui::WindowRuntimeContext& context) {
        view.repeating_count += 1;
        view.repeating_last_order = ++view.timer_order;
        if (view.repeating_count == 2) {
          view.repeating_second_saw_invalidation =
              context.runtime.invalidation_state().render;
        }
      });

  if (one_shot.value == 0 || repeating.value == 0 || one_shot == repeating) {
    return 369;
  }

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  timer_api_view = nullptr;
  timer_api_runtime = nullptr;
  if (result != 0) {
    return 370;
  }

  if (view.one_shot_count != 1 || view.repeating_count != 4 ||
      view.repeating_last_order != 5) {
    return 374;
  }

  if (!runtime.cancel_timer(repeating) || runtime.cancel_timer(one_shot)) {
    return 375;
  }

  runtime.advance_time(20);
  if (view.one_shot_count != 1 || view.repeating_count != 4) {
    return 376;
  }

  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 ||
      view.paint_count != 1) {
    return 377;
  }

  return 0;
}

class AnimationApiView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  cgpui::AnimationHandle animation;
  int tick_count = 0;
  int paint_count = 0;
  bool start_snapshot_is_zero = false;
  bool quarter_snapshot_matches_easing = false;
  bool half_snapshot_matches_easing = false;
  bool completion_snapshot_matches = false;
  bool handle_reports_completion = false;
  bool callback_saw_context_and_requested_redraw = false;
  float last_eased_progress = 0.0F;
};

void dispatch_animation_api_sequence() {
  const std::optional<cgpui::AnimationSnapshot> start =
      animation_api_runtime->animation_snapshot(animation_api_view->animation.id());
  animation_api_view->start_snapshot_is_zero =
      start.has_value() && !start->complete &&
      start->linear_progress == 0.0F && start->eased_progress == 0.0F;

  animation_api_runtime->advance_time(25);
  const std::optional<cgpui::AnimationSnapshot> quarter =
      animation_api_runtime->animation_snapshot(animation_api_view->animation.id());
  animation_api_view->quarter_snapshot_matches_easing =
      quarter.has_value() && !quarter->complete &&
      quarter->elapsed_ms == 25 && quarter->duration_ms == 100 &&
      quarter->linear_progress == 0.25F &&
      quarter->eased_progress == 0.125F;

  animation_api_runtime->advance_time(25);
  const std::optional<cgpui::AnimationSnapshot> half =
      animation_api_view->animation.progress();
  animation_api_view->half_snapshot_matches_easing =
      half.has_value() && !half->complete &&
      half->linear_progress == 0.5F && half->eased_progress == 0.5F;

  animation_api_runtime->advance_time(50);
  const std::optional<cgpui::AnimationSnapshot> completed =
      animation_api_runtime->animation_snapshot(animation_api_view->animation.id());
  animation_api_view->completion_snapshot_matches =
      completed.has_value() && completed->complete &&
      completed->elapsed_ms == 100 &&
      completed->linear_progress == 1.0F &&
      completed->eased_progress == 1.0F;
  animation_api_view->handle_reports_completion =
      !animation_api_view->animation.active() &&
      animation_api_view->animation.complete();

  const int ticks_after_completion = animation_api_view->tick_count;
  animation_api_runtime->advance_time(100);
  if (animation_api_view->tick_count != ticks_after_completion) {
    animation_api_view->handle_reports_completion = false;
  }
}

int test_runtime_animation_clock_ticks_tweens_and_redraws() {
  RuntimeFixture fixture;
  AnimationApiView view;
  animation_api_view = &view;
  fixture.app.on_run = &dispatch_animation_api_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  animation_api_runtime = &runtime;

  view.animation = runtime.start_animation(
      cgpui::AnimationOptions{
          .duration_ms = 100,
          .easing = cgpui::AnimationEasing::ease_in_out,
          .tick_interval_ms = 16,
      },
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::AnimationSnapshot& snapshot) {
        view.tick_count += 1;
        view.last_eased_progress = snapshot.eased_progress;
        view.callback_saw_context_and_requested_redraw =
            context.window_runtime_id == runtime.root_window_runtime_id();
        context.request_render();
      });
  if (view.animation.id().value == 0 || !view.animation.active() ||
      view.animation.complete()) {
    return 387;
  }

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  animation_api_view = nullptr;
  animation_api_runtime = nullptr;
  if (result != 0) {
    return 388;
  }
  if (!view.start_snapshot_is_zero || !view.quarter_snapshot_matches_easing ||
      !view.half_snapshot_matches_easing ||
      !view.completion_snapshot_matches ||
      !view.handle_reports_completion) {
    return 389;
  }
  if (view.tick_count != 3 || view.last_eased_progress != 1.0F ||
      !view.callback_saw_context_and_requested_redraw) {
    return 390;
  }
  if (fixture.window.request_redraw_count != 3 ||
      fixture.renderer.begin_frame_count != 3 || view.paint_count != 3) {
    return 391;
  }
  if (runtime.cancel_animation(view.animation.id())) {
    return 392;
  }
  return 0;
}

class AsyncTaskApiView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  int completion_order = 0;
  int first_completion_order = 0;
  int second_completion_order = 0;
  int paint_count = 0;
  bool first_saw_no_prior_invalidation = false;
  bool second_saw_first_invalidation = false;
};

void dispatch_async_task_completion_sequence() {
  const cgpui::TaskHandle first = async_task_api_runtime->spawn_task(
      [](const cgpui::WindowRuntimeContext& context) {
        async_task_api_view->first_completion_order =
            ++async_task_api_view->completion_order;
        async_task_api_view->first_saw_no_prior_invalidation =
            !context.runtime.invalidation_state().render;
        context.request_render();
      });
  const cgpui::TaskHandle second = async_task_api_runtime->spawn_task(
      [](const cgpui::WindowRuntimeContext& context) {
        async_task_api_view->second_completion_order =
            ++async_task_api_view->completion_order;
        async_task_api_view->second_saw_first_invalidation =
            context.runtime.invalidation_state().render;
      });

  if (first.id().value == 0 || second.id().value == 0 ||
      first.id() == second.id() || !first.active() || !second.active()) {
    return;
  }
  if (!async_task_api_runtime->complete_task(first.id()) ||
      !async_task_api_runtime->complete_task(second.id())) {
    return;
  }
  if (async_task_api_view->completion_order != 0 || first.active() ||
      second.active()) {
    return;
  }

  async_task_api_runtime->drain_task_completions();
  if (!first.complete() || !second.complete()) {
    return;
  }
  if (async_task_api_runtime->complete_task(first.id())) {
    return;
  }
}

int test_async_task_completion_dispatches_on_runtime_queue() {
  RuntimeFixture fixture;
  AsyncTaskApiView view;
  async_task_api_view = &view;
  fixture.app.on_run = &dispatch_async_task_completion_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  async_task_api_runtime = &runtime;

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  async_task_api_view = nullptr;
  async_task_api_runtime = nullptr;
  if (result != 0) {
    return 378;
  }

  if (view.completion_order != 2 || view.first_completion_order != 1 ||
      view.second_completion_order != 2) {
    return 379;
  }
  if (!view.first_saw_no_prior_invalidation ||
      !view.second_saw_first_invalidation) {
    return 380;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 ||
      view.paint_count != 1) {
    return 381;
  }

  return 0;
}

class ThreadedAsyncExecutorView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  cgpui::TaskHandle completed_task;
  cgpui::TaskHandle cancelled_task;
  std::atomic<int> worker_start_count = 0;
  std::atomic<int> worker_finish_count = 0;
  std::atomic<int> cancellation_seen_count = 0;
  std::atomic<bool> allow_completed_task_to_finish = false;
  int completion_count = 0;
  int paint_count = 0;
  bool completion_saw_background_result = false;
  bool cancelled_completion_ran = false;
  bool active_diagnostics_match = false;
  bool cancellation_diagnostics_match = false;
  bool completion_diagnostics_match = false;
};

void dispatch_threaded_async_executor_sequence() {
  ThreadedAsyncExecutorView& view = *threaded_async_executor_view;
  cgpui::WindowRuntime& runtime = *threaded_async_executor_runtime;

  view.completed_task = runtime.spawn_background_task(
      [](cgpui::TaskCancellationToken token) {
        threaded_async_executor_view->worker_start_count.fetch_add(1);
        while (!threaded_async_executor_view->allow_completed_task_to_finish
                    .load()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        if (!token.cancellation_requested()) {
          threaded_async_executor_view->worker_finish_count.fetch_add(1);
        }
      },
      [](const cgpui::WindowRuntimeContext& context) {
        threaded_async_executor_view->completion_count += 1;
        threaded_async_executor_view->completion_saw_background_result =
            threaded_async_executor_view->worker_finish_count.load() == 1;
        context.request_render();
      });
  view.cancelled_task = runtime.spawn_background_task(
      [](cgpui::TaskCancellationToken token) {
        threaded_async_executor_view->worker_start_count.fetch_add(1);
        while (!token.cancellation_requested()) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        threaded_async_executor_view->cancellation_seen_count.fetch_add(1);
      },
      [](const cgpui::WindowRuntimeContext&) {
        threaded_async_executor_view->cancelled_completion_ran = true;
      });

  for (int attempt = 0; attempt < 200 &&
                        view.worker_start_count.load() != 2;
       ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  const cgpui::RuntimeDiagnosticsSnapshot active =
      runtime.diagnostics_snapshot();
  view.active_diagnostics_match =
      view.completed_task.id().value != 0 &&
      view.cancelled_task.id().value != 0 &&
      view.completed_task.active() && view.cancelled_task.active() &&
      active.task_count == 2 && active.background_task_count == 2 &&
      active.active_task_count == 2 && active.completed_task_count == 0 &&
      active.cancelled_task_count == 0 && active.queued_task_count == 0;

  const bool cancelled = view.cancelled_task.cancel();
  for (int attempt = 0; attempt < 200 &&
                        view.cancellation_seen_count.load() != 1;
       ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  const cgpui::RuntimeDiagnosticsSnapshot after_cancel =
      runtime.diagnostics_snapshot();
  view.cancellation_diagnostics_match =
      cancelled && !view.cancelled_task.active() &&
      view.cancelled_task.cancelled() && !view.cancelled_task.complete() &&
      !runtime.complete_task(view.cancelled_task.id()) &&
      after_cancel.task_count == 2 && after_cancel.background_task_count == 2 &&
      after_cancel.active_task_count == 1 &&
      after_cancel.cancelled_task_count == 1 &&
      after_cancel.completed_task_count == 0;

  view.allow_completed_task_to_finish.store(true);
  for (int attempt = 0; attempt < 200 &&
                        view.worker_finish_count.load() != 1;
       ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  for (int attempt = 0; attempt < 200 && view.completed_task.active();
       ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  runtime.drain_task_completions();
  const cgpui::RuntimeDiagnosticsSnapshot after_completion =
      runtime.diagnostics_snapshot();
  view.completion_diagnostics_match =
      view.completed_task.complete() && !view.completed_task.active() &&
      view.cancelled_task.cancelled() &&
      after_completion.task_count == 2 &&
      after_completion.background_task_count == 2 &&
      after_completion.active_task_count == 0 &&
      after_completion.queued_task_count == 0 &&
      after_completion.completed_task_count == 1 &&
      after_completion.cancelled_task_count == 1;
}

int test_threaded_async_executor_runs_cancels_and_dispatches_completion() {
  RuntimeFixture fixture;
  ThreadedAsyncExecutorView view;
  threaded_async_executor_view = &view;
  fixture.app.on_run = &dispatch_threaded_async_executor_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  threaded_async_executor_runtime = &runtime;

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  threaded_async_executor_view = nullptr;
  threaded_async_executor_runtime = nullptr;
  if (result != 0) {
    return 393;
  }
  if (!view.active_diagnostics_match || !view.cancellation_diagnostics_match ||
      !view.completion_diagnostics_match) {
    return 394;
  }
  if (view.worker_start_count.load() != 2 ||
      view.worker_finish_count.load() != 1 ||
      view.cancellation_seen_count.load() != 1) {
    return 395;
  }
  if (view.completion_count != 1 || view.cancelled_completion_ran ||
      !view.completion_saw_background_result) {
    return 396;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 || view.paint_count != 1) {
    return 397;
  }
  return 0;
}

class PlatformWakeupQueueView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {
    paint_count += 1;
  }

  int task_count = 0;
  int timer_count = 0;
  int deferred_count = 0;
  int paint_count = 0;
  bool queued_work_stayed_pending_before_wakeup = false;
  bool wakeup_drained_all_queues = false;
  bool task_saw_no_timer_or_defer = false;
  bool timer_saw_task_before_defer = false;
  bool deferred_saw_prior_queues = false;
};

void dispatch_platform_wakeup_sequence() {
  platform_wakeup_runtime->defer(
      [](const cgpui::WindowRuntimeContext& context) {
        platform_wakeup_view->deferred_count += 1;
        platform_wakeup_view->deferred_saw_prior_queues =
            platform_wakeup_view->task_count == 1 &&
            platform_wakeup_view->timer_count == 1;
        context.request_render();
      });
  const cgpui::TimerId timer = platform_wakeup_runtime->schedule_timer(
      0,
      [](const cgpui::WindowRuntimeContext& context) {
        platform_wakeup_view->timer_count += 1;
        platform_wakeup_view->timer_saw_task_before_defer =
            platform_wakeup_view->task_count == 1 &&
            platform_wakeup_view->deferred_count == 0;
        context.request_paint();
      });
  const cgpui::TaskHandle task = platform_wakeup_runtime->spawn_task(
      [](const cgpui::WindowRuntimeContext& context) {
        platform_wakeup_view->task_count += 1;
        platform_wakeup_view->task_saw_no_timer_or_defer =
            platform_wakeup_view->timer_count == 0 &&
            platform_wakeup_view->deferred_count == 0;
        context.request_layout();
      });

  if (timer.value == 0 || task.id().value == 0 ||
      !platform_wakeup_runtime->complete_task(task.id())) {
    return;
  }
  platform_wakeup_view->queued_work_stayed_pending_before_wakeup =
      platform_wakeup_view->task_count == 0 &&
      platform_wakeup_view->timer_count == 0 &&
      platform_wakeup_view->deferred_count == 0;
  if (platform_wakeup_fixture->app.request_wakeup_count < 3 ||
      !platform_wakeup_fixture->app.wakeup_pending) {
    return;
  }

  platform_wakeup_fixture->app.dispatch_wakeup();
  platform_wakeup_view->wakeup_drained_all_queues =
      platform_wakeup_view->task_count == 1 &&
      platform_wakeup_view->timer_count == 1 &&
      platform_wakeup_view->deferred_count == 1;
}

int test_runtime_async_timer_and_defer_request_platform_wakeup() {
  RuntimeFixture fixture;
  PlatformWakeupQueueView view;
  platform_wakeup_fixture = &fixture;
  platform_wakeup_view = &view;
  fixture.app.on_run = &dispatch_platform_wakeup_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  platform_wakeup_runtime = &runtime;

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  platform_wakeup_fixture = nullptr;
  platform_wakeup_runtime = nullptr;
  platform_wakeup_view = nullptr;
  if (result != 0) {
    return 382;
  }
  if (!view.queued_work_stayed_pending_before_wakeup ||
      !view.wakeup_drained_all_queues) {
    return 383;
  }
  if (!view.task_saw_no_timer_or_defer ||
      !view.timer_saw_task_before_defer ||
      !view.deferred_saw_prior_queues) {
    return 384;
  }
  if (fixture.app.request_wakeup_count < 3 || fixture.app.wakeup_pending) {
    return 385;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 || view.paint_count != 1) {
    return 386;
  }

  return 0;
}

RuntimeFixture* invalidation_fixture = nullptr;

void dispatch_invalidation_sequence() {
  auto& callback = invalidation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 73,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_tracks_layout_and_paint_invalidation_requests() {
  RuntimeFixture fixture;
  invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_invalidation_sequence;
  fixture.view.exercise_invalidation_requests = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::InvalidationState callback_invalidation{};
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord&) {
        callback_invalidation = context.runtime.invalidation_state();
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  invalidation_fixture = nullptr;

  if (result != 0) {
    return 178;
  }
  if (fixture.view.initial_invalidation.render ||
      fixture.view.initial_invalidation.layout ||
      fixture.view.initial_invalidation.paint) {
    return 179;
  }
  if (fixture.view.after_layout_request_invalidation.render ||
      !fixture.view.after_layout_request_invalidation.layout ||
      !fixture.view.after_layout_request_invalidation.paint) {
    return 180;
  }
  if (fixture.view.after_paint_request_invalidation.render ||
      !fixture.view.after_paint_request_invalidation.layout ||
      !fixture.view.after_paint_request_invalidation.paint) {
    return 181;
  }
  if (fixture.view.after_clear_invalidation.render ||
      fixture.view.after_clear_invalidation.layout ||
      fixture.view.after_clear_invalidation.paint) {
    return 182;
  }
  if (runtime.invalidation_state().render ||
      runtime.invalidation_state().layout ||
      runtime.invalidation_state().paint || callback_invalidation.layout ||
      !callback_invalidation.paint || callback_invalidation.render) {
    return 183;
  }

  return 0;
}

RuntimeFixture* scheduled_invalidation_fixture = nullptr;

void dispatch_scheduled_invalidation_sequence() {
  auto& callback = scheduled_invalidation_fixture->window.callback;
  callback(cgpui::KeyboardKey{
      .key_code = 82,
      .action = cgpui::KeyAction::pressed});
}

int test_runtime_schedules_redraw_for_invalidation_requests() {
  RuntimeFixture fixture;
  scheduled_invalidation_fixture = &fixture;
  fixture.app.on_run = &dispatch_scheduled_invalidation_sequence;
  fixture.view.exercise_scheduled_invalidation_redraw = true;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  cgpui::InvalidationState after_frame_invalidation{
      .layout = true,
      .paint = true,
  };
  runtime.set_after_frame_callback(
      [&](const cgpui::WindowRuntimeContext& context) {
        after_frame_invalidation = context.runtime.invalidation_state();
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  scheduled_invalidation_fixture = nullptr;

  if (result != 0) {
    return 184;
  }
  if (fixture.window.request_redraw_count != 1) {
    return 185;
  }
  if (fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1 || fixture.frame.present_count != 1) {
    return 186;
  }
  if (runtime.invalidation_state().layout ||
      runtime.invalidation_state().paint || after_frame_invalidation.layout ||
      after_frame_invalidation.paint) {
    return 187;
  }

  return 0;
}

struct BatchedRuntimeGlobal {
  int value = 0;
};

RuntimeFixture* update_batch_fixture = nullptr;
cgpui::WindowRuntime* update_batch_runtime = nullptr;
int update_batch_redraws_during_callback = -1;
bool update_batch_model_update = false;
bool update_batch_global_update = false;
bool update_batch_saw_render_invalidation = false;

void dispatch_update_batch_sequence() {
  auto& runtime = *update_batch_runtime;
  runtime.batch_updates(
      [&](const cgpui::ViewContext& batch_context) {
        const cgpui::Model<RuntimeEntity> model =
            batch_context.new_model<RuntimeEntity>(10);
        batch_context.subscribe_view_to_entity(batch_context.view_id, model);
        update_batch_model_update = batch_context.update_model(
            model,
            [](RuntimeEntity& entity) {
              entity.value += 5;
            });
        batch_context.set_global(BatchedRuntimeGlobal{.value = 7});
        update_batch_global_update =
            batch_context.update_global<BatchedRuntimeGlobal>(
                [](BatchedRuntimeGlobal& global) {
                  global.value += 3;
                });
        update_batch_saw_render_invalidation =
            batch_context.runtime.invalidation_state().render;
        update_batch_redraws_during_callback =
            update_batch_fixture->window.request_redraw_count;
      });
}

int test_runtime_batches_model_and_global_update_redraws() {
  RuntimeFixture fixture;
  update_batch_fixture = &fixture;
  fixture.app.on_run = &dispatch_update_batch_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  update_batch_runtime = &runtime;

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  update_batch_runtime = nullptr;
  update_batch_fixture = nullptr;
  if (result != 0) {
    return 382;
  }
  if (!update_batch_model_update || !update_batch_global_update) {
    return 383;
  }
  const BatchedRuntimeGlobal* global =
      runtime.global<BatchedRuntimeGlobal>();
  if (global == nullptr || global->value != 10) {
    return 384;
  }
  if (!update_batch_saw_render_invalidation ||
      update_batch_redraws_during_callback != 0) {
    return 385;
  }
  if (fixture.window.request_redraw_count != 1 ||
      fixture.renderer.begin_frame_count != 1 ||
      fixture.view.paint_count != 1) {
    return 386;
  }

  return 0;
}

cgpui::WindowRuntime* diagnostics_runtime = nullptr;
cgpui::Subscription diagnostics_subscription;
cgpui::RuntimeDiagnosticsSnapshot diagnostics_context_snapshot{};

void dispatch_diagnostics_snapshot_sequence() {
  auto& runtime = *diagnostics_runtime;
  runtime.batch_updates(
      [](const cgpui::ViewContext& context) {
        const cgpui::Model<RuntimeEntity> first_model =
            context.new_model<RuntimeEntity>(1);
        const cgpui::Model<RuntimeEntity> second_model =
            context.new_model<RuntimeEntity>(2);
        context.subscribe_view_to_entity(context.view_id, first_model);
        context.subscribe_view_to_entity(context.view_id, second_model);
        (void)context.observe_model(
            first_model,
            [](const cgpui::ViewContext&, cgpui::Model<RuntimeEntity>) {});
        diagnostics_subscription = context.observe_model_subscription(
            second_model,
            [](const cgpui::ViewContext&, cgpui::Model<RuntimeEntity>) {});
        (void)diagnostics_subscription.connected();
        (void)context.update_model(
            first_model,
            [](RuntimeEntity& entity) {
              entity.value += 10;
            });
        diagnostics_context_snapshot = context.diagnostics_snapshot();
      });
}

int test_runtime_reports_public_diagnostics_snapshot() {
  RuntimeFixture fixture;
  fixture.app.on_run = &dispatch_diagnostics_snapshot_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  diagnostics_runtime = &runtime;

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  diagnostics_runtime = nullptr;
  if (result != 0) {
    (void)diagnostics_subscription.release();
    return 387;
  }

  const cgpui::RuntimeDiagnosticsSnapshot after_run =
      runtime.diagnostics_snapshot();
  if (diagnostics_context_snapshot.entity_store_count != 1 ||
      diagnostics_context_snapshot.entity_count != 2 ||
      diagnostics_context_snapshot.view_entity_subscription_count != 2 ||
      diagnostics_context_snapshot.entity_observer_count != 2 ||
      diagnostics_context_snapshot.connected_subscription_count != 1) {
    return 388;
  }
  if (!diagnostics_context_snapshot.invalidation.render ||
      !diagnostics_context_snapshot.invalidation.layout ||
      !diagnostics_context_snapshot.invalidation.paint) {
    return 389;
  }
  if (diagnostics_context_snapshot.frame_index != 0 ||
      diagnostics_context_snapshot.last_render_record.has_value()) {
    return 390;
  }
  if (after_run.entity_store_count != 1 || after_run.entity_count != 2 ||
      after_run.view_entity_subscription_count != 2 ||
      after_run.entity_observer_count != 2 ||
      after_run.connected_subscription_count != 1) {
    return 391;
  }
  if (after_run.invalidation.render || after_run.invalidation.layout ||
      after_run.invalidation.paint || after_run.frame_index != 1) {
    return 392;
  }
  if (!after_run.last_render_record.has_value() ||
      after_run.last_render_record->sequence != 1 ||
      after_run.last_render_record->view_id != cgpui::ViewId{1}) {
    (void)diagnostics_subscription.release();
    return 393;
  }

  (void)diagnostics_subscription.release();
  return 0;
}

RuntimeFixture* platform_diagnostics_fixture = nullptr;

void dispatch_platform_diagnostics_sequence() {
  auto& callback = platform_diagnostics_fixture->window.callback;
  callback(cgpui::WindowActivated{.active = true});
  callback(cgpui::DragEntered{
      .position = {5.0F, 5.0F},
      .payload =
          cgpui::DragDropPayload{
              .kind = cgpui::DragDropPayloadKind::text,
              .text = "Dragged text",
          },
      .action = cgpui::DragDropAction::copy,
  });
  callback(cgpui::DragDropped{
      .position = {6.0F, 5.0F},
      .payload =
          cgpui::DragDropPayload{
              .kind = cgpui::DragDropPayloadKind::files,
              .files = {"C:\\Temp\\first.txt", "C:\\Temp\\second.cpp"},
          },
      .action = cgpui::DragDropAction::move,
  });
  callback(cgpui::KeyboardKey{
      .key_code = 84,
      .action = cgpui::KeyAction::pressed});
  callback(cgpui::WindowCloseRequested{});
}

int test_runtime_collects_platform_diagnostics_stream() {
  RuntimeFixture fixture;
  platform_diagnostics_fixture = &fixture;
  fixture.app.on_run = &dispatch_platform_diagnostics_sequence;

  cgpui::TextModel model("abcd");
  model.set_selection(1, 3);
  auto tree = std::make_unique<cgpui::ElementTree>();
  const cgpui::ElementId input_id =
      tree->set_root(cgpui::text_input(model).font_size(20.0F).build());
  fixture.view.focused_keyboard_element_id = input_id;
  fixture.view.request_keyboard_focus_element_on_first_key = true;

  cgpui::MemoryClipboard clipboard;
  (void)clipboard.write_text("!");
  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });
  runtime.set_element_tree(std::move(tree));
  runtime.set_clipboard(&clipboard);

  bool copied = false;
  bool pasted = false;
  runtime.set_after_event_callback(
      [&](const cgpui::WindowRuntimeContext& context,
          const cgpui::EventDispatchRecord& record) {
        if (record.event_kind == cgpui::EventKind::keyboard_key) {
          copied = context.runtime.copy_selection_to_clipboard();
          pasted = context.runtime.paste_clipboard_text();
        }
      });

  const int result = runtime.run(cgpui::WindowDescriptor{});
  platform_diagnostics_fixture = nullptr;

  if (result != 0) {
    return 520;
  }
  if (!copied || !pasted) {
    return 521;
  }
  if (fixture.window.accessibility_update_count == 0 ||
      fixture.window.ime_placement_count == 0) {
    return 522;
  }

  const cgpui::RuntimeDiagnosticsSnapshot snapshot =
      runtime.diagnostics_snapshot();
  const std::span<const cgpui::PlatformDiagnosticEvent> direct_stream =
      runtime.platform_diagnostics();
  if (snapshot.platform_diagnostics.empty() ||
      snapshot.platform_diagnostics.size() != direct_stream.size()) {
    return 523;
  }

  if (!has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::accessibility,
          "update-tree") ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::ime,
          "set-placement") ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::clipboard,
          "copy-selection") ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::clipboard,
          "paste-text") ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::drag_drop,
          "drag-entered",
          cgpui::EventKind::drag_entered) ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::drag_drop,
          "drag-dropped",
          cgpui::EventKind::drag_dropped) ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::window_lifecycle,
          "window-lifecycle",
          cgpui::EventKind::window_activated) ||
      !has_platform_diagnostic(
          direct_stream,
          cgpui::PlatformDiagnosticKind::window_lifecycle,
          "window-lifecycle",
          cgpui::EventKind::window_close_requested)) {
    return 524;
  }

  const auto accessibility = std::ranges::find_if(
      direct_stream,
      [](const cgpui::PlatformDiagnosticEvent& event) {
        return event.kind == cgpui::PlatformDiagnosticKind::accessibility &&
               event.operation == "update-tree";
      });
  if (accessibility == direct_stream.end() || !accessibility->succeeded ||
      accessibility->value_count == 0 || accessibility->sequence <= 0) {
    return 525;
  }

  return 0;
}

RuntimeFixture* platform_diagnostics_bound_fixture = nullptr;

void dispatch_platform_diagnostics_bound_sequence() {
  auto& callback = platform_diagnostics_bound_fixture->window.callback;
  for (int index = 0; index < 40; ++index) {
    callback(cgpui::DragUpdated{
        .position = {static_cast<float>(index), 1.0F},
        .payload =
            cgpui::DragDropPayload{
                .kind = cgpui::DragDropPayloadKind::text,
                .text = "drag",
            },
        .action = cgpui::DragDropAction::copy,
    });
  }
}

int test_runtime_bounds_platform_diagnostics_stream() {
  RuntimeFixture fixture;
  platform_diagnostics_bound_fixture = &fixture;
  fixture.app.on_run = &dispatch_platform_diagnostics_bound_sequence;

  cgpui::WindowRuntime runtime(
      fixture.app,
      fixture.view,
      [&](const cgpui::RenderSurfaceDescriptor&) {
        return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
      });

  const int result =
      runtime.run(cgpui::WindowDescriptor{},
                  cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
  platform_diagnostics_bound_fixture = nullptr;

  if (result != 0) {
    return 526;
  }
  const std::span<const cgpui::PlatformDiagnosticEvent> events =
      runtime.platform_diagnostics();
  const cgpui::RuntimeDiagnosticsSnapshot snapshot =
      runtime.diagnostics_snapshot();
  if (events.size() != 32 || snapshot.platform_diagnostics.size() != 32) {
    return 527;
  }
  if (events.front().sequence != 9 || events.back().sequence != 40) {
    return 528;
  }
  for (const cgpui::PlatformDiagnosticEvent& event : events) {
    if (event.kind != cgpui::PlatformDiagnosticKind::drag_drop ||
        event.operation != "drag-updated" ||
        event.event_kind != cgpui::EventKind::drag_updated ||
        !event.succeeded || event.value_count != 1) {
      return 529;
    }
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_deferred_callbacks_run_after_event_before_redraw_fifo(); result != 0) {
    return result;
  }
  if (const int result = test_one_shot_and_repeating_timers_tick_deterministically(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_animation_clock_ticks_tweens_and_redraws(); result != 0) {
    return result;
  }
  if (const int result = test_async_task_completion_dispatches_on_runtime_queue(); result != 0) {
    return result;
  }
  if (const int result = test_threaded_async_executor_runs_cancels_and_dispatches_completion(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_async_timer_and_defer_request_platform_wakeup(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_tracks_layout_and_paint_invalidation_requests(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_schedules_redraw_for_invalidation_requests(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_batches_model_and_global_update_redraws(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_reports_public_diagnostics_snapshot(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_collects_platform_diagnostics_stream(); result != 0) {
    return result;
  }
  if (const int result = test_runtime_bounds_platform_diagnostics_stream(); result != 0) {
    return result;
  }
  return 0;
}
