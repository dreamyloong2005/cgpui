# DX12 window compatibility: next falsifiable paths

Date: 2026-11-24. Read-only investigation against baseline `241b226a` and the
current checkout. This note is planning, not feature acceptance. No candidate
below was implemented or run during this investigation. No production gate may
be relaxed on the strength of this note, and no claim of completed DX12/window
parity follows from it.

## Existing evidence: reuse, do not repeat rejected recipes

- `docs/dx12-ordinary-transparency-probe.md` records the ordinary transparency
  experiments and the later native-menu alpha-only repair.
- `.planning/2026-11-23-dx12-client-alpha/native-menu-findings.md` records the
  opaque client-titlebar/HMENU geometry failure, failed adjusted NCCALCSIZE
  candidate, and rejected captionless topology.
- Original transition source is archived at local commit
  `65fc087703985e7070e32cade39d8b42b5482035`, branch
  `codex/prototype-dx12-native-menu-alpha`, file
  `tests/renderer-dx12/dx12_ordinary_transition_probe_test.cpp`.
- Raw transition evidence:
  `D:/Dev/shots/cgpui-dx12-nc-alpha-20261123/transition-warp.log` and
  `D:/Dev/shots/cgpui-dx12-nc-alpha-20261123/transition-three-warp.log`.

The raw logs distinguish three results. Fresh composition displayed real alpha.
The same HWND after retired opaque HWND-flip presentation still displayed stale
client pixels despite exact GPU captures and `state_expired=1`. Composition from
the first opaque frame passed its own fixed-state/device/queue/swapchain toggle
control. The archived source explicitly excludes native-menu and rollback
claims. Its overall process remains RED because the same-HWND flip transition
fails. Releasing more references or repeating that transition is not a new path.

## Priority 1: system-caption composition from the first frame

The next substantive question is whether a **private default presentation route**
can support the first opaque-to-alpha request while retaining ordinary-window
contracts. Start with fresh, CGPUI-managed, decorated system-caption windows on
Windows build 26100+, including real application HMENU bars. Do not introduce an
application-facing renderer/window mode, switch rendering backends, hide menus,
or silently rewrite native chrome.

This is a new acceptance question extending the archived positive control; the
old no-menu/no-rollback result does not answer it.

### Private state must distinguish separate facts

Currently `Dx12State::composition_surface` controls the presentation route,
swapchain alpha/flags, composition host, clear premultiplication, blur pipeline
choice and public composition report. An opaque composition route cannot safely
be implemented by merely setting that boolean.

Keep private presentation route, effective desktop-alpha policy, requested
intent and alpha-lease ownership distinct. In particular, acquiring the current
active alpha lease for every initially opaque window would unexpectedly freeze
ordinary chrome changes and reject native-menu installation on client-titlebar
windows. An opaque composition candidate should not acquire that restrictive
lease merely to obtain a composition target. Preserve the existing opaque
worker-thread factory behavior; first alpha acquisition still requires the HWND
owner thread and the platform's instance/cookie and menu-preflight safeguards.

### GPU alpha is not the same contract as desktop opacity

Normal opaque runtime clearing must remain alpha 1, with partial-alpha primitives
blended over that opaque background. Also test the lower-level renderer seam:
explicit clears with alpha 0, 0.5 and 1 must be compared with the existing opaque
route for both captured GPU RGBA and desktop RGB. HWND presentation that ignores
alpha can display unscaled RGB even when a captured alpha channel is fractional.
A premultiplied composition candidate must not silently darken those RGB values,
expose the desktop, or change captured-alpha semantics without an explicit
contract decision. Do not loosen either oracle just to make the default switch
pass. A composition swapchain alone is not proof of desktop transparency when
the redirected backing remains opaque.

The first candidate is the archived fixed-premultiplied-chain approach, expanded
to these public contracts. If it cannot preserve opaque semantics, a distinct,
unproven alternative is an opaque **composition** swapchain with
`DXGI_ALPHA_MODE_IGNORE`, followed by a premultiplied composition candidate
swapchain attached through `SetContent`/`Commit` when upgrading. This never uses
the rejected opaque HWND-flip history. The generic alpha enum documents IGNORE,
but actual creation, presentation, menu preservation and transition behavior of
this candidate still require native validation. Do not claim support from the
enum alone.

### Minimum falsifiable experiment matrix

1. Create the real platform window and HMENU; stabilize same-window opaque RGB
   and focus references before renderer creation. Preserve native menu/model and
   accelerator identities. Create the candidate opaque composition renderer
   without first presenting an HWND flip chain.
2. Verify opaque runtime clear, explicit fractional-alpha clears, primitive
   blending and complete desktop opacity while the independently owned backing
   window changes green/blue without a new target present.
3. Make the real transparency request, then render alpha 0/0.5/1 bands and change
   both source content and backing color. Require exact GPU results and bounded
   stable desktop results, not just successful HRESULTs or a green-looking frame.
4. Require exact whole-native-menu RGB/focus before, during and after genuine
   selection and Escape-cancel loops. Retain the event-driven alpha-only repair;
   no hidden test-side repair, owner-draw substitution or undocumented UAH hooks.
5. Exercise menu install/remove/replace, resize and renderer scale, Mica/none,
   device removal/reconstruction, retained frames and renderer teardown. Test
   same HWND and adapter identity, expected generation changes and no stale
   CPU/GPU owner retention. Physical input/monitor migration need separate proof.
