# Upstream Drift Policy

CGPUI Phase J is audited against the pinned Zed/GPUI revision
`5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0`. The checked-in upstream snapshot,
official example matrix, parity ledger, and Phase J completion evidence are an
immutable record of that revision.

## Detection

An upstream review starts by running the strict extractor and auditors against
a newly selected immutable revision. It must compare public re-exports,
official examples, candidate rows, and source hashes with the Phase J snapshot.
A network or parser failure cannot silently fall back to the pinned inventory.

## Delta Ownership

Any change after the pinned revision belongs to Steps 841+ and requires a new
parity delta plan. The delta must name the old and new revisions, classify each
added, changed, or removed public surface, and provide its own behavior,
structure, example, performance, stress, documentation, and platform evidence
where affected.

The delta must not mutate Phase J history, rewrite its accepted adaptations, or
replace its final verification counts. Corrections to factual errors must be
recorded as explicit errata that preserve the original evidence.

## Completion Gate

The new parity delta plan remains open while any required active-platform row
lacks a C++ equivalent or accepted adaptation. Windows, Wayland, and X11 must
repeat affected matrices. macOS follows the scope explicitly authorized for the
delta rather than being inferred from Phase J's non-macOS closeout.
