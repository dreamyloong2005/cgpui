#include "ui_internal.hpp"

namespace cgpui {

WindowRuntime::StaticRenderInstallResult WindowRuntime::install_static_render_tree(
    View& view,
    ViewContext& render_context) {
  if (!view.supports_static_render()) {
    return {};
  }

  const StaticElementTreeView rendered = view.render_static(render_context);
  if (rendered.empty() || !rendered.valid()) {
    return {};
  }

  set_static_element_tree(rendered);
  return StaticRenderInstallResult{
      .installed = true,
      .root_id = rendered.root_id(),
      .node_count = rendered.size(),
  };
}

} // namespace cgpui
