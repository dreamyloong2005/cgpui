# Text Input Routing Design

## Goal

Route text input into the `TextModel` bound to the focused element.

## Decisions

- `WindowRuntime` stores non-owning `TextModel*` bindings by `ElementId`.
- `bind_text_model(ElementId, TextModel*)` installs a model binding; passing
  `nullptr` removes the binding.
- On `TextInput`, the runtime checks the current element keyboard-focus owner
  and inserts the text into that element's bound model if present.
- View event delivery remains intact after model insertion so existing
  observation and handler behavior is preserved.

## Non-Goals

- No text selection, IME composition, or undo stack.
- No ownership of text models by the runtime.
- No validation against a live element tree.
