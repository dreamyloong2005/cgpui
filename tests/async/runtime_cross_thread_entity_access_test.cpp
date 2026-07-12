#include "cgpui/ui/cross_thread_entity.hpp"
#include "window_runtime_test_support.hpp"

#include <array>
#include <atomic>
#include <concepts>
#include <thread>
#include <type_traits>
#include <vector>

namespace {

struct CrossThreadState {
  explicit CrossThreadState(int initial_value) : value(initial_value) {}
  int value = 0;
};

using Access = cgpui::CrossThreadEntity<CrossThreadState>;

static_assert(std::is_copy_constructible_v<Access>);
static_assert(requires(
    const cgpui::AsyncContextCapability& async,
    cgpui::EntityHandle<CrossThreadState> entity) {
  { async.entity(entity) } -> std::same_as<Access>;
});

class CrossThreadEntityView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  std::thread::id runtime_thread;
  std::vector<int> values;
  std::vector<cgpui::CrossThreadEntityAccessStatus> statuses;
  bool callbacks_on_runtime_thread = true;
  bool worker_submissions_accepted = false;
  bool foreign_rejected = false;
};

RuntimeFixture* access_fixture = nullptr;
cgpui::WindowRuntime* access_runtime = nullptr;
CrossThreadEntityView* access_view = nullptr;
Access detached_access;

void dispatch_cross_thread_entity_sequence() {
  auto& fixture = *access_fixture;
  auto& runtime = *access_runtime;
  auto& view = *access_view;
  const cgpui::TaskHandle setup = runtime.spawn_task(
      [&](const cgpui::WindowRuntimeContext& context) {
        view.runtime_thread = std::this_thread::get_id();
        const cgpui::EntityHandle<CrossThreadState> entity =
            context.new_entity<CrossThreadState>(5);
        detached_access = context.async_context().entity(entity);
        const cgpui::EntityHandle<CrossThreadState> foreign(
            entity.id(), entity.context_token() + 1);
        const Access foreign_access = context.async_context().entity(foreign);
        view.foreign_rejected = foreign_access.empty() &&
            !foreign_access.read([](const CrossThreadState&,
                                    const cgpui::WindowRuntimeContext&) {});

        const auto complete = [&](cgpui::CrossThreadEntityAccessStatus status,
                                  const cgpui::WindowRuntimeContext&) {
          view.callbacks_on_runtime_thread =
              view.callbacks_on_runtime_thread &&
              std::this_thread::get_id() == view.runtime_thread;
          view.statuses.push_back(status);
        };
        bool read_initial = false;
        bool update = false;
        bool read_updated = false;
        std::thread worker([&] {
          read_initial = detached_access.read(
              [&](const CrossThreadState& state,
                  const cgpui::WindowRuntimeContext&) {
                view.callbacks_on_runtime_thread =
                    view.callbacks_on_runtime_thread &&
                    std::this_thread::get_id() == view.runtime_thread;
                view.values.push_back(state.value);
              },
              complete);
          update = detached_access.update(
              [&](CrossThreadState& state,
                  const cgpui::WindowRuntimeContext&) {
                view.callbacks_on_runtime_thread =
                    view.callbacks_on_runtime_thread &&
                    std::this_thread::get_id() == view.runtime_thread;
                state.value = 9;
              },
              complete);
        });
        worker.join();

        std::array<std::atomic_bool, 16> burst_accepted{};
        std::vector<std::thread> burst_workers;
        for (std::size_t index = 0; index < burst_accepted.size(); ++index) {
          burst_workers.emplace_back([&, index] {
            burst_accepted[index].store(detached_access.update(
                [](CrossThreadState& state,
                   const cgpui::WindowRuntimeContext&) {
                  state.value += 1;
                },
                complete));
          });
        }
        for (auto& burst_worker : burst_workers) burst_worker.join();

        std::thread final_reader([&] {
          read_updated = detached_access.read(
              [&](const CrossThreadState& state,
                  const cgpui::WindowRuntimeContext&) {
                view.values.push_back(state.value);
              },
              [&, entity, complete](
                  cgpui::CrossThreadEntityAccessStatus status,
                  const cgpui::WindowRuntimeContext& completion_context) {
                complete(status, completion_context);
                (void)completion_context.remove_entity(entity);
                std::thread missing_worker([&] {
                  (void)detached_access.read(
                      [](const CrossThreadState&,
                         const cgpui::WindowRuntimeContext&) {},
                      complete);
                });
                missing_worker.join();
              });
        });
        final_reader.join();
        const bool burst_all_accepted = std::ranges::all_of(
            burst_accepted,
            [](const std::atomic_bool& accepted) {
              return accepted.load();
            });
        view.worker_submissions_accepted =
            read_initial && update && burst_all_accepted && read_updated;
      });
  (void)runtime.complete_task(setup.id());
  fixture.app.dispatch_wakeup();
}

} // namespace

int main() {
  RuntimeFixture fixture;
  CrossThreadEntityView view;
  {
    fixture.app.on_run = &dispatch_cross_thread_entity_sequence;
    cgpui::WindowRuntime runtime(
        fixture.app,
        view,
        [&](const cgpui::RenderSurfaceDescriptor&) {
          return cgpui::Result<cgpui::Renderer*>{&fixture.renderer};
        });
    access_fixture = &fixture;
    access_runtime = &runtime;
    access_view = &view;
    const int result = runtime.run(
        cgpui::WindowDescriptor{},
        cgpui::WindowRuntimeOptions{.request_initial_redraw = false});
    access_fixture = nullptr;
    access_runtime = nullptr;
    access_view = nullptr;
    if (result != 0) return 1;
    if (!view.worker_submissions_accepted || !view.foreign_rejected) return 2;
    if (!view.callbacks_on_runtime_thread) return 3;
    if (view.values != std::vector<int>({5, 25})) return 4;
    if (view.statuses.size() != 20 ||
        view.statuses.front() != cgpui::CrossThreadEntityAccessStatus::read ||
        !std::ranges::all_of(
            view.statuses.begin() + 1,
            view.statuses.begin() + 18,
            [](cgpui::CrossThreadEntityAccessStatus status) {
              return status ==
                  cgpui::CrossThreadEntityAccessStatus::updated;
            }) ||
        view.statuses[18] != cgpui::CrossThreadEntityAccessStatus::read ||
        view.statuses[19] != cgpui::CrossThreadEntityAccessStatus::missing) {
      return 5;
    }
  }
  return !detached_access.read(
      [](const CrossThreadState&, const cgpui::WindowRuntimeContext&) {})
      ? 0
      : 6;
}
