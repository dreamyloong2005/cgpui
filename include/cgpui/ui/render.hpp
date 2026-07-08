#pragma once

#include "cgpui/ui/element_core.hpp"
#include "cgpui/ui/static_element_tree.hpp"

#include <concepts>

namespace cgpui {

struct WindowRuntimeContext;
using ViewContext = WindowRuntimeContext;

template <typename T>
using Context = ViewContext;

using IntoElement = AnyElement;
using StaticIntoElement = StaticElementTreeView;

template <typename T>
concept Render = requires(T& view, Context<T>& context) {
  { view.render(context) } -> std::same_as<IntoElement>;
};

template <typename T>
concept StaticRender = requires(T& view, Context<T>& context) {
  { view.render_static(context) } -> std::same_as<StaticIntoElement>;
};

} // namespace cgpui
