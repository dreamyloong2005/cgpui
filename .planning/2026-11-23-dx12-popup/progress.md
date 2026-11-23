# Evidence and progress

The public native desktop tracer failed on baseline with expected exit4:
`popup-red.log`, 0.36s. The independent green backing-window screen control was
correct; factory rejected unsupported transparent/composition presentation.

`popup-green.log` passes after direct DX12 composition hosting, 0.82s:
texture RGBA (128,0,0,128)/(0,0,0,0)/(255,0,0,255) and actual screen
(128,127,0)/(0,255,0)/(255,0,0); the next frame changes red to blue and checks
both texture and desktop again. This is actual desktop alpha evidence, not just
successful APIs or alpha in readback. WARP+Debug was explicitly selected.

Existing Vulkan bridge/platform code is unchanged. The new DX12 host owns its
native lifetime separately, uses no D3D11 or readback presentation, and commits
the visual only after a successful presented/validated frame. Composition and
ordinary HWND paths keep separate private swapchain flags. Surface policy lives
in its own leaf and validates the HWND's actual redirection style.

The transparent backdrop issue was found by source inspection before claiming
full Popup rendering: source-over of a blurred premultiplied scene would add its
alpha a second time, while the existing opaque shader ignores source alpha.
The next tracer uses independent literal RGBA expectations for replacement.

## Alpha regression and expanded coverage

`backdrop-alpha-red.log` and its repeat both fail exit13 (2.44s/2.40s):
an all-zero transparent scene becomes texture (0,0,0,255) and desktop black.
`diagnosing-bugs` was loaded; ranked hypotheses were shared: dropped source alpha
in the composite shader, repeated source-over alpha, or compositor presentation.
The existing successful alpha tracer plus the erroneous texture alpha localize
the fault before DWM. The dedicated replacement shader/PSO fixes the first two
without changing existing opaque/Vulkan entries. `backdrop-alpha-green.log`
passes all five literal cases with matching desktop pixels (1.10s).

The normal shader generator refreshed DXIL and the Vulkan backdrop source hash.
Vulkan's 2873 SPIR-V words are byte-identical (`shader-scope.json`). It also
regenerated unrelated Metal outputs with a different compiler version; those
agent-generated changes were restored immediately, leaving Metal unchanged.

The first expanded no-run build missed the native platform library; adding the
test-only platform-win32 dependency fixed that link error. No product fallback
or stub was added. `popup-suite-green.log` passes the initial six Popup programs;
`native-complete-test.log` passes all 53 native programs, including independent
Gaussian-CDF transparent blur edges, rounded masks/clips and ordered snapshots,
mixed image/text/shadow/rounded draws, real platform Popup and pointer-transparent
Tooltip, resize/DPI, two recovery epochs and orphan/duplicate-target lifetimes.
The recovery scene also creates the alpha-backdrop PSO before removal.

`architecture-initial-test.log` passes all 204 structure programs. Final reviews
and frozen full-platform/native/SDK/ASan validation remain before acceptance.

Reviews: Standards/native lifetime has no hard violations; one nonblocking
possible shader-code duplication is accepted for this slice to keep existing
opaque/Vulkan generated instructions unchanged. Spec and independent native
lifetime review report no actionable product findings. The latter requested
explicit invalid native-host validation coverage; the surface test now checks
both redirection/descriptor mismatches and a destroyed HWND, followed by valid
creation on the original live HWNDs. Full source freeze follows its targeted run.

## Frozen acceptance

The final host-validation tracer passes. Frozen manifest: 2727 source files.
Both serial validation drivers exit 0: Windows 717/717, WSL 637/637, architecture
204, public headers 47, neutral renderer 15; both platforms' all-library/example
builds and installed-only SDK consumers; DX12 ASan+WARP+Debug 53/53; Intel+Debug,
NVIDIA+Debug and WARP+Debug+GPU validation each 53/53. SDK fixtures pass 4+15.
All three native matrix logs exercise distinct hardware/WARP LUIDs without SKIP.
Windows raw-byte and WSL sync-normalized source comparisons match all 2727 paths.
Only closeout planning notes changed after verification. See acceptance.md for
evidence, review disposition and explicit limits.
