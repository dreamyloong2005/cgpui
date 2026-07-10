#pragma once

#include <wayland-client.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

namespace cgpui {

class WaylandOutputScaleRegistry {
 public:
  using ChangeCallback =
      std::function<void(wl_output*, std::int32_t, bool)>;

  WaylandOutputScaleRegistry();
  ~WaylandOutputScaleRegistry();

  void set_change_callback(ChangeCallback callback);
  void bind(
      wl_registry* registry,
      std::uint32_t name,
      std::uint32_t version);
  void remove(std::uint32_t name);
  void reset();
  [[nodiscard]] std::int32_t scale_for(wl_output* output) const;

 private:
  struct Record;

  static void handle_geometry(
      void* data,
      wl_output* output,
      std::int32_t x,
      std::int32_t y,
      std::int32_t physical_width,
      std::int32_t physical_height,
      std::int32_t subpixel,
      const char* make,
      const char* model,
      std::int32_t transform);
  static void handle_mode(
      void* data,
      wl_output* output,
      std::uint32_t flags,
      std::int32_t width,
      std::int32_t height,
      std::int32_t refresh);
  static void handle_done(void* data, wl_output* output);
  static void handle_scale(
      void* data,
      wl_output* output,
      std::int32_t factor);

  ChangeCallback callback_;
  std::vector<std::unique_ptr<Record>> records_;
};

} // namespace cgpui
