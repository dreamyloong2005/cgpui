# Ordinary transparency: validated next slice

Baseline `65eb6ae6`, branch `phase-v`. Continue the DX12 backend work without
changing Vulkan/default/auto or failure-only Popup fallback. The adopted policy
retains private DComp, not public window presentation modes. Preserve `.vscode/`,
the design checkout and the user's Linux checkout. Local commit only, no push.

## Questions and ownership

1. Can a normal redirected HWND preserve native HMENU pixels while a direct DX12
   composition visual presents real desktop alpha? Probe DWM backing recipes
   before adding production support. GPU capture and API success are insufficient.
2. Explicit ordinary `transparent(true)` currently asks for an alpha surface but
   the runtime's default clear only considers an applied backdrop. Lock down the
   real root/secondary runtime draw path, then fix the focused backdrop leaf.
3. A successful initial topology is not dynamic-route acceptance: existing HWND
   flip frames, restoration, native menu changes, backdrop re-evaluation and
   device recovery must also be validated before support is advertised.

The topology probe is throwaway and deliberately bypasses production policy;
archive it outside the production tree. Runtime color selection stays private;
no public API or backend switch. Keep structural caps unchanged.

## Sequence

- [done] Real desktop topology experiments with a green backing, two
  distinct frames, alpha 0/0.5/1 and a native nonclient HMENU draw marker.
- [done] Root/secondary runtime regression RED, focused fix GREEN,
  explicit-intent vs material-request and actual-alpha capability matrix.
- [done] Decide the bounded deliverable from actual probe evidence. Do not
  claim ordinary transparency finished if native menu coexistence fails.
- [done] Independent Standards/Spec reviews, targeted/whole Windows + WSL
  regression, appropriate native/ASan coverage and local commit.

Delivered slice: runtime explicit-transparent clear fix plus archived native
topology evidence. Ordinary DX12 transparency/HMENU support is still incomplete;
no tested recipe is enabled. See `acceptance.md` for scope and final verification.

Root runs builds/tests serially: MCPP_JOBS=4 for no-run build, 1 for execution.
Evidence: `D:/Dev/shots/cgpui-dx12-ordinary-20261123/`.
