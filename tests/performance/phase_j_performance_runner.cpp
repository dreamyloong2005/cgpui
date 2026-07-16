#include "cgpui/prelude.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/test_app.hpp"

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;

struct Metric {
  std::string name;
  std::size_t operation_count = 0;
  std::uint64_t total_ns = 0;

  [[nodiscard]] std::uint64_t mean_ns() const {
    return operation_count == 0 ? 0 : total_ns / operation_count;
  }
};

volatile std::uint64_t metric_sink = 0;

template <typename Operation>
Metric measure(std::string name, std::size_t operation_count,
               Operation&& operation) {
  const auto start = Clock::now();
  operation();
  const auto end = Clock::now();
  return Metric{
      .name = std::move(name),
      .operation_count = operation_count,
      .total_ns = static_cast<std::uint64_t>(
          std::chrono::duration_cast<std::chrono::nanoseconds>(end - start)
              .count()),
  };
}

class BenchmarkView final : public cgpui::View {
 public:
  cgpui::AnyElement render(cgpui::ViewContext& context) override {
    return cgpui::into_element(
        cgpui::div().size(context.viewport_size).background(cgpui::rgb(20, 30, 40)));
  }

  void paint(cgpui::PaintList& paint, cgpui::Size) override {
    paint.fill_rect({.origin = {1.0F, 1.0F}, .size = {8.0F, 8.0F}},
                    cgpui::rgb(40, 120, 200));
  }

  cgpui::EventResult handle_event(
      const cgpui::PlatformEvent& event,
      const cgpui::WindowRuntimeContext& context) override {
    const auto* key = std::get_if<cgpui::KeyboardKey>(&event);
    if (key == nullptr) return cgpui::EventResult::unhandled();
    if (key->key_code == 'A') {
      tasks.clear();
      completed_tasks = 0;
      for (std::size_t index = 0; index < task_batch_size; ++index) {
        tasks.push_back(context.spawn_task(
            [this](const cgpui::WindowRuntimeContext&) { ++completed_tasks; }));
      }
      return cgpui::EventResult::consumed_event();
    }
    if (key->key_code == 'P') {
      animation_ticks = 0;
      animation = context.start_animation(
          {.duration_ms = 160, .tick_interval_ms = 16},
          [this](const cgpui::WindowRuntimeContext&,
                 const cgpui::AnimationSnapshot&) { ++animation_ticks; });
      return cgpui::EventResult::consumed_event();
    }
    return cgpui::EventResult::unhandled();
  }

  static constexpr std::size_t task_batch_size = 64;
  std::vector<cgpui::TaskHandle> tasks;
  cgpui::AnimationHandle animation;
  std::size_t completed_tasks = 0;
  std::size_t animation_ticks = 0;
};

std::vector<std::uint8_t> decode_base64(std::string_view encoded) {
  const auto value = [](char character) {
    if (character >= 'A' && character <= 'Z') return character - 'A';
    if (character >= 'a' && character <= 'z') return character - 'a' + 26;
    if (character >= '0' && character <= '9') return character - '0' + 52;
    if (character == '+') return 62;
    if (character == '/') return 63;
    return -1;
  };
  std::vector<std::uint8_t> bytes;
  int accumulator = 0;
  int bits = -8;
  for (const char character : encoded) {
    if (character == '=') break;
    const int next = value(character);
    if (next < 0) continue;
    accumulator = (accumulator << 6) | next;
    bits += 6;
    if (bits >= 0) {
      bytes.push_back(static_cast<std::uint8_t>((accumulator >> bits) & 0xff));
      bits -= 8;
    }
  }
  return bytes;
}

std::vector<std::uint8_t> png_bytes() {
  return decode_base64(
      "iVBORw0KGgoAAAANSUhEUgAAAAIAAAABCAYAAAD0In+KAAAAAXNSR0IArs4c6QAA"
      "AARnQU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAOSURBVBhXY/jP"
      "wPAfBAEQ+AP9OGZFWQAAAABJRU5ErkJggg==");
}

