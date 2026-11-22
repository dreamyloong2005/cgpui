#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeTextState {
  std::vector<TextEditBinding> text_edit_bindings_;
  std::unordered_map<std::uint64_t, TextModel*> text_models_;
  Clipboard* clipboard_ = nullptr;
  std::optional<ImeTextInputPlacement> applied_ime_text_input_placement_;
};

} // namespace cgpui
