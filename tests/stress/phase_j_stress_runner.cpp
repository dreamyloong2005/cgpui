#include "cgpui/prelude.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_app.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

struct ScenarioResult {
  std::string name;
  std::size_t operation_count = 0;
  bool complete = false;
};

struct StressEntity {
  std::size_t value = 0;
};

class StressView final : public cgpui::View {
 public:
  void paint(cgpui::PaintList&, cgpui::Size) override {}

  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    return cgpui::into_element(cgpui::text(text_model).size(context.viewport_size));
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    if (const auto* key = std::get_if<cgpui::KeyboardKey>(&event)) {
      if (key->key_code == 'W') {
        wakeup_task = context.spawn_task(
            [](const cgpui::WindowRuntimeContext&) {});
        return cgpui::EventResult::consumed_event();
      }
      if (key->key_code == 'E') {
        entities.clear();
        for (std::size_t index = 0; index < entity_target; ++index) {
          entities.push_back(context.new_entity<StressEntity>(StressEntity{index}));
        }
        for (const auto entity : entities) {
          if (context.remove_entity(entity)) ++removed_entities;
        }
        return cgpui::EventResult::consumed_event();
      }
      if (key->key_code == 'T') {
        tasks.clear();
        for (std::size_t index = 0; index < task_target; ++index) {
          tasks.push_back(context.spawn_task(
              [this](const cgpui::WindowRuntimeContext&) { ++executed_tasks; }));
        }
        return cgpui::EventResult::consumed_event();
      }
    }
    if (const auto* ime = std::get_if<cgpui::ImeComposition>(&event)) {
      if (ime->phase == cgpui::ImeCompositionPhase::update)
        text_model.set_composition_text(ime->text);
      else if (ime->phase == cgpui::ImeCompositionPhase::commit)
        text_model.commit_composition();
      else
        text_model.cancel_composition();
      ++ime_events;
      return cgpui::EventResult::consumed_event();
    }
    if (std::holds_alternative<cgpui::DragEntered>(event)) {
      ++drag_enters;
      return cgpui::EventResult::consumed_event();
    }
    if (std::holds_alternative<cgpui::DragExited>(event)) {
      ++drag_exits;
      return cgpui::EventResult::consumed_event();
    }
    return cgpui::EventResult::unhandled();
  }

  std::size_t entity_target = 0;
  std::size_t task_target = 0;
  std::size_t removed_entities = 0;
  std::size_t executed_tasks = 0;
  std::size_t ime_events = 0;
  std::size_t drag_enters = 0;
  std::size_t drag_exits = 0;
  cgpui::TextModel text_model{"seed"};
  std::vector<cgpui::EntityHandle<StressEntity>> entities;
  std::vector<cgpui::TaskHandle> tasks;
  cgpui::TaskHandle wakeup_task;
};

ScenarioResult run_window_churn(std::size_t count) {
  cgpui::TestApp app;
  auto control = app.open_window(cgpui::WindowOptions{},
                                 std::make_unique<StressView>());
  auto* control_view = control.root_view_as<StressView>();
  bool complete = true;
  for (std::size_t index = 0; index < count; ++index) {
    auto window = app.open_window(cgpui::WindowOptions{},
                                  std::make_unique<StressView>());
    complete = complete && window.dispatch_platform_event(
                               cgpui::PlatformEvent{cgpui::WindowCloseRequested{}});
  }
  control.dispatch_keystroke({.key_code = 'W'});
  complete = complete && app.complete_task(control_view->wakeup_task.id());
  app.run_until_parked();
  return {"window_churn", count, complete && app.window_count() == 1};
}

ScenarioResult run_many_entities(cgpui::TestAppWindow& window,
                                 StressView& view, std::size_t count) {
  view.entity_target = count;
  window.dispatch_keystroke({.key_code = 'E'});
  return {"many_entities", count,
          view.entities.size() == count && view.removed_entities == count};
}

ScenarioResult run_large_uniform_list(std::size_t item_count,
                                      std::size_t probe_count) {
  std::vector<cgpui::UniformListItemIdentity> items;
  items.reserve(item_count);
  for (std::size_t index = 0; index < item_count; ++index) {
    items.push_back({.index = index,
                     .key = {.value = std::to_string(index)},
                     .element_id = cgpui::ElementId{index + 1},
                     .content_bounds = {.origin = {0.0F, index * 18.0F},
                                        .size = {600.0F, 18.0F}}});
  }
  bool complete = true;
  for (std::size_t index = 0; index < probe_count; ++index) {
    const auto range = cgpui::calculate_uniform_list_visible_range(
        items, {0.0F, static_cast<float>((index * 997) % (item_count * 17))},
        {600.0F, 720.0F});
    complete = complete && !range.empty() && range.end_index <= item_count;
  }
  return {"large_uniform_list", item_count, complete};
}

ScenarioResult run_ime_composition(cgpui::TestAppWindow& window,
                                   StressView& view, std::size_t cycles) {
  for (std::size_t index = 0; index < cycles; ++index) {
    (void)window.dispatch_platform_event(cgpui::ImeComposition{
        .phase = cgpui::ImeCompositionPhase::update, .text = "preedit"});
    (void)window.dispatch_platform_event(cgpui::ImeComposition{
        .phase = index % 2 == 0 ? cgpui::ImeCompositionPhase::commit
                               : cgpui::ImeCompositionPhase::cancel});
  }
  return {"ime_composition", cycles,
          view.ime_events == cycles * 2 && !view.text_model.has_composition()};
}