std::vector<Metric> run_baselines(std::size_t iterations) {
  std::vector<Metric> metrics;
  metrics.push_back(measure("startup", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      cgpui::TestApp app;
      metric_sink += app.window_count();
    }
  }));
  metrics.push_back(measure("first_frame", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      cgpui::TestApp app;
      auto window = app.open_window(cgpui::WindowOptions{},
                                    std::make_unique<BenchmarkView>());
      window.draw_frame();
      metric_sink += window.render_snapshot().present_count;
    }
  }));

  cgpui::TestApp app;
  auto window = app.open_window(cgpui::WindowOptions{},
                                std::make_unique<BenchmarkView>());
  auto* view = window.root_view_as<BenchmarkView>();
  metrics.push_back(measure("resize", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const float edge = index % 2 == 0 ? 640.0F : 800.0F;
      window.simulate_resize({edge, edge * 0.75F}, cgpui::DpiScale{1.0F});
    }
    metric_sink += window.render_snapshot().resize_count;
  }));

  constexpr std::string_view paragraph =
      "CGPUI public text layout baseline: Latin, CJK, emoji, wrapping, and bidi metadata.";
  metrics.push_back(measure("text_layout", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto measured = cgpui::measure_text(paragraph, {}, 16.0F,
                                                cgpui::DpiScale{1.0F});
      metric_sink += measured.shape_run.glyphs.size();
    }
  }));

  std::vector<cgpui::UniformListItemIdentity> items;
  items.reserve(10000);
  for (std::size_t index = 0; index < 10000; ++index) {
    items.push_back({.index = index,
                     .key = {.value = std::to_string(index)},
                     .element_id = cgpui::ElementId{index + 1},
                     .content_bounds = {.origin = {0.0F, index * 20.0F},
                                        .size = {400.0F, 20.0F}}});
  }
  metrics.push_back(measure("list_scrolling", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto visible = cgpui::calculate_uniform_list_visible_range(
          items, {0.0F, static_cast<float>((index * 137) % 190000)},
          {400.0F, 600.0F});
      metric_sink += visible.end_index - visible.start_index;
    }
  }));

  const auto encoded_image = png_bytes();
  metrics.push_back(measure("image_loading", iterations, [&] {
    for (std::size_t index = 0; index < iterations; ++index) {
      const auto decoded = cgpui::decode_image(encoded_image);
      metric_sink += decoded.bitmap.pixels.size();
    }
  }));

  metrics.push_back(measure("async_wakeup", BenchmarkView::task_batch_size, [&] {
    window.dispatch_keystroke({.key_code = 'A'});
    for (const auto& task : view->tasks) (void)app.complete_task(task.id());
    app.drain_task_completions();
    metric_sink += view->completed_tasks;
  }));
  metrics.push_back(measure("frame_pacing", 10, [&] {
    window.dispatch_keystroke({.key_code = 'P'});
    for (int frame = 0; frame < 10; ++frame) app.advance_time_until_parked(16);
    metric_sink += view->animation_ticks;
  }));
  return metrics;
}

int write_report(std::ostream& output, std::string_view platform,
                 std::size_t iterations, std::span<const Metric> metrics) {
  output << "{\n  \"schema_version\": 1,\n  \"platform\": \"" << platform
         << "\",\n  \"iterations\": " << iterations << ",\n  \"metrics\": [\n";
  for (std::size_t index = 0; index < metrics.size(); ++index) {
    const auto& metric = metrics[index];
    output << "    {\"name\": \"" << metric.name
           << "\", \"unit\": \"ns_per_operation\", \"operation_count\": "
           << metric.operation_count << ", \"total_ns\": " << metric.total_ns
           << ", \"mean_ns\": " << metric.mean_ns() << "}"
           << (index + 1 == metrics.size() ? "\n" : ",\n");
  }
  output << "  ]\n}\n";
  return output ? 0 : 5;
}

}  // namespace

int main(int argc, char** argv) {
  std::string platform = "portable";
  std::string output_path;
  std::size_t iterations = 16;
  for (int index = 1; index < argc; ++index) {
    const std::string_view argument = argv[index];
    if (argument == "--platform" && index + 1 < argc) platform = argv[++index];
    else if (argument == "--output" && index + 1 < argc) output_path = argv[++index];
    else if (argument == "--iterations" && index + 1 < argc)
      iterations = std::stoul(argv[++index]);
    else return 2;
  }
  if (iterations == 0 || iterations > 10000 ||
      (platform != "portable" && platform != "windows" &&
       platform != "wayland" && platform != "x11")) return 3;
  const auto metrics = run_baselines(iterations);
  if (metrics.size() != 8 || metric_sink == 0) return 4;
  if (output_path.empty()) return write_report(std::cout, platform, iterations, metrics);
  const std::filesystem::path output_file{output_path};
  if (output_file.has_parent_path()) {
    std::filesystem::create_directories(output_file.parent_path());
  }
  std::ofstream output(output_path, std::ios::binary);
  return write_report(output, platform, iterations, metrics);
}
