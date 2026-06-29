# Element Base API Design

## Goal

Introduce the smallest element identity layer needed before building an element
tree, reconcile pass, layout, and routing.

## API Shape

- `ElementId` is a typed numeric id wrapper with `value` and equality.
- `Element` is a polymorphic base class with a virtual destructor.
- `Element::id()` returns the currently assigned id, defaulting to
  `ElementId{0}`.
- `Element::assign_id(ElementId)` binds the runtime/tree-assigned id.

## Behavior

- `ElementId{0}` stays invalid/unassigned.
- Elements do not allocate ids themselves.
- There are no children, parent links, layout methods, hit testing, or event
  hooks in this step.
- Later tree and reconcile steps will assign ids and preserve them across
  rebuilds.
