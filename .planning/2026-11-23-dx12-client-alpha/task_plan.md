# Initial DX12 alpha with client-drawn title bars

Baseline4ce08c0a, phase-v. Preserve user.vscode; no push. Existing system-caption
alpha/native menu behavior, opaque HWNDflip path, backend defaults and native-first
Popup policies stay unchanged. Root owns all builds, serial execution.

## Feedback-loop-first scope

1. Public client-titlebar initial transparent factory must reproduce current
   unsupported rejection. No pre-test apply_window_chrome workaround. Require
   correct initial frame metrics/style/client geometry and hit-test regions.
2. Independently validate opaque client-titlebar + true native HMENU before
   enabling alpha. Existing WM_NCCALCSIZE does not reserve menu space and custom
   WM_NCHITTEST does not return HTMENU. A blank opaque reference is not success.
   Preserve actual native menu drawing/interaction rather than hiding/replacing it.
3. If viable, put menu geometry/input in a focused private platform leaf, with
   behavior and structure guards. Broad message owner only delegates; keep caps.
4. Enable only experimentally verified initial decorated client-titlebar alpha
   on Windows>=26100. Keep raw/undecorated/olderOS/fullscreen and opaque upgrades
   unsupported. Don't inadvertently enable live system/client topology switching.
5. Public GPU+desktop+native menu, controls/resize/lifetime/recovery/material
   regression coverage, strict reference RGB, no hidden repair/manual workaround.
6. Independent two-axis review, full relevant Windows/WSL/native/ASan validation,
   documentation and local commit after verification.

Disk: D~340MiB. Asked to remove only prior disposable SDKcopies; no permission
received and no cleanup performed. Use existing incremental outputs; do not make
another full SDK copy. Prior static link closure remains relevant unless changed.
Evidence: D:/Dev/shots/cgpui-dx12-client-alpha-20261123/.


## Evidence-driven boundary decision

- Public initial no-menu client factory RED exit7, then GREEN with body/title
  row alpha0/.5/1 and real desktop pixels; resize/lifetime/Mica/recovery pass.
- Opaque client+nativeHMENU baseline RED exit6 (menu overlapsclient); adjusted
  NCCALCSIZE candidate also RED. Captionless raw style probe v2 passed geometry,
  real menu/systemmenu loops and native state changes, but violates documented
  WS_SYSMENU prerequisite WS_CAPTION, so NOT adopted. No style rewrite ships.
- Candidates/probe files archived under local
  codex/prototype-dx12-client-menu-20261123 at
  bb343e0a77a9ad1d99b56d51c31b6a3c31b410bc and removed from production.
  (That branch was deleted on 2026-11-25 without merging; its source is gone.)
- Final bounded scope: initial decorated visible client titlebar alpha WITHOUT
  native HMENU; active mode stays fixed. Existing menu blocks factory; active
  client alpha blocks nonempty application menu installation, preserving model,
  accelerators and HWNDs. Nothing is hidden or replaced.
- Menu admission belongs to private platform RAII policy leaf. App adds only
  preflight delegation. Thread reservation spans mutation callbacks, blocking
  reentrant installers and client-alpha creation even on a temporarily menu-free
  target. Reservation is set after read-only allocation-bearing preflight to
  avoid leaving a latch if construction throws.
- Public menu refusal RED exit4 then GREEN; actual native install and clear each
  invoke a WindowResized callback while candidate target has no HMENU, proving
  reentrant factory rejection is not an accidental existing-menu gate.
- All206 architecture tests and platform alpha lease pass. Native controls
  test initially assumed a WindowMinimized event not emitted by existing Win32;
  corrected oracle requires actual IsIconic and public lifecycle display state,
  not a new unrelated event implementation. Retest GREEN with actual native
  maximize/restore/minimize and close cancellation; no physical input claim.

Validation uses no new full SDK library copies. New installed-only consumer will
hardlink current built libraries within D: to avoid ~700MiB duplication; headers
and executable are still external to workspace and no source include paths used.
No SDK cleanup permission received; previous copies remain untouched.

## Bounded milestone completion

- [x] No-menu client initial-alpha behavior, focused policy and structure tests.
- [x] Preserve native styles/menu commands; archive rejected geometry/style probes.
- [x] Menu refusal and real native callback reentry coverage.
- [x] Initial pixels, lifetime, recovery, material and controls coverage.
- [x] Three public docs, independent Standards/Spec/docs review.
- [x] Windows 739, WSL 641; three native adapter runs of 70 and ASan 70.
- [x] SDK fixtures and installed-only consumer; frozen-source identity checks.
- [x] Record limitations, red-to-green history and all evidence in acceptance.md.
- [x] Local commit on phase-v; no push, preserve user .vscode.
