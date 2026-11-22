#include "ui_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

bool WindowRuntime::retire_runtime_user_resources() {
  // Every source container is coherent before any user destructor can reenter.
  auto globals = std::exchange(state_->entities.globals_, {});
  auto stores = std::exchange(state_->entities.entity_stores_, {});
  auto entity_observers = std::exchange(state_->entities.entity_observers_, {});
  auto window_observers = std::exchange(state_->entities.window_observers_, {});
  auto view_observers = std::exchange(state_->entities.view_observers_, {});
  auto actions = std::exchange(state_->actions.action_handlers_, {});
  auto window_actions = std::exchange(state_->actions.window_action_handlers_, {});
  auto view_actions = std::exchange(state_->actions.view_action_handlers_, {});
  auto element_actions = std::exchange(state_->actions.focused_element_action_handlers_, {});
  auto animations = std::exchange(state_->animations.animations_, {});
  auto timers = std::exchange(state_->scheduling.timers_, {});
  auto deferred = std::exchange(state_->scheduling.deferred_callbacks_, {});
  auto frame_callback = std::exchange(state_->core.after_frame_callback_, {});
  auto render_callback = std::exchange(state_->core.after_render_callback_, {});
  auto event_callback = std::exchange(state_->core.after_event_callback_, {});
  auto close_callback = std::exchange(state_->core.close_requested_callback_, {});
  auto error_callback = std::exchange(state_->core.error_callback_, {});
  auto appearance_callback = std::exchange(state_->platform.system_appearance_callback_, {});
  auto factory = std::exchange(state_->core.renderer_factory_, {});
  auto owned_factory = std::exchange(state_->windows.window_lifetime_->factory, {});
  auto views = std::exchange(state_->views.view_registry_, {});
  auto hosts = std::exchange(state_->elements.window_element_hosts_, {});
  auto tree = std::move(state_->elements.owned_element_tree_);
  state_->elements.element_root_ = nullptr;
  const bool had_resources = !globals.empty() || !stores.empty() || !entity_observers.empty() ||
      !window_observers.empty() || !view_observers.empty() || !actions.empty() ||
      !window_actions.empty() || !view_actions.empty() || !element_actions.empty() ||
      !animations.empty() || !timers.empty() || !deferred.empty() || frame_callback ||
      render_callback || event_callback || close_callback || error_callback || appearance_callback ||
      factory || owned_factory || !views.empty() || !hosts.empty() || tree;
  tree.reset();
  hosts.clear();
  state_->windows.window_lifetime_->release_retired_views();
  views.clear();
  return had_resources;
}

} // namespace cgpui
