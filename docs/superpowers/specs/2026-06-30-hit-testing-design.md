# Hit Testing Design

## Goal

Add a small retained-geometry hit-testing API over elements that have already
been laid out.

## Decisions

- `Element::layout` records the bounds it returned so callers can inspect the
  most recent laid-out rectangle.
- `Element::hit_test(Point)` returns the element's id when the point is inside
  its retained bounds, or `ElementId{0}` when it is outside or before layout.
- `VerticalStackElement` records each child at its top-to-bottom laid-out
  origin and hit-tests children before returning the stack id.
- Hit testing is purely element-local in this slice. Pointer event routing,
  pointer capture, and runtime dispatch integration remain later steps.

## Non-Goals

- No pointer event delivery in this slice.
- No `ElementTree` layout traversal.
- No keyed z-order or overlap policy beyond reverse child order for container
  hit testing.
