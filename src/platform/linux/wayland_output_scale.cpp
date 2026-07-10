#include "wayland_output_scale.hpp"

#include <algorithm>
#include <utility>

namespace cgpui {

struct WaylandOutputScaleRegistry::Record {
  WaylandOutputScaleRegistry* owner = nullptr;
  std::uint32_t name = 0;
  wl_output* output = nullptr;
  std::int32_t scale = 1;
};

WaylandOutputScaleRegistry::WaylandOutputScaleRegistry() = default;

WaylandOutputScaleRegistry::~WaylandOutputScaleRegistry() {
  reset();
}

void WaylandOutputScaleRegistry::set_change_callback(
    ChangeCallback callback) {
  callback_ = std::move(callback);
}

void WaylandOutputScaleRegistry::bind(
    wl_registry* registry,
    std::uint32_t name,
    std::uint32_t version) {
  auto record = std::make_unique<Record>();
  record->owner = this;
  record->name = name;
  record->output = static_cast<wl_output*>(wl_registry_bind(
      registry,
      name,
      &wl_output_interface,
      std::min<std::uint32_t>(version, 2)));
  if (record->output == nullptr) {
    return;
  }
  static const wl_output_listener listener{
      .geometry = &WaylandOutputScaleRegistry::handle_geometry,
      .mode = &WaylandOutputScaleRegistry::handle_mode,
      .done = &WaylandOutputScaleRegistry::handle_done,
      .scale = &WaylandOutputScaleRegistry::handle_scale,
      .name = nullptr,
      .description = nullptr,
  };
  wl_output_add_listener(record->output, &listener, record.get());
  records_.push_back(std::move(record));
}

void WaylandOutputScaleRegistry::remove(std::uint32_t name) {
  const auto found = std::ranges::find(
      records_,
      name,
      [](const std::unique_ptr<Record>& record) { return record->name; });
  if (found == records_.end()) {
    return;
  }
  (*found)->scale = 1;
  if (callback_) {
    callback_((*found)->output, 1, false);
  }
  wl_output_destroy((*found)->output);
  records_.erase(found);
}

void WaylandOutputScaleRegistry::reset() {
  for (const auto& record : records_) {
    wl_output_destroy(record->output);
  }
  records_.clear();
}

std::int32_t WaylandOutputScaleRegistry::scale_for(wl_output* output) const {
  const auto found = std::ranges::find(
      records_,
      output,
      [](const std::unique_ptr<Record>& record) { return record->output; });
  return found == records_.end() ? 1 : (*found)->scale;
}

void WaylandOutputScaleRegistry::handle_geometry(
    void*,
    wl_output*,
    std::int32_t,
    std::int32_t,
    std::int32_t,
    std::int32_t,
    std::int32_t,
    const char*,
    const char*,
    std::int32_t) {}

void WaylandOutputScaleRegistry::handle_mode(
    void*,
    wl_output*,
    std::uint32_t,
    std::int32_t,
    std::int32_t,
    std::int32_t) {}

void WaylandOutputScaleRegistry::handle_done(void*, wl_output*) {}

void WaylandOutputScaleRegistry::handle_scale(
    void* data,
    wl_output* output,
    std::int32_t factor) {
  auto* record = static_cast<Record*>(data);
  const std::int32_t scale = std::max(factor, 1);
  if (record->scale == scale) {
    return;
  }
  record->scale = scale;
  if (record->owner->callback_) {
    record->owner->callback_(output, scale, true);
  }
}

} // namespace cgpui
