#include "ui_internal.hpp"

namespace cgpui {

namespace {

bool same_action_registration_key(
    const ActionRegistration& lhs,
    const ActionRegistration& rhs) {
  return lhs.name == rhs.name &&
         lhs.registration_scope == rhs.registration_scope &&
         lhs.view_id == rhs.view_id &&
         lhs.element_id == rhs.element_id;
}

} // namespace

void WindowRuntime::upsert_action_registration(
    ActionRegistration registration) {
  for (auto existing = action_registrations_.begin();
       existing != action_registrations_.end();
       ++existing) {
    if (same_action_registration_key(*existing, registration)) {
      action_registrations_.erase(existing);
      break;
    }
  }
  action_registrations_.push_back(std::move(registration));
}

std::span<const ActionRegistration> WindowRuntime::action_registrations()
    const {
  return action_registrations_;
}

std::vector<ActionRegistration> WindowRuntime::action_registrations_for_scope(
    ActionRegistrationScope scope) const {
  std::vector<ActionRegistration> registrations;
  for (const ActionRegistration& registration : action_registrations_) {
    if (registration.registration_scope == scope) {
      registrations.push_back(registration);
    }
  }
  return registrations;
}

std::vector<ActionRegistration> WindowRuntime::action_registrations_for_enabled(
    bool enabled) const {
  std::vector<ActionRegistration> registrations;
  for (const ActionRegistration& registration : action_registrations_) {
    if (registration.enabled == enabled) {
      registrations.push_back(registration);
    }
  }
  return registrations;
}

std::optional<bool> WindowRuntime::action_registration_enabled(
    std::string_view name,
    ActionScope dispatch_scope,
    std::optional<ViewId> view_id,
    std::optional<ElementId> element_id) const {
  for (auto registration = action_registrations_.rbegin();
       registration != action_registrations_.rend();
       ++registration) {
    if (registration->name == name &&
        registration->dispatch_scope == dispatch_scope &&
        registration->view_id == view_id &&
        registration->element_id == element_id) {
      return registration->enabled;
    }
  }
  return std::nullopt;
}

} // namespace cgpui
