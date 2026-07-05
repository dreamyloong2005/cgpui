#include "cgpui/ui/test_context.hpp"

#include "cgpui/platform/clipboard.hpp"
#include "cgpui/ui/runtime_context.hpp"
#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

void TestContextCapability::set_clipboard(Clipboard* clipboard) const {
  context_->runtime.set_clipboard(clipboard);
}

bool TestContextCapability::write_to_clipboard(std::string_view text) const {
  return context_->runtime.write_clipboard_text(text);
}

std::optional<std::string> TestContextCapability::read_from_clipboard() const {
  return context_->runtime.read_clipboard_text();
}

bool TestContextCapability::paste_clipboard_text() const {
  return context_->runtime.paste_clipboard_text();
}

bool TestContextCapability::copy_selection_to_clipboard() const {
  return context_->runtime.copy_selection_to_clipboard();
}

bool TestContextCapability::cut_selection_to_clipboard() const {
  return context_->runtime.cut_selection_to_clipboard();
}

} // namespace cgpui
