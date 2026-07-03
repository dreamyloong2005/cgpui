#pragma once

#include <any>
#include <typeindex>
#include <typeinfo>
#include <utility>

namespace cgpui {

template <typename T>
T* ElementTree::state(ElementId id) {
  Node* node = find_node(id);
  if (node == nullptr) {
    return nullptr;
  }
  const auto entry = node->states.find(std::type_index(typeid(T)));
  if (entry == node->states.end()) {
    return nullptr;
  }
  return std::any_cast<T>(&entry->second);
}

template <typename T>
const T* ElementTree::state(ElementId id) const {
  const Node* node = find_node(id);
  if (node == nullptr) {
    return nullptr;
  }
  const auto entry = node->states.find(std::type_index(typeid(T)));
  if (entry == node->states.end()) {
    return nullptr;
  }
  return std::any_cast<T>(&entry->second);
}

template <typename T, typename... Args>
T* ElementTree::emplace_state(ElementId id, Args&&... args) {
  Node* node = find_node(id);
  if (node == nullptr) {
    return nullptr;
  }
  std::any& stored = node->states[std::type_index(typeid(T))];
  stored.emplace<T>(std::forward<Args>(args)...);
  return std::any_cast<T>(&stored);
}

template <typename T, typename... Args>
T* ElementTree::state_or_init(ElementId id, Args&&... args) {
  if (T* existing = state<T>(id); existing != nullptr) {
    return existing;
  }
  return emplace_state<T>(id, std::forward<Args>(args)...);
}

template <typename T>
T* ElementTree::find_as(ElementId id) {
  return dynamic_cast<T*>(get(id));
}

template <typename T>
const T* ElementTree::find_as(ElementId id) const {
  return dynamic_cast<const T*>(get(id));
}

} // namespace cgpui
