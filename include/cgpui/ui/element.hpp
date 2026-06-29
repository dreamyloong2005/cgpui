#pragma once

#include <cstdint>

namespace cgpui {

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
};

class Element {
 public:
  virtual ~Element() = default;

  [[nodiscard]] ElementId id() const {
    return id_;
  }

  void assign_id(ElementId id) {
    id_ = id;
  }

 private:
  ElementId id_;
};

} // namespace cgpui
