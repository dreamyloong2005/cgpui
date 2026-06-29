# Element Tree Container Design

## Goal

Add a minimal owning element tree container that can store a root element,
children, and parent links before reconcile, layout, and routing exist.

## API Shape

- `ElementTree::set_root(std::unique_ptr<Element>)` stores the root element and
  assigns its id.
- `ElementTree::append_child(ElementId parent, std::unique_ptr<Element>)`
  stores a child, assigns its id, and records its parent.
- `ElementTree::root_id()` returns the current root id or `ElementId{0}`.
- `ElementTree::get(ElementId)` returns a mutable or const element pointer.
- `ElementTree::parent(ElementId)` returns the parent id when present.
- `ElementTree::children(ElementId)` returns child ids in append order.

## Behavior

- `ElementId{0}` stays invalid.
- Tree-assigned ids are monotonic and never reused.
- Setting a new root clears the previous tree.
- Appending under an unknown parent returns `ElementId{0}` and does not take
  ownership.
- No reconcile, layout, hit testing, or event routing behavior is introduced in
  this step.