6. Existing instances with recorded HWND-flip history remain refused. A new
   default cannot retroactively erase that proven hazard.

### Rollback remains unknown

Inject failures in composition creation/binding/commit, DWM alpha enable and menu
repair. After a rejected operation, inspect HWND/style, HMENU/model/accelerators,
effective composition report, pixels, generation and last-successful diagnostic
storage. Keep the old native owners until completion and a successful handoff
are proven. Never publish the candidate's requested mode as applied on failure.

DirectComposition batches are atomic **within one composition device**. That is
not a transaction encompassing DWM attributes, HWND painting, GPU queue work or
native menu callbacks. If restoring the old attribute or visual also fails, the
implementation must report the failed/uncertain state rather than pretend that
rollback succeeded. Initial opaque fallback, if considered, is allowed only
before candidate publication and must preserve truthful capability/refusal and
flip-history tracking; it is not a silent rendering-backend fallback.

## Separate track: intentionally undecorated initial alpha

This must not be conflated with removing WS_CAPTION to make a native menu fit.
The existing `decorations=false` request intentionally creates a normal CGPUI
window with WS_POPUP, without the system-caption contract. A fresh managed window
of this requested shape is a distinct, falsifiable initial-alpha experiment on
modern Windows, using the existing DWM attribute/private composition mechanism.
Preserve its requested style, activation, placement, resize and input behavior;
do not relabel it as a nonactivating tooltip. Begin with no attached application
HMENU and do not hide an existing menu to satisfy that condition. Menu admission
and later chrome changes need explicit validation before broadening support.

By contrast, `titlebar_visible=false` with `decorations=true` currently removes
WS_CAPTION while retaining WS_SYSMENU in the style helper. Audit that existing
opaque contract independently; do not treat it as equivalent to deliberately
undecorated chrome or use it to bypass the earlier native-style objection.

## Still-independent blockers

### Client title bar plus application HMENU

All-composition presentation does not relocate the native menu. The current
client NCCALCSIZE calculation ignores its band and custom hit testing never
routes HTMENU. Merely reserving client height, adding HTMENU, repairing alpha or
repeating SetMenu/DrawMenuBar does not overcome the demonstrated placement
failure. The only successful archived relocation experiment explicitly removed
WS_CAPTION and was not adopted because of the documented WS_SYSMENU prerequisite.
No newly verified documented menu-origin mechanism was found in this review.
Keep the refusal until an actual platform-layout solution preserves native
drawing, interaction, resize/system-menu behavior and the strict geometry
contract. Do not weaken geometry assertions to promote the rejected candidate.

### Windows versions before build 26100

An unproven compatibility candidate combines the existing legacy blur-region
alpha path with the later native-menu alpha-only repair. The earlier 33 negative
recipe cases did not include that repair, so the paired mechanism asks a
different question. It must run on an actual supported older Windows host;
disabling the newer attribute on a modern host is not older-OS validation.
Separate material/blur ownership, composition-change notifications, teardown and
rollback from the existing Mica/frame-extension implementation. Preserve the
version gate until this evidence exists. Do not infer that every old-Windows
topology is impossible, or that the candidate has already succeeded there.

## Official references checked during this investigation

- [CreateSwapChainForComposition](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/nf-dxgi1_2-idxgifactory2-createswapchainforcomposition):
  requires FLIP_SEQUENTIAL and STRETCH; SetFullscreenState, ResizeTarget,
  GetContainingOutput, GetHwnd and GetCoreWindow are not valid for this swapchain
  kind. Window-style fullscreen must be tested separately, not equated with an
  exclusive fullscreen swapchain.
- [DXGI_ALPHA_MODE](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_2/ne-dxgi1_2-dxgi_alpha_mode):
  defines premultiplied behavior and IGNORE; values above alpha in premultiplied
  color can create additive blending rather than ordinary straight-alpha output.
- [CreateTargetForHwnd](https://learn.microsoft.com/en-us/windows/win32/api/dcomp/nf-dcomp-idcompositiondevice-createtargetforhwnd):
  target HWND must belong to the process; at most one topmost and one non-topmost
  target can bind each HWND; composition layering and window clipping apply.
- [Commit](https://learn.microsoft.com/en-us/windows/win32/api/dcomp/nf-dcomp-idcompositiondevice-commit):
  composition methods batch into a device-local atomic transaction.
- [WaitForCommitCompletion](https://learn.microsoft.com/en-us/windows/win32/api/dcomp/nf-dcomp-idcompositiondevice-waitforcommitcompletion):
  waits for processing the previous composition commit; not interchangeable with
  the renderer's GPU fence and not a justification for an unbounded failure wait.
- [DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute):
  REDIRECTIONBITMAP_ALPHA defaults false/opaque and is supported from Windows 11
  build 26100; enabled redirected pixels must contain premultiplied alpha.
- [DwmEnableBlurBehindWindow](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmenableblurbehindwindow):
  discusses honoring window alpha and GDI alpha hazards, top-level-only use and
  WM_DWMCOMPOSITIONCHANGED handling; Windows 8 onward no longer produces the
  earlier blur effect merely by calling this function.

Next implementation decision should follow the strict system-caption experiment
or the separate intentionally-undecorated experiment, not another micro-
optimization used as a substitute for resolving window compatibility.
