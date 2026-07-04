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
  for (ActionRegistration& existing : action_registrations_) {
    if (same_action_registration_key(existing, registration)) {
      existing = std::move(registration);
      return;
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

} // namespace cgpui
