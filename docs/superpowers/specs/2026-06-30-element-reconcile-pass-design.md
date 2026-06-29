# Element Reconcile Pass Design

## Goal

Add a first reconcile pass that preserves `ElementId` values across simple
root and child rebuilds.

## API Shape

- `ElementTree::reconcile_root(std::unique_ptr<Element>)` updates or creates
  the root while preserving the current root id when one exists.
- `ElementTree::reconcile_child(ElementId parent, std::size_t index,
  std::unique_ptr<Element>)` updates or creates a child at a parent-local index.
- Existing lookup APIs keep exposing ids, parent links, and children in order.

## Behavior

- Reconciled roots preserve their previous id and replace the element instance.
- Reconciled children preserve the id already present at the requested index.
- Reconciling past the current end appends a new child.
- Reconciling against an unknown parent or with an empty element returns
  `ElementId{0}`.
- This step does not implement keyed diffing, deletion pruning, layout, hit
  testing, or event routing.
