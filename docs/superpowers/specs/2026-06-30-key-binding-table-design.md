# Key Binding Table Design

## Goal

Map keyboard events to named actions so the runtime can trigger semantic
commands from key input.

## Decisions

- Add a simple `KeyBinding` value with key code, key action, modifiers, and an
  action name.
- Store bindings on `WindowRuntime` for now, beside the runtime-local action
  table.
- On `KeyboardKey` events, find the first binding that exactly matches key
  code, key action, and modifiers, then dispatch its named action.
- Keep view event dispatch intact after key binding lookup.

## Non-Goals

- No platform-specific accelerator translation.
- No key chord sequences or binding priority scopes.
- No event cancellation policy tied to action results yet.