ScenarioResult run_clipboard_ownership(cgpui::TestApp& app,
                                       std::size_t count) {
  bool complete = true;
  for (std::size_t index = 0; index < count; ++index) {
    const std::string value = "owner-" + std::to_string(index);
    complete = complete && app.write_to_clipboard(value) &&
        app.read_from_clipboard() == value;
  }
  return {"clipboard_ownership", count, complete};
}

ScenarioResult run_drag_drop_cancel(cgpui::TestAppWindow& window,
                                    StressView& view, std::size_t count) {
  const cgpui::DragDropPayload payload{
      .kind = cgpui::DragDropPayloadKind::text, .text = "payload"};
  bool complete = true;
  for (std::size_t index = 0; index < count; ++index) {
    complete = complete && window.dispatch_platform_event(cgpui::DragEntered{
                               .position = {4.0F, 4.0F},
                               .payload = payload,
                               .action = cgpui::DragDropAction::copy});
    complete = complete && window.dispatch_platform_event(cgpui::DragExited{
                               .position = {5.0F, 5.0F},
                               .payload = payload,
                               .action = cgpui::DragDropAction::none});
  }
  return {"drag_drop_cancel", count,
          complete && view.drag_enters == count && view.drag_exits == count};
}

ScenarioResult run_asset_reload(std::size_t count) {
  cgpui::AssetReloadState reloads;
  constexpr std::size_t thread_count = 4;
  std::vector<std::thread> workers;
  for (std::size_t thread = 0; thread < thread_count; ++thread) {
    workers.emplace_back([&] {
      for (std::size_t index = 0; index < count / thread_count; ++index)
        (void)reloads.invalidate(cgpui::AssetSourceId{7}, "stress.png");
    });
  }
  for (auto& worker : workers) worker.join();
  const auto key = reloads.key(cgpui::AssetSourceId{7}, "stress.png",
                               cgpui::AssetCacheKind::raster_image);
  return {"asset_reload", count,
          key && key->revision == count &&
              reloads.snapshot().invalidation_count == count};
}

ScenarioResult run_task_cancellation(cgpui::TestApp& app,
                                     cgpui::TestAppWindow& window,
                                     StressView& view, std::size_t count) {
  view.task_target = count;
  window.dispatch_keystroke({.key_code = 'T'});
  std::size_t cancelled = 0;
  for (auto& task : view.tasks) cancelled += task.cancel() ? 1U : 0U;
  app.drain_task_completions();
  return {"task_cancellation", count,
          cancelled == count && view.executed_tasks == 0};
}

std::vector<ScenarioResult> run_scenarios(std::size_t scale) {
  std::vector<ScenarioResult> results;
  results.push_back(run_window_churn(64 * scale));
  cgpui::TestApp app;
  auto window = app.open_window(cgpui::WindowOptions{},
                                std::make_unique<StressView>());
  auto* view = window.root_view_as<StressView>();
  results.push_back(run_many_entities(window, *view, 2000 * scale));
  results.push_back(run_large_uniform_list(50000 * scale, 500 * scale));
  results.push_back(run_ime_composition(window, *view, 1000 * scale));
  results.push_back(run_clipboard_ownership(app, 1000 * scale));
  results.push_back(run_drag_drop_cancel(window, *view, 1000 * scale));
  results.push_back(run_asset_reload(2000 * scale));
  results.push_back(run_task_cancellation(app, window, *view, 2000 * scale));
  return results;
}

int write_report(std::ostream& output, std::string_view platform,
                 std::size_t scale, std::span<const ScenarioResult> results) {
  output << "{\n  \"schema_version\": 1,\n  \"platform\": \"" << platform
         << "\",\n  \"scale\": " << scale << ",\n  \"scenarios\": [\n";
  for (std::size_t index = 0; index < results.size(); ++index) {
    const auto& result = results[index];
    output << "    {\"name\": \"" << result.name
           << "\", \"operation_count\": " << result.operation_count
           << ", \"status\": \"" << (result.complete ? "complete" : "failed")
           << "\"}" << (index + 1 == results.size() ? "\n" : ",\n");
  }
  output << "  ]\n}\n";
  return output ? 0 : 5;
}

}  // namespace

int main(int argc, char** argv) {
  std::string platform = "portable";
  std::string output_path;
  std::size_t scale = 1;
  for (int index = 1; index < argc; ++index) {
    const std::string_view argument = argv[index];
    if (argument == "--platform" && index + 1 < argc) platform = argv[++index];
    else if (argument == "--output" && index + 1 < argc) output_path = argv[++index];
    else if (argument == "--scale" && index + 1 < argc) scale = std::stoul(argv[++index]);
    else return 2;
  }
  const bool valid_platform =
      platform == "portable" || platform == "windows" ||
      platform == "wayland" || platform == "x11" ||
      platform == "macos-arm64" || platform == "macos-x86_64";
  if (scale == 0 || scale > 100 || !valid_platform) return 3;
  const auto results = run_scenarios(scale);
  bool complete = true;
  for (const auto& result : results) {
    if (!result.complete) {
      std::cerr << result.name << " failed\n";
      complete = false;
    }
  }
  if (output_path.empty()) {
    const int report_result = write_report(std::cout, platform, scale, results);
    return report_result == 0 && !complete ? 4 : report_result;
  }
  const std::filesystem::path output_file{output_path};
  if (output_file.has_parent_path())
    std::filesystem::create_directories(output_file.parent_path());
  std::ofstream output(output_file, std::ios::binary);
  const int report_result = write_report(output, platform, scale, results);
  return report_result == 0 && !complete ? 4 : report_result;
}
