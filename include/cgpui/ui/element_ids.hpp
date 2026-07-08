#pragma once

#include <cstdint>
#include <string>

namespace cgpui {

struct ElementId {
  std::uint64_t value = 0;

  friend bool operator==(ElementId, ElementId) = default;
};

struct ViewId {
  std::uint64_t value = 0;

  friend bool operator==(ViewId, ViewId) = default;
};

struct ElementKey {
  std::string value;

  friend bool operator==(const ElementKey&, const ElementKey&) = default;
};

} // namespace cgpui
