#pragma once

#include "cgpui/ui/element_core.hpp"

#include <concepts>

namespace cgpui {

struct WindowRuntimeContext;
using ViewContext = WindowRuntimeContext;

template <typename T>
using Context = ViewContext;

using IntoElement = AnyElement;

template <typename T>
concept Render = requires(T& view, Context<T>& context) {
  { view.render(context) } -> std::same_as<IntoElement>;
};

} // namespace cgpui
