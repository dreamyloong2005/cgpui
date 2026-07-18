# Phase J macOS Completion Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reopen Phase J Steps 799-840 in place and complete official examples, performance, stress, deterministic Release packaging, CI, and final native verification for macOS arm64 and x86_64 without rewriting prior platform evidence.

**Architecture:** Existing platform-neutral Phase J runners and validators remain the source of behavior; focused macOS shell entry points provide native host, architecture, deployment-target, and output-root policy. A new thin Xmake module owns two completion guards, the Windows/Linux workflow becomes the all-desktop workflow, and final authority changes from `complete_non_macos` to `complete` only after downloaded artifacts and successful native jobs prove both architectures.

**Tech Stack:** C++23, Objective-C++, Xmake 3.0.9, Bash, Python 3.12, JSON, GitHub Actions, Cocoa/AppKit, Metal, Git, GitHub CLI.

---

## File Ownership Map

- `build/xmake/phase_j_macos_targets.lua` owns only the two Phase J macOS completion guard targets.
- `tests/api_parity/phase_j_macos_completion_test.cpp` owns cross-file behavior and authority invariants.
- `tests/architecture/phase_j_macos_completion_structure_test.cpp` owns file placement, thin-root registration, workflow rename, executable-script, and line-budget invariants.
- `scripts/ci/macos-example-smoke.sh` owns the canonical 21-target example build/run plus macOS-native smoke/pixel/input test selection.
- `scripts/ci/macos-performance-baseline.sh` and `scripts/ci/macos-stress-matrix.sh` each own one runner invocation and one structured validator invocation.
- `scripts/ci/macos-package.sh` owns one native architecture-specific package tree and its base manifest.
- `scripts/ci/macos-phase-j-release.sh` owns two-pass archive reproduction and archive-content verification for one native architecture.
- `scripts/ci/macos-dependencies.sh` owns repository-local Xmake roots and the pinned macOS Xmake repository checkout shared by local runs and CI.
- `scripts/ci/macos-architecture-header.sh` owns the macOS architecture/header target matrix.
- `scripts/ci/create-release-artifact.py` remains the single deterministic archive writer.
- `.github/actions/setup-phase-j-dependencies/action.yml` owns OS-specific Xmake dependency roots; `.github/workflows/phase-j-desktop.yml` owns the desktop job matrices.
- `docs/gpui-*.json`, the roadmap, ledgers, README, and focused guides own policy and evidence. Historical Phase H, Windows, Wayland, and X11 values are never recomputed.

## Fixed Contracts

- Native architectures are exactly `arm64` and `x86_64`; `uname -m` must equal the requested value.
- Every macOS configure/build/test entry exports `MACOSX_DEPLOYMENT_TARGET=13.0`.
- Xmake must report `xmake v3.0.9`; all build/test commands use noninteractive
  `-y` and `-j 1` where the subcommand accepts those options.
- Release names are exactly `cgpui-macos-arm64-release.tar.gz` and `cgpui-macos-x86_64-release.tar.gz`.
- Runners record platform as `macos-arm64` or `macos-x86_64`.
- Current authority stays `complete_non_macos` while either native architecture is unverified.
- No task changes the pinned upstream revision or Steps 841+ drift policy.

After Task 1 creates `macos-dependencies.sh`, every later command block that
invokes Xmake must first activate the pinned environment in the same shell:

```bash
source scripts/ci/macos-dependencies.sh
cgpui_configure_macos_dependency_environment \
  build/phase-j-implementation/dependencies/macos-arm64
```

This is required even when a developer already has a global Xmake repository;
global repository state is not Phase J evidence.

### Task 1: Add Staged macOS Completion Guards

**Files:**
- Create: `scripts/ci/macos-dependencies.sh`
- Create: `build/xmake/phase_j_macos_targets.lua`
- Create: `tests/api_parity/phase_j_macos_completion_test.cpp`
- Create: `tests/architecture/phase_j_macos_completion_structure_test.cpp`
- Modify: `xmake.lua:4132`
- Modify: `scripts/ci/architecture-header-targets.txt`

- [x] **Step 1: Establish the pinned local dependency environment**

Create the helper with function
`cgpui_configure_macos_dependency_environment <root> [--github-env]`. It
resolves the root using the nearest existing ancestor, rejects `..`, the
repository root, and paths outside the repository, then exports five roots and
checks out the fixed repository:

```bash
#!/usr/bin/env bash
set -euo pipefail
cgpui_configure_macos_dependency_environment() {
  [[ "$(uname -s)" == "Darwin" ]] || return 2
  local repo_root input candidate probe suffix root export_github
  repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
  input=${1:-build/phase-j-ci/dependencies/macos-$(uname -m)}
  export_github=${2:-}
  [[ "/$input/" != */../* ]] || return 2
  if [[ "$input" == /* ]]; then candidate="$input"; else candidate="$repo_root/$input"; fi
  probe="$candidate"; suffix=""
  while [[ ! -e "$probe" ]]; do
    suffix="/$(basename "$probe")$suffix"; probe="$(dirname "$probe")"
  done
  [[ -d "$probe" ]] || return 2
  root="$(cd "$probe" && pwd -P)$suffix"
  [[ "$root" != "$repo_root" && "$root/" == "$repo_root/"* ]] || return 2
  export XMAKE_CONFIGDIR="$root/config"
  export XMAKE_GLOBALDIR="$root/global"
  export XMAKE_PKG_CACHEDIR="$root/pkg-cache"
  export XMAKE_PKG_INSTALLDIR="$root/pkg-install"
  export TMPDIR="$root/tmp"
  mkdir -p "$XMAKE_CONFIGDIR" "$XMAKE_GLOBALDIR/.xmake/repositories" \
    "$XMAKE_PKG_CACHEDIR" "$XMAKE_PKG_INSTALLDIR" "$TMPDIR"
  local repository="$XMAKE_GLOBALDIR/.xmake/repositories/xmake-repo"
  local commit=b9256335e0b6e70808e23dfe71627d8a4dcc0abf
  [[ -d "$repository/.git" ]] || \
    git clone --no-checkout https://github.com/xmake-io/xmake-repo.git "$repository"
  git -C "$repository" fetch --depth 1 origin "$commit"
  git -C "$repository" checkout --detach "$commit"
  touch "$repository/updated"
  local locked="$XMAKE_CONFIGDIR/repositories/efa340bf3b6f6de54c5f0ab8c98fba7d.lock"
  if [[ ! -d "$locked/.git" ]]; then
    mkdir -p "$(dirname "$locked")"; git clone --shared "$repository" "$locked"
  fi
  git -C "$locked" fetch "$repository" "$commit"
  git -C "$locked" checkout --detach "$commit"
  if [[ "$export_github" == "--github-env" ]]; then
    [[ -n "${GITHUB_ENV:-}" ]] || return 2
    printf '%s\n' "XMAKE_CONFIGDIR=$XMAKE_CONFIGDIR" \
      "XMAKE_GLOBALDIR=$XMAKE_GLOBALDIR" \
      "XMAKE_PKG_CACHEDIR=$XMAKE_PKG_CACHEDIR" \
      "XMAKE_PKG_INSTALLDIR=$XMAKE_PKG_INSTALLDIR" "TMPDIR=$TMPDIR" >> "$GITHUB_ENV"
  elif [[ -n "$export_github" ]]; then
    return 2
  fi
}
if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  cgpui_configure_macos_dependency_environment "$@"
fi
```

Run `bash -n scripts/ci/macos-dependencies.sh`, source it, invoke it with the
fixed implementation dependency root above, and require the checked-out commit
from `git -C "$XMAKE_GLOBALDIR/.xmake/repositories/xmake-repo" rev-parse HEAD`.

- [x] **Step 2: Write the behavior guard with staged failure bands**

Create a source-only guard using the repository's `read_source`, `contains`, and
`count` helpers. The main body must use stable exit bands so later tasks have an
exact RED reason:

```cpp
int main() {
  const std::string matrix = read_source("docs/gpui-official-example-matrix.json");
  const std::string example_audit = read_source("docs/gpui-phase-j-example-audit.json");
  const std::string performance = read_source("docs/gpui-performance-baseline-policy.json");
  const std::string stress = read_source("docs/gpui-stress-profile.json");
  const std::string release = read_source("docs/gpui-release-artifact-policy.json");
  const std::string packer = read_source("scripts/ci/create-release-artifact.py");
  const std::string package = read_source("scripts/ci/macos-package.sh");
  const std::string release_script = read_source("scripts/ci/macos-phase-j-release.sh");
  const std::string performance_script = read_source("scripts/ci/macos-performance-baseline.sh");
  const std::string stress_script = read_source("scripts/ci/macos-stress-matrix.sh");
  const std::string architecture_script = read_source("scripts/ci/macos-architecture-header.sh");
  const std::string workflow = read_source(".github/workflows/phase-j-desktop.yml");
  const std::string verification = read_source("docs/gpui-phase-j-final-verification.json");
  const std::string ledger = read_source("docs/gpui-complete-parity-ledger.json");
  const std::string ledger_md = read_source("docs/gpui-complete-parity-ledger.md");

  if (count(matrix, "\"macos\": \"compiled_and_smoked\"") != 20 ||
      !contains(example_audit, "\"macos_execution\": \"required\"")) return 20;
  if (!contains(performance, "\"macos-arm64\"") ||
      !contains(performance, "\"macos-x86_64\"") || performance_script.empty()) return 30;
  if (!contains(stress, "\"macos-arm64\"") ||
      !contains(stress, "\"macos-x86_64\"") || stress_script.empty()) return 40;
  if (package.empty() || release_script.empty() ||
      !contains(release, "cgpui-macos-arm64-release.tar.gz") ||
      !contains(release, "cgpui-macos-x86_64-release.tar.gz") ||
      !contains(packer, "\"macos\"")) return 50;
  if (workflow.empty() || architecture_script.empty() ||
      !contains(workflow, "macos-15-intel") ||
      !contains(workflow, "MACOSX_DEPLOYMENT_TARGET: \"13.0\"")) return 60;
  const bool pending = contains(ledger,
      "\"phase_j_macos_completion_status\": \"pending_native_dual_arch_verification\"");
  const bool complete = contains(ledger, "\"phase_j_status\": \"complete\"") &&
      contains(verification, "\"scope\": \"all_desktop\"") &&
      contains(verification, "\"status\": \"passed\"");
  if ((!pending && !complete) ||
      !contains(ledger_md, "Phase J macOS completion") ||
      !contains(verification, "\"arm64\"") ||
      !contains(verification, "\"x86_64\"")) return 70;
  return 0;
}
```

- [x] **Step 3: Write the structure guard before creating the owned files**

Require the thin root include, the new module, all focused scripts, the
desktop workflow, and architecture manifest registration. Enforce these line
budgets: module 30, behavior guard 130, structure guard 120, each thin wrapper
100, dependency helper 150, package 230, Release wrapper 150, and desktop
workflow 360 lines.

```cpp
const std::array required_paths{
    "scripts/ci/macos-package.sh",
    "scripts/ci/macos-phase-j-release.sh",
    "scripts/ci/macos-dependencies.sh",
    "scripts/ci/macos-performance-baseline.sh",
    "scripts/ci/macos-stress-matrix.sh",
    "scripts/ci/macos-architecture-header.sh",
    ".github/workflows/phase-j-desktop.yml"};
for (const auto* path : required_paths)
  if (read_source(path).empty()) return 10;
if (!contains(root, "includes(\"build/xmake/phase_j_macos_targets.lua\")") ||
    contains(root, "target(\"phase_j_macos_completion_test\")") ||
    !contains(module, "target(\"phase_j_macos_completion_test\")") ||
    !contains(module, "target(\"phase_j_macos_completion_structure_test\")")) return 20;
if (!contains(manifest,
    "phase_j_macos_completion_structure_test|tests/architecture/"
    "phase_j_macos_completion_structure_test.cpp")) return 30;
```

- [x] **Step 4: Register both guards in a focused Xmake module**

```lua
target("phase_j_macos_completion_structure_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files(path.join(os.projectdir(), "tests/architecture/phase_j_macos_completion_structure_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})

target("phase_j_macos_completion_test")
    set_kind("binary")
    set_rundir(os.projectdir())
    add_runenvs("CGPUI_SOURCE_ROOT", os.projectdir())
    add_files(path.join(os.projectdir(), "tests/api_parity/phase_j_macos_completion_test.cpp"))
    add_tests("default", {rundir = os.projectdir(), runenvs = {CGPUI_SOURCE_ROOT = os.projectdir()}})
```

Add `includes("build/xmake/phase_j_macos_targets.lua")` after the existing
Phase J closeout include and append the exact structure target/source pair to
`scripts/ci/architecture-header-targets.txt`.

- [x] **Step 5: Run the deliberate RED guards**

Run:

```bash
xmake build -P . -y -j 1 phase_j_macos_completion_test
xmake build -P . -y -j 1 phase_j_macos_completion_structure_test
CGPUI_SOURCE_ROOT="$PWD" build/macosx/arm64/release/phase_j_macos_completion_test
CGPUI_SOURCE_ROOT="$PWD" build/macosx/arm64/release/phase_j_macos_completion_structure_test
```

Expected: both targets compile; behavior exits `20` because all 20 matrix rows
still exclude macOS, and structure exits `10` because the focused scripts and
desktop workflow do not exist.

- [x] **Step 6: Commit the RED guard boundary**

```bash
git add scripts/ci/macos-dependencies.sh build/xmake/phase_j_macos_targets.lua \
  tests/api_parity/phase_j_macos_completion_test.cpp \
  tests/architecture/phase_j_macos_completion_structure_test.cpp \
  scripts/ci/architecture-header-targets.txt xmake.lua
git commit -m "test: establish phase j macos completion"
```

### Task 2: Complete the 20-Example macOS Matrix

**Files:**
- Modify: `scripts/ci/macos-example-smoke.sh`
- Delete: `scripts/ci/macos-example-targets.txt`
- Modify: `docs/gpui-official-example-matrix.json`
- Modify: `docs/gpui-phase-j-example-audit.json`
- Modify: `tools/gpui_parity/audit_official_examples.py`
- Modify: `tests/api_parity/phase_j_official_example_audit_test.cpp`
- Modify: `tests/architecture/phase_j_official_example_structure_test.cpp`

- [x] **Step 1: Make the existing focused audit tests RED for macOS**

Replace the exclusion assertions with these requirements:

```cpp
if (count(matrix, "\"macos\": \"compiled_and_smoked\"") != examples.size() ||
    count(matrix, "\"macos_tests\":") != examples.size()) return 3;
if (!contains(report, "\"macos_execution\": \"required\"") ||
    !contains(report, "\"macos_mapped_examples\": 20")) return 5;
```

In the structure guard, require `macos-example-smoke.sh` to read
`scripts/ci/example-targets.txt`, require the duplicate macOS inventory to be
absent via `std::filesystem::exists`, and raise the behavior/structure line
budgets only if the new exact checks exceed the current limits.

Run:

```bash
xmake build -P . -y -j 1 phase_j_official_example_audit_test
xmake build -P . -y -j 1 phase_j_official_example_structure_test
xmake test -P . -y -j 1 -v phase_j_official_example_audit_test/default \
  phase_j_official_example_structure_test/default
```

Expected: `0/2`; the behavior guard reports exit `3`, and the structure guard
reports its new macOS runner/inventory exit.

- [x] **Step 2: Use the canonical example inventory and native test groups**

Delete `scripts/ci/macos-example-targets.txt`. In the runner, validate
`CGPUI_EXPECTED_ARCH`, deployment target, and Xmake before the loop, then use:

```bash
expected_arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
if [[ "$(uname -m)" != "$expected_arch" ]] ||
   [[ ! "$expected_arch" =~ ^(arm64|x86_64)$ ]]; then
  echo "macOS host architecture does not match: $expected_arch" >&2
  exit 3
fi
if [[ "${MACOSX_DEPLOYMENT_TARGET:-}" != "13.0" ]]; then
  echo "MACOSX_DEPLOYMENT_TARGET must be 13.0" >&2
  exit 3
fi
if [[ "$(xmake --version | sed -n '1p')" != *"xmake v3.0.9"* ]]; then
  echo "macos-example-smoke.sh requires xmake v3.0.9" >&2
  exit 3
fi

while IFS= read -r target; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  [[ "$target" =~ ^api_parity_[a-z0-9_]+$ ]] || exit 4
  xmake build -P "$repo_root" -y -j 1 "$target"
  if [[ "$target" == "api_parity_hello_world" ]]; then
    CGPUI_EXIT_AFTER_FIRST_FRAME=1 xmake run -P "$repo_root" "$target"
  else
    xmake run -P "$repo_root" "$target"
  fi
done < "$repo_root/scripts/ci/example-targets.txt"

native_tests=(
  macos_example_smoke_test/default
  metal_primitive_pixel_test/default
  metal_text_image_pixel_test/default
  metal_clip_transform_pixel_test/default
  macos_input_event_test/default
  macos_text_input_test/default
  macos_platform_services_test/default
  macos_accessibility_test/default
)
xmake test -P "$repo_root" -y -j 1 -v "${native_tests[@]}"
```

Retain the four `hello_window/macos_*` smoke modes and run them with `-j 1 -v`.

- [x] **Step 3: Make the matrix and auditor represent required macOS execution**

For each of the 20 rows, change `"macos": "excluded_by_user"` to
`"macos": "compiled_and_smoked"` and add a non-empty `macos_tests` array under
`verification`. Use this exact mapping; every named target is executed by the
runner or its native test groups:

| Upstream example | `macos_tests` |
|---|---|
| `hello_world` | `api_parity_hello_world`, `hello_window/macos_first_frame`, `metal_primitive_pixel_test/default` |
| `move_entity_between_windows` | `api_parity_public_authoring_workflow`, `phase_f_multi_window_lifecycle_churn_test/default` |
| `image` | `api_parity_public_image_example`, `metal_text_image_pixel_test/default` |
| `input` | `api_parity_public_text_input_examples`, `macos_input_event_test/default`, `macos_text_input_test/default` |
| `on_window_close_quit` | `api_parity_public_window_examples_workflow`, `hello_window/macos_close_after_first_frame` |
| `opacity` | `api_parity_public_opacity_example`, `metal_primitive_pixel_test/default` |
| `pattern` | `api_parity_public_svg_image_sources`, `metal_text_image_pixel_test/default` |
| `set_menus` | `api_parity_public_window_examples`, `macos_platform_services_test/default` |
| `shadow` | `api_parity_public_window_examples_workflow`, `metal_primitive_pixel_test/default` |
| `svg` | `api_parity_public_svg_raster_upload`, `metal_text_image_pixel_test/default` |
| `tab_stop` | `api_parity_public_window_examples_interaction_states`, `macos_input_event_test/default` |
| `text` | `api_parity_public_rich_text_examples`, `metal_text_image_pixel_test/default`, `macos_text_input_test/default` |
| `text_wrapper` | `api_parity_public_text_wrapper_examples`, `metal_text_image_pixel_test/default` |
| `tree` | `api_parity_public_window_examples_widget_catalog`, `macos_example_smoke_test/default` |
| `uniform_list` | `api_parity_public_window_examples_widget_catalog`, `macos_example_smoke_test/default` |
| `window_shadow` | `api_parity_public_window_examples_service_matrix`, `metal_primitive_pixel_test/default` |
| `grid_layout` | `api_parity_public_window_examples_widget_catalog`, `metal_clip_transform_pixel_test/default` |
| `list_example` | `api_parity_public_window_examples_widget_catalog`, `macos_example_smoke_test/default` |
| `mouse_pressure` | `api_parity_public_window_examples_interaction_states`, `macos_input_event_test/default` |
| `a11y` | `api_parity_public_window_examples_service_matrix`, `macos_accessibility_test/default` |

For example, the first row becomes:

```json
"verification": {
  "smoke_tests": ["hello_window/windows_first_frame", "hello_window/linux_first_frame", "hello_window/x11_first_frame"],
  "pixel_tests": ["vulkan_frame_pixel_capture_test/default", "wayland_frame_pixel_capture_test/default"],
  "input_tests": [],
  "macos_tests": ["api_parity_hello_world", "hello_window/macos_first_frame", "metal_primitive_pixel_test/default"]
}
```

Update the auditor's platform loop to include `macos`, require a non-empty
`macos_tests` list, and emit:

```python
"macos_execution": "required",
"macos_mapped_examples": sum(
    bool(row.get("verification", {}).get("macos_tests")) for row in rows
),
```

Regenerate the checked-in audit:

```bash
python3 tools/gpui_parity/audit_official_examples.py \
  --snapshot docs/gpui-upstream-snapshot.json \
  --matrix docs/gpui-official-example-matrix.json \
  --targets scripts/ci/example-targets.txt \
  --xmake xmake.lua --output docs/gpui-phase-j-example-audit.json
```

Expected: exit `0`, `official_examples=20`, `mapped_examples=20`,
`macos_mapped_examples=20`, and no issues.

- [x] **Step 4: Run GREEN and syntax verification**

```bash
bash -n scripts/ci/macos-example-smoke.sh
xmake build -P . -y -j 1 phase_j_official_example_audit_test
xmake build -P . -y -j 1 phase_j_official_example_structure_test
xmake test -P . -y -j 1 -v phase_j_official_example_audit_test/default \
  phase_j_official_example_structure_test/default
xmake run -P . phase_j_macos_completion_test; test $? -eq 30
```

Expected: official-example tests pass `2/2`; the staged completion guard advances
from exit `20` to exit `30` because performance is next.

- [x] **Step 5: Commit the example band**

```bash
git add scripts/ci/macos-example-smoke.sh scripts/ci/macos-example-targets.txt \
  docs/gpui-official-example-matrix.json docs/gpui-phase-j-example-audit.json \
  tools/gpui_parity/audit_official_examples.py \
  tests/api_parity/phase_j_official_example_audit_test.cpp \
  tests/architecture/phase_j_official_example_structure_test.cpp
git commit -m "test: require phase j macos examples"
```

### Task 3: Add Architecture-Specific macOS Performance Baselines

**Files:**
- Create: `scripts/ci/macos-performance-baseline.sh`
- Modify: `tests/performance/phase_j_performance_runner.cpp`
- Modify: `tools/gpui_parity/validate_performance_baseline.py`
- Modify: `docs/gpui-performance-baseline-policy.json`
- Modify: `tests/api_parity/phase_j_performance_baseline_audit_test.cpp`
- Modify: `tests/architecture/phase_j_performance_baseline_structure_test.cpp`

- [x] **Step 1: Add RED assertions for active macOS platforms**

Require the policy, runner, validator, and wrapper to contain both
`macos-arm64` and `macos-x86_64`; require the validator to compare
`report["platform"]` against `policy["platforms"]`.

```cpp
if (!contains(policy, "\"macos-arm64\"") ||
    !contains(policy, "\"macos-x86_64\"") ||
    !contains(runner, "platform == \"macos-arm64\"") ||
    !contains(runner, "platform == \"macos-x86_64\"") ||
    !contains(validator, "report.get(\"platform\")") ||
    !contains(macos, "phase_j_performance_runner")) return 6;
```

Run the focused pair. Expected: `0/2`; behavior exits at the new platform
assertion and structure exits because the wrapper is missing.

- [x] **Step 2: Extend the runner and validator without duplicating metrics**

Change only the runner platform predicate:

```cpp
const bool valid_platform =
    platform == "portable" || platform == "windows" ||
    platform == "wayland" || platform == "x11" ||
    platform == "macos-arm64" || platform == "macos-x86_64";
if (iterations == 0 || iterations > 10000 || !valid_platform) return 3;
```

Add this validator check before metric checks:

```python
if report.get("platform") not in policy.get("platforms", []):
    issues.append("report platform is not active in policy")
```

Add the exact policy key while retaining all budgets:

```json
"platforms": ["windows", "wayland", "x11", "macos-arm64", "macos-x86_64"]
```

Replace the old exclusion note with
`"macOS arm64 and x86_64 retain separate reports at the same revision."`.

- [x] **Step 3: Create the thin native wrapper**

```bash
#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == "Darwin" ]] || { echo "macOS required" >&2; exit 2; }
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3
output_dir=${CGPUI_PERFORMANCE_OUTPUT_DIR:-artifacts/phase-j-performance}
iterations=${CGPUI_PERFORMANCE_ITERATIONS:-64}
[[ "/$output_dir/" != */../* ]] || exit 3
if [[ "$output_dir" == /* ]]; then output_path="$output_dir";
else output_path="$root/$output_dir"; fi
output_probe="$output_path"; output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 3
output_path="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_path" != "$root" && "$output_path/" == "$root/"* ]] || exit 3
mkdir -p "$output_path"
cd "$root"
xmake build -P "$root" -y -j 1 phase_j_performance_runner
xmake run -P "$root" phase_j_performance_runner --platform "macos-$arch" \
  --output "$output_path/macos-$arch.json" --iterations "$iterations"
python3 tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report "$output_path/macos-$arch.json"
```

- [x] **Step 4: Run GREEN and the staged guard**

```bash
bash -n scripts/ci/macos-performance-baseline.sh
xmake build -P . -y -j 1 phase_j_performance_baseline_audit_test
xmake build -P . -y -j 1 phase_j_performance_baseline_structure_test
xmake test -P . -y -j 1 -v phase_j_performance_baseline_audit_test/default \
  phase_j_performance_baseline_structure_test/default
xmake run -P . phase_j_macos_completion_test; test $? -eq 40
```

Expected: focused tests pass `2/2`; completion advances to stress exit `40`.

- [x] **Step 5: Commit the performance band**

```bash
git add scripts/ci/macos-performance-baseline.sh \
  tests/performance/phase_j_performance_runner.cpp \
  tools/gpui_parity/validate_performance_baseline.py \
  docs/gpui-performance-baseline-policy.json \
  tests/api_parity/phase_j_performance_baseline_audit_test.cpp \
  tests/architecture/phase_j_performance_baseline_structure_test.cpp
git commit -m "perf: add phase j macos baselines"
```

### Task 4: Add the Exact Eight-Scenario macOS Stress Matrix

**Files:**
- Create: `scripts/ci/macos-stress-matrix.sh`
- Modify: `tests/stress/phase_j_stress_runner.cpp`
- Modify: `tools/gpui_parity/validate_stress_report.py`
- Modify: `docs/gpui-stress-profile.json`
- Modify: `tests/api_parity/phase_j_stress_audit_test.cpp`
- Modify: `tests/architecture/phase_j_stress_structure_test.cpp`

- [x] **Step 1: Add RED assertions for two macOS reports and exactly eight rows**

Require both macOS platform names, the new wrapper, validator platform checking,
and `count(runner, "results.push_back(") == 8`. Expected RED is the missing
wrapper, platform identities, and active-platform validation; the runner's
existing eight-row inventory is frozen by the new assertion.

```cpp
if (!contains(profile, "\"macos-arm64\"") ||
    !contains(profile, "\"macos-x86_64\"") ||
    !contains(runner, "platform == \"macos-arm64\"") ||
    !contains(runner, "platform == \"macos-x86_64\"") ||
    count(runner, "results.push_back(") != 8 ||
    !contains(validator, "report.get(\"platform\")") ||
    !contains(macos, "phase_j_stress_runner")) return 6;
```

- [x] **Step 2: Preserve the runner inventory and extend the platform predicate**

Keep the existing one window-churn insertion and seven other scenario insertions:

```cpp
results.push_back(run_window_churn(64 * scale));
cgpui::TestApp app;
// Existing setup remains here.
results.push_back(run_many_entities(window, *view, 2000 * scale));
results.push_back(run_large_uniform_list(50000 * scale, 500 * scale));
results.push_back(run_ime_composition(window, *view, 1000 * scale));
results.push_back(run_clipboard_ownership(app, 1000 * scale));
results.push_back(run_drag_drop_cancel(window, *view, 1000 * scale));
results.push_back(run_asset_reload(2000 * scale));
results.push_back(run_task_cancellation(app, window, *view, 2000 * scale));
```

Extend the accepted platform predicate with `macos-arm64` and
`macos-x86_64`. Add this validator check before scenario checks:

```python
if report.get("platform") not in profile.get("platforms", []):
    issues.append("report platform is not active in profile")
```

Change the profile to:

```json
"platforms": ["windows", "wayland", "x11", "macos-arm64", "macos-x86_64"]
```

Remove `macos_execution: excluded_by_user`.

- [x] **Step 3: Create the native stress wrapper**

Begin with the Darwin, architecture, deployment-target, and Xmake checks shown
below, then resolve the output path with the full workspace policy before the
execution block:

```bash
#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == "Darwin" ]] || { echo "macOS required" >&2; exit 2; }
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3
output_dir=${CGPUI_STRESS_OUTPUT_DIR:-artifacts/phase-j-stress}
scale=${CGPUI_STRESS_SCALE:-1}
[[ "/$output_dir/" != */../* ]] || exit 3
if [[ "$output_dir" == /* ]]; then output_path="$output_dir";
else output_path="$root/$output_dir"; fi
output_probe="$output_path"; output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 3
output_path="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_path" != "$root" && "$output_path/" == "$root/"* ]] || exit 3
mkdir -p "$output_path"
cd "$root"
xmake build -P "$root" -y -j 1 phase_j_stress_runner
xmake run -P "$root" phase_j_stress_runner --platform "macos-$arch" \
  --output "$output_path/macos-$arch.json" --scale "$scale"
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report "$output_path/macos-$arch.json"
```

- [x] **Step 4: Run GREEN and advance the staged guard**

```bash
bash -n scripts/ci/macos-stress-matrix.sh
xmake build -P . -y -j 1 phase_j_stress_audit_test
xmake build -P . -y -j 1 phase_j_stress_structure_test
xmake test -P . -y -j 1 -v phase_j_stress_audit_test/default \
  phase_j_stress_structure_test/default
xmake run -P . phase_j_macos_completion_test; test $? -eq 50
```

Expected: focused `2/2`; completion advances to package/Release exit `50`.

- [x] **Step 5: Commit the stress band**

```bash
git add scripts/ci/macos-stress-matrix.sh tests/stress/phase_j_stress_runner.cpp \
  tools/gpui_parity/validate_stress_report.py docs/gpui-stress-profile.json \
  tests/api_parity/phase_j_stress_audit_test.cpp \
  tests/architecture/phase_j_stress_structure_test.cpp
git commit -m "test: add phase j macos stress matrix"
```

### Task 5: Stage Native Architecture-Specific macOS Packages

**Files:**
- Create: `scripts/ci/macos-package.sh`
- Modify: `tests/api_parity/phase_j_release_audit_test.cpp`
- Modify: `tests/architecture/phase_j_release_structure_test.cpp`

- [x] **Step 1: Make release guards RED for the macOS package contract**

Require `macos-package.sh`, seven exact static libraries, `hello_window`,
`deployment_target`, `metadata/build.json`, `metadata/xmake-requires.lock`, and
the two supported architectures. Expected: focused release behavior returns
the new package exit and structure reports the missing script.

- [x] **Step 2: Implement strict argument and output-root checks**

The script interface is `macos-package.sh <arm64|x86_64> <debug|release>
[output-root]`. Begin with:

```bash
#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == "Darwin" ]] || exit 2
arch=${1:-}; mode=${2:-}
[[ "$arch" =~ ^(arm64|x86_64)$ ]] || { echo "invalid architecture" >&2; exit 2; }
[[ "$mode" =~ ^(debug|release)$ ]] || { echo "invalid mode" >&2; exit 2; }
[[ "$(uname -m)" == "$arch" ]] || { echo "native architecture required" >&2; exit 2; }
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 2
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
output_input=${3:-build/phase-j-release/macos-$arch}
[[ "/$output_input/" != */../* ]] || exit 2
if [[ "$output_input" == /* ]]; then output_candidate="$output_input";
else output_candidate="$repo_root/$output_input"; fi
output_probe="$output_candidate"; output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 2
output_root="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_root" != "$repo_root" && "$output_root/" == "$repo_root/"* ]] || exit 2
```

Only after these checks may the script run `rm -rf -- "$output_root"` and create
the build/config/global/package/temp directories.

- [x] **Step 3: Configure, build, and stage the exact package payload**

Use repository-local config/global/package/temp roots and:

```bash
export XMAKE_CONFIGDIR="${XMAKE_CONFIGDIR:-$output_root/config}"
export XMAKE_GLOBALDIR="${XMAKE_GLOBALDIR:-$output_root/global}"
export XMAKE_PKG_CACHEDIR="${XMAKE_PKG_CACHEDIR:-$output_root/pkg-cache}"
export XMAKE_PKG_INSTALLDIR="${XMAKE_PKG_INSTALLDIR:-$output_root/pkg-install}"
export TMPDIR="${TMPDIR:-$output_root/tmp}"
xmake f -P "$repo_root" -y -c -m "$mode" -a "$arch" --ccache=n -o "$build_root"
xmake build -P "$repo_root" -y -j 1 cgpui_app
xmake build -P "$repo_root" -y -j 1 hello_window
binary_root="$build_root/macosx/$arch/$mode"
libraries=(
  libcgpui_core.a libcgpui_platform.a libcgpui_platform_macos.a
  libcgpui_renderer.a libcgpui_renderer_metal.a libcgpui_ui.a libcgpui_app.a
)
```

Install libraries at mode `0644`, `hello_window` at `0755`, all
`include/cgpui` headers, README, `xmake-requires.lock`, and a parsed JSON
`metadata/build.json` containing schema version, architecture, deployment target,
Xmake version first line, `sw_vers` product/build values, and Xcode version.

- [x] **Step 4: Write and immediately reparse the base manifest**

The manifest object is exact except for the requested architecture and mode:

```python
manifest = {
    "schema_version": 1,
    "package": f"cgpui-macos-{arch}-{mode}",
    "platform": "macos",
    "architecture": arch,
    "mode": mode,
    "deployment_target": "13.0",
    "header_root": "include/cgpui",
    "libraries": [f"lib/{name}" for name in libraries],
    "executables": ["bin/hello_window"],
    "documents": ["README.md"],
    "metadata": ["metadata/build.json", "metadata/xmake-requires.lock"],
}
```

Fail unless the reparsed object is identical, all required paths exist, the
staged public-header count equals the repository count, and every staged library
comes from `$binary_root`.

- [x] **Step 5: Run syntax and focused GREEN checks**

```bash
bash -n scripts/ci/macos-package.sh
xmake build -P . -y -j 1 phase_j_release_audit_test
xmake build -P . -y -j 1 phase_j_release_structure_test
xmake test -P . -y -j 1 -v phase_j_release_audit_test/default \
  phase_j_release_structure_test/default
```

Expected: release tests pass `2/2`; the comprehensive completion guard remains
at exit `50` because the Release wrapper and macOS policy entries are next.

- [x] **Step 6: Commit the package stage**

```bash
git add scripts/ci/macos-package.sh \
  tests/api_parity/phase_j_release_audit_test.cpp \
  tests/architecture/phase_j_release_structure_test.cpp
git commit -m "build: stage native macos packages"
```

### Task 6: Create and Verify Deterministic macOS Release Archives

**Files:**
- Create: `scripts/ci/macos-phase-j-release.sh`
- Modify: `scripts/ci/create-release-artifact.py`
- Modify: `docs/gpui-release-artifact-policy.json`
- Modify: `tests/api_parity/phase_j_release_audit_test.cpp`
- Modify: `tests/architecture/phase_j_release_structure_test.cpp`

- [x] **Step 1: Run a RED packer probe using a temporary package tree**

```bash
probe=$(mktemp -d)
PROBE="$probe" python3 - <<'PY'
import json, os
from pathlib import Path
root = Path(os.environ["PROBE"])
(root / "README.md").write_text("probe\n", encoding="utf-8")
(root / "manifest.json").write_text(json.dumps({
    "schema_version": 1,
    "package": "cgpui-macos-arm64-release",
    "platform": "macos",
    "architecture": "arm64",
    "mode": "release",
    "deployment_target": "13.0"
}), encoding="utf-8")
PY
python3 scripts/ci/create-release-artifact.py --platform macos \
  --package-root "$probe" --package-name cgpui-macos-arm64-release \
  --archive-name cgpui-macos-arm64-release.tar.gz --output "$probe/a.tar.gz"
```

Expected: argparse rejects `macos` as an invalid platform before the packer
change. Remove only the printed temporary probe directory after inspection.

- [x] **Step 2: Extend the structured archive writer**

Change platform choices to `("windows", "linux", "macos")`. After reading the
base manifest, enforce:

```python
architecture = base_manifest.get("architecture")
if not isinstance(architecture, str) or not architecture:
    raise SystemExit("package manifest architecture is missing")
if platform == "macos":
    if architecture not in {"arm64", "x86_64"}:
        raise SystemExit("unsupported macOS package architecture")
    if base_manifest.get("deployment_target") != "13.0":
        raise SystemExit("macOS package deployment target must be 13.0")
```

Add `architecture` to every content manifest and add
`deployment_target: "13.0"` only for macOS. Keep Windows ZIP behavior and route
Linux/macOS through the existing deterministic USTAR plus gzip writer.

- [x] **Step 3: Create the one-architecture Release wrapper**

Use interface `macos-phase-j-release.sh <arm64|x86_64> [output-root]`, repeat
the native host, parent traversal, workspace, deployment target, and Xmake
checks with this entry block, then run the package/archive block:

```bash
#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == "Darwin" ]] || exit 2
script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)
repo_root=$(cd "$script_dir/../.." && pwd -P)
arch=${1:-}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 2
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 2
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3
output_input=${2:-build/phase-j-release/macos-$arch}
[[ "/$output_input/" != */../* ]] || exit 2
if [[ "$output_input" == /* ]]; then output_candidate="$output_input";
else output_candidate="$repo_root/$output_input"; fi
output_probe="$output_candidate"; output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 2
output_root="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_root" != "$repo_root" && "$output_root/" == "$repo_root/"* ]] || exit 2

"$script_dir/macos-package.sh" "$arch" release "$output_root"
package_name="cgpui-macos-$arch-release"
archive_name="$package_name.tar.gz"
python3 "$script_dir/create-release-artifact.py" --platform macos \
  --package-root "$package_root" --package-name "$package_name" \
  --archive-name "$archive_name" --output "$artifact_root/$archive_name"
python3 "$script_dir/create-release-artifact.py" --platform macos \
  --package-root "$package_root" --package-name "$package_name" \
  --archive-name "$archive_name" --output "$artifact_root/$package_name.verify.tar.gz"
cmp -s "$artifact_root/$archive_name" "$artifact_root/$package_name.verify.tar.gz" || exit 4
```

Use Python `tarfile` and `json` to require the package prefix, exact entry set,
every manifest file size/SHA-256/mode, `platform=macos`, exact architecture,
`mode=release`, and `deployment_target=13.0`. Remove only the verification
archive after it passes and copy `release-manifest.json` beside the artifact.

- [x] **Step 4: Extend release policy with exact artifact names**

```json
"archives": {
  "windows": "cgpui-windows-release.zip",
  "linux": "cgpui-linux-release.tar.gz",
  "macos_arm64": "cgpui-macos-arm64-release.tar.gz",
  "macos_x86_64": "cgpui-macos-x86_64-release.tar.gz"
},
"macos_deployment_target": "13.0"
```

- [x] **Step 5: Run GREEN and advance the staged guard**

```bash
python3 -m py_compile scripts/ci/create-release-artifact.py
bash -n scripts/ci/macos-phase-j-release.sh
xmake build -P . -y -j 1 phase_j_release_audit_test
xmake build -P . -y -j 1 phase_j_release_structure_test
xmake test -P . -y -j 1 -v phase_j_release_audit_test/default \
  phase_j_release_structure_test/default
xmake run -P . phase_j_macos_completion_test; test $? -eq 60
```

Expected: release pair passes `2/2`; completion advances to CI exit `60`.

- [x] **Step 6: Commit deterministic Release support**

```bash
git add scripts/ci/macos-phase-j-release.sh scripts/ci/create-release-artifact.py \
  docs/gpui-release-artifact-policy.json \
  tests/api_parity/phase_j_release_audit_test.cpp \
  tests/architecture/phase_j_release_structure_test.cpp
git commit -m "build: add deterministic macos releases"
```

### Task 7: Add Darwin Dependency Setup and Dual-Architecture Desktop CI

**Files:**
- Create: `scripts/ci/macos-architecture-header.sh`
- Modify: `scripts/ci/macos-debug.sh`
- Modify: `xmake-requires.lock`
- Modify: `.github/actions/setup-phase-j-dependencies/action.yml`
- Rename: `.github/workflows/phase-j-windows-linux.yml` to `.github/workflows/phase-j-desktop.yml`
- Modify: `.github/workflows/phase-j-desktop.yml`
- Modify: `tests/api_parity/phase_j_release_audit_test.cpp`
- Modify: `tests/architecture/phase_j_release_structure_test.cpp`

- [x] **Step 1: Make structure and release guards RED for the renamed workflow**

Change every current workflow reference to `phase-j-desktop.yml`; require the
old path to be absent, require four macOS job names, both pinned runners,
architecture checks, deployment target, and architecture-specific uploads.
Expected: RED because the workflow has not yet been renamed.

- [x] **Step 2: Add Darwin repository-local dependency roots**

`macos-dependencies.sh` already exposes the complete pinned-root function used
by local commands. The dependency action invokes that function rather than
duplicating root and repository ownership. Its fixed bootstrap remains:

```bash
repository="$XMAKE_GLOBALDIR/.xmake/repositories/xmake-repo"
repository_commit=b9256335e0b6e70808e23dfe71627d8a4dcc0abf
mkdir -p "$XMAKE_CONFIGDIR" "$XMAKE_GLOBALDIR/.xmake/repositories" \
  "$XMAKE_PKG_CACHEDIR" "$XMAKE_PKG_INSTALLDIR" "$TMPDIR"
if [[ ! -d "$repository/.git" ]]; then
  git clone --no-checkout https://github.com/xmake-io/xmake-repo.git "$repository"
fi
git -C "$repository" fetch --depth 1 origin "$repository_commit"
git -C "$repository" checkout --detach "$repository_commit"
touch "$repository/updated"
locked_repository="$XMAKE_CONFIGDIR/repositories/efa340bf3b6f6de54c5f0ab8c98fba7d.lock"
if [[ ! -d "$locked_repository/.git" ]]; then
  mkdir -p "$(dirname "$locked_repository")"
  git clone --shared "$repository" "$locked_repository"
fi
git -C "$locked_repository" fetch "$repository" "$repository_commit"
git -C "$locked_repository" checkout --detach "$repository_commit"
```

When `--github-env` is present, append all five exported variables to
`$GITHUB_ENV` and fail if that variable is empty.

Add a Darwin composite step that invokes the helper with `$RUNNER_ARCH` in the
root name:

```yaml
- name: Configure macOS dependency roots
  if: runner.os == 'macOS'
  shell: bash
  run: |
    root="$GITHUB_WORKSPACE/build/phase-j-ci/dependencies/macos-$RUNNER_ARCH"
    bash ./scripts/ci/macos-dependencies.sh "$root" --github-env
```

Keep the Xmake version and cache key inputs unchanged.

- [x] **Step 3: Lock the x86_64 macOS package graph**

Add a `['macosx|x86_64']` section to `xmake-requires.lock` containing the same
four package identities and versions as the existing arm64 section: CMake
`4.3.4`, LunaSVG `v3.5.0`, Ninja `v1.13.2`, and PlutoVG `v1.3.3`. Each entry
uses repository URL `https://github.com/xmake-io/xmake-repo.git`, branch
`master`, and commit `82466fe9815001a215d26b7c119bb223d757949d`.

Run:

```bash
rg -n '\["macosx\|(arm64|x86_64)"\]' xmake-requires.lock
rg -n -A 48 '\["macosx\|arm64"\]|\["macosx\|x86_64"\]' \
  xmake-requires.lock | rg 'macosx|cmake|lunasvg|ninja|plutovg|82466fe'
```

Expected: both architecture sections exist and each reports all four packages
plus the locked repository commit.

- [x] **Step 4: Make macOS Debug consume configured roots**

Change each unconditional root export to a defaulting export, for example:

```bash
export XMAKE_CONFIGDIR="${XMAKE_CONFIGDIR:-$output_root/config}"
export XMAKE_GLOBALDIR="${XMAKE_GLOBALDIR:-$output_root/global}"
export XMAKE_PKG_CACHEDIR="${XMAKE_PKG_CACHEDIR:-$output_root/pkg-cache}"
export XMAKE_PKG_INSTALLDIR="${XMAKE_PKG_INSTALLDIR:-$output_root/pkg-install}"
export TMPDIR="${TMPDIR:-$output_root/tmp}"
```

At entry, require `CGPUI_EXPECTED_ARCH` when provided and require/export
`MACOSX_DEPLOYMENT_TARGET=13.0`. Source `macos-dependencies.sh` and call its
function only when one of the five Xmake roots is absent; remove the duplicated
repository bootstrap from `macos-debug.sh`. Keep the Phase H test command and
historical output text intact.

- [x] **Step 5: Create the macOS architecture/header runner**

Parse the shared `target|source` inventory with strict Darwin, architecture,
deployment-target, and Xmake checks. The complete control flow is:

```bash
#!/usr/bin/env bash
set -euo pipefail
[[ "$(uname -s)" == "Darwin" ]] || exit 2
repo_root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)
arch=${CGPUI_EXPECTED_ARCH:-$(uname -m)}
[[ "$arch" =~ ^(arm64|x86_64)$ && "$(uname -m)" == "$arch" ]] || exit 3
[[ "${MACOSX_DEPLOYMENT_TARGET:-}" == "13.0" ]] || exit 3
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]] || exit 3
matrix="$repo_root/scripts/ci/architecture-header-targets.txt"
targets=()
while IFS='|' read -r target source extra; do
  [[ -z "$target" || "$target" == \#* ]] && continue
  [[ -n "$source" && -z "$extra" && "$target" =~ ^[a-z0-9_]+$ ]] || exit 4
  [[ -f "$repo_root/$source" ]] || exit 4
  targets+=("$target")
done < "$matrix"
[[ ${#targets[@]} -gt 0 ]] || exit 4
cd "$repo_root"
for target in "${targets[@]}"; do
  xmake build -P "$repo_root" -y -j 1 "$target"
done
filters=()
for target in "${targets[@]}"; do filters+=("$target/default"); done
xmake test -P "$repo_root" -y -j 1 -v "${filters[@]}"
```

- [x] **Step 6: Rename and extend the workflow with four matrices**

Use this exact matrix in each macOS job:

```yaml
strategy:
  fail-fast: false
  matrix:
    include:
      - runner: macos-15
        arch: arm64
      - runner: macos-15-intel
        arch: x86_64
runs-on: ${{ matrix.runner }}
env:
  CGPUI_EXPECTED_ARCH: ${{ matrix.arch }}
  MACOSX_DEPLOYMENT_TARGET: "13.0"
```

Add jobs named `macos-debug-examples`, `macos-performance-stress`,
`macos-release`, and `macos-architecture-header`. Each checks
`test "$(uname -m)" = "$CGPUI_EXPECTED_ARCH"` before invoking scripts.
Upload:

```yaml
- uses: actions/upload-artifact@v4
  with:
    name: phase-j-macos-${{ matrix.arch }}-reports
    path: |
      artifacts/phase-j-performance/macos-${{ matrix.arch }}.json
      artifacts/phase-j-stress/macos-${{ matrix.arch }}.json
    if-no-files-found: error
```

The release job uploads `cgpui-macos-${{ matrix.arch }}-release` from
`build/phase-j-release/macos-${{ matrix.arch }}/artifacts`. Preserve all
existing Windows/Linux jobs byte-for-byte except workflow display name and
references required by the rename.

- [x] **Step 7: Run local static CI verification**

```bash
bash -n scripts/ci/macos-debug.sh scripts/ci/macos-dependencies.sh \
  scripts/ci/macos-architecture-header.sh
ruby -e 'require "yaml"; YAML.load_file(".github/workflows/phase-j-desktop.yml"); YAML.load_file(".github/actions/setup-phase-j-dependencies/action.yml")'
test ! -e .github/workflows/phase-j-windows-linux.yml
xmake build -P . -y -j 1 phase_j_release_audit_test
xmake build -P . -y -j 1 phase_j_release_structure_test
xmake test -P . -y -j 1 -v phase_j_release_audit_test/default \
  phase_j_release_structure_test/default
xmake run -P . phase_j_macos_completion_test; test $? -eq 70
xmake run -P . phase_j_macos_completion_structure_test
```

Expected: YAML parses; release pair and structure guard pass; behavior advances
to authority exit `70`.

- [x] **Step 8: Commit desktop CI**

```bash
git add scripts/ci/macos-debug.sh scripts/ci/macos-architecture-header.sh \
  xmake-requires.lock \
  .github/actions/setup-phase-j-dependencies/action.yml \
  .github/workflows/phase-j-windows-linux.yml \
  .github/workflows/phase-j-desktop.yml \
  tests/api_parity/phase_j_release_audit_test.cpp \
  tests/architecture/phase_j_release_structure_test.cpp
git commit -m "ci: add phase j macos matrices"
```

### Task 8: Record Pending macOS Authority Without Claiming Completion

**Files:**
- Modify: `README.md`
- Modify: `docs/platform-dependencies.md`
- Modify: `docs/examples.md`
- Modify: `docs/architecture.md`
- Modify: `docs/gpui-complete-parity-ledger.json`
- Modify: `docs/gpui-complete-parity-ledger.md`
- Modify: `docs/gpui-phase-j-final-verification.json`
- Modify: `docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`
- Modify: `tests/api_parity/phase_j_final_closeout_test.cpp`

- [x] **Step 1: Write RED closeout assertions for pending dual-architecture work**

Keep the historical Phase H evidence assertion. Replace current exclusion
assertions with exact pending fields:

```cpp
if (!contains(ledger, "\"phase_j_status\": \"complete_non_macos\"") ||
    !contains(ledger, "\"phase_j_macos_completion_status\": "
                      "\"pending_native_dual_arch_verification\"") ||
    !contains(ledger, "\"phase_j_required_non_macos_gaps\": 0") ||
    !contains(verification, "\"status\": \"pending\"") ||
    !contains(verification, "\"arm64\"") ||
    !contains(verification, "\"x86_64\"")) return 30;
```

Expected: final closeout behavior is RED before authority files change.

- [x] **Step 2: Add explicit pending fields while preserving every old result**

Keep `phase_j_status`, Steps 829-834/835-840 status, and all historical hashes
and counts unchanged. Add:

```json
"phase_j_macos_completion_status": "pending_native_dual_arch_verification",
"phase_j_required_macos_gaps": 2,
"phase_j_macos_architectures": ["arm64", "x86_64"],
"phase_j_macos_deployment_target": "13.0"
```

Change `docs/gpui-phase-j-final-verification.json` to overall `status: pending`
and add:

```json
"macos": {
  "deployment_target": "13.0",
  "arm64": {"runner": "macos-15", "status": "pending"},
  "x86_64": {"runner": "macos-15-intel", "status": "pending"}
}
```

Retain the complete original Windows, WSLg, X11, Windows Release, Linux Release,
performance, stress, and `preserved_macos_evidence` objects.

- [x] **Step 3: Reopen Steps 805-840 in place in user-facing authority**

Roadmap checkboxes remain checked for the historical non-macOS portions, and a
new unchecked nested line under each affected band states its exact macOS gate.
Use these exact nested lines:

```markdown
  - [ ] macOS arm64/x86_64: compile and smoke all 20 official equivalents.
  - [ ] macOS arm64/x86_64: retain and validate all eight performance reports.
  - [ ] macOS arm64/x86_64: complete all eight stress scenarios.
  - [ ] macOS arm64/x86_64: reproduce and verify both Release tar.gz artifacts.
  - [ ] macOS arm64/x86_64: pass Debug, examples, performance, stress, Release,
    architecture/header CI and close the all-desktop ledger.
```

The immediate next slice becomes “Phase J macOS arm64 local verification, then
native arm64/x86_64 CI”; Steps 841+ remain reserved and unchanged. Remove all
current “macOS excluded” prose from README/current guides and replace it with
the pending dual-architecture contract. Historical dated prose remains intact.

- [x] **Step 4: Run pending-state GREEN checks**

```bash
xmake build -P . -y -j 1 phase_j_final_closeout_test
xmake build -P . -y -j 1 phase_j_macos_completion_test
xmake test -P . -y -j 1 -v phase_j_final_closeout_test/default \
  phase_j_macos_completion_test/default
python3 -m json.tool docs/gpui-complete-parity-ledger.json >/dev/null
python3 -m json.tool docs/gpui-phase-j-final-verification.json >/dev/null
git diff --check
```

Expected: `2/2`; completion guard accepts the explicit pending state, while no
document claims Phase J complete for all desktop platforms.

- [x] **Step 5: Commit pending authority**

```bash
git add README.md docs task_plan.md findings.md progress.md \
  tests/api_parity/phase_j_final_closeout_test.cpp
git commit -m "docs: reopen phase j macos verification"
```

### Task 9: Run and Record the Complete Local arm64 Matrix

**Files:**
- Modify: `docs/gpui-complete-parity-ledger.json`
- Modify: `docs/gpui-phase-j-final-verification.json`
- Modify: `scripts/ci/macos-debug.sh`
- Modify: `tests/architecture/phase_g_action_macro_structure_test.cpp`
- Modify: `tests/architecture/phase_g_architecture_header_matrix_structure_test.cpp`
- Modify: `tests/architecture/phase_g_linux_debug_packaging_structure_test.cpp`
- Modify: `tests/architecture/phase_g_release_packaging_structure_test.cpp`
- Modify: `tests/architecture/phase_g_windows_debug_packaging_structure_test.cpp`
- Modify: `tests/architecture/phase_h_macos_example_smoke_structure_test.cpp`
- Modify: `tests/architecture/phase_h_macos_full_debug_verification_structure_test.cpp`
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`

- [x] **Step 1: Confirm the native environment before any build**

```bash
test "$(uname -s)" = Darwin
test "$(uname -m)" = arm64
[[ "$(xmake --version | sed -n '1p')" == *"xmake v3.0.9"* ]]
export CGPUI_EXPECTED_ARCH=arm64
export MACOSX_DEPLOYMENT_TARGET=13.0
source scripts/ci/macos-dependencies.sh
cgpui_configure_macos_dependency_environment \
  build/phase-j-local/dependencies/macos-arm64
```

Expected: all checks exit `0`. If the Xmake suffix changes but remains the
3.0.9 line, record the observed line in evidence and continue only after the
scripts' locked-version checks pass.

- [x] **Step 2: Run the full Debug suite and official examples**

```bash
bash scripts/ci/macos-debug.sh build/phase-j-local/macos-arm64-debug \
  2>&1 | tee build/phase-j-local/macos-arm64-debug.log
bash scripts/ci/macos-example-smoke.sh \
  2>&1 | tee build/phase-j-local/macos-arm64-examples.log
```

Expected: complete Xmake Debug summary has zero failures; 21 canonical example
targets, native smoke/pixel/input groups, and four `hello_window` modes pass.

- [x] **Step 3: Run performance, stress, Release, and structure matrices**

```bash
CGPUI_PERFORMANCE_OUTPUT_DIR=build/phase-j-local/reports/performance \
  bash scripts/ci/macos-performance-baseline.sh
CGPUI_STRESS_OUTPUT_DIR=build/phase-j-local/reports/stress \
  bash scripts/ci/macos-stress-matrix.sh
bash scripts/ci/macos-phase-j-release.sh arm64 \
  build/phase-j-local/macos-arm64-release
bash scripts/ci/macos-architecture-header.sh \
  2>&1 | tee build/phase-j-local/macos-arm64-architecture.log
```

Expected: eight performance metrics pass policy; exactly eight stress scenarios
pass profile; the arm64 archive reproduces byte-for-byte and every archived hash
matches; every architecture/header target passes.

- [x] **Step 4: Audit generated evidence with structured readers**

```bash
python3 tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report build/phase-j-local/reports/performance/macos-arm64.json
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report build/phase-j-local/reports/stress/macos-arm64.json
shasum -a 256 build/phase-j-local/macos-arm64-release/artifacts/cgpui-macos-arm64-release.tar.gz
python3 -m json.tool build/phase-j-local/macos-arm64-release/artifacts/release-manifest.json >/dev/null
```

Expected: both validators exit `0`; one exact archive SHA-256 is printed; JSON
parses; performance and stress reports remain in separate subdirectories.

- [x] **Step 5: Record provisional arm64 evidence without closing Phase J**

Set `macos.arm64.status` to `passed_local`, record the executed commands, Debug
test count/duration from the summary, archive name/SHA-256/manifest file count,
eight performance metrics, and eight stress scenarios. Keep x86_64 `pending`,
overall status `pending`, `phase_j_status=complete_non_macos`, and required macOS
gaps at `1`.

- [x] **Step 6: Re-run focused guards and commit local evidence**

```bash
xmake test -P . -y -j 1 -v phase_j_macos_completion_test/default \
  phase_j_macos_completion_structure_test/default \
  phase_j_final_closeout_test/default \
  phase_j_final_closeout_structure_test/default
git diff --check
git add docs/gpui-complete-parity-ledger.json \
  docs/gpui-phase-j-final-verification.json \
  docs/superpowers/plans/2026-07-18-phase-j-macos-completion.md \
  scripts/ci/macos-debug.sh \
  tests/architecture/phase_g_action_macro_structure_test.cpp \
  tests/architecture/phase_g_architecture_header_matrix_structure_test.cpp \
  tests/architecture/phase_g_linux_debug_packaging_structure_test.cpp \
  tests/architecture/phase_g_release_packaging_structure_test.cpp \
  tests/architecture/phase_g_windows_debug_packaging_structure_test.cpp \
  tests/architecture/phase_h_macos_example_smoke_structure_test.cpp \
  tests/architecture/phase_h_macos_full_debug_verification_structure_test.cpp \
  task_plan.md findings.md progress.md
git commit -m "test: record local phase j macos arm64"
```

Expected: `4/4`; no authority field says complete all-desktop.

### Task 10: Prove Both Native Architectures, Close Phase J, and Merge

**Files:**
- Modify: `.github/workflows/phase-j-desktop.yml`
- Create: `scripts/ci/run-with-github-annotation.sh`
- Create: `tests/ci/run_with_github_annotation_test.sh`
- Modify: `tests/architecture/phase_j_macos_completion_structure_test.cpp`
- Modify: `docs/gpui-phase-j-final-verification.json`
- Modify: `docs/gpui-complete-parity-ledger.json`
- Modify: `docs/gpui-complete-parity-ledger.md`
- Modify: `docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md`
- Modify: `README.md`
- Modify: `docs/platform-dependencies.md`
- Modify: `docs/examples.md`
- Modify: `docs/architecture.md`
- Modify: `task_plan.md`
- Modify: `findings.md`
- Modify: `progress.md`
- Modify: `tests/api_parity/phase_j_final_closeout_test.cpp`

- [ ] **Step 1: Push the feature branch and identify its desktop workflow run**

```bash
git status --short
git push -u origin codex/phase-j-macos-completion
gh run list --workflow phase-j-desktop.yml \
  --branch codex/phase-j-macos-completion --limit 5
```

Expected: clean worktree before push; branch push succeeds; the newest run's
head SHA equals local `git rev-parse HEAD`.

- [ ] **Step 2: Wait for every native job instead of merging early**

```bash
run_id=$(gh run list --workflow phase-j-desktop.yml \
  --branch codex/phase-j-macos-completion --limit 1 \
  --json databaseId --jq '.[0].databaseId')
gh run watch "$run_id" --exit-status
gh run view "$run_id" --json conclusion,headSha,url,jobs \
  > build/phase-j-ci-run.json
jq -e '.conclusion == "success"' build/phase-j-ci-run.json
jq -e '[.jobs[] | select(.name | test("macos.*(arm64|x86_64)")) | .conclusion] |
       length == 8 and all(. == "success")' build/phase-j-ci-run.json
```

Expected: workflow conclusion `success`; four macOS job groups times two native
architectures produce eight successful job conclusions. On failure, diagnose
the failing job, add a focused regression, push a normal follow-up commit, and
repeat this step with the new run; never force-push.

- [ ] **Step 3: Download and independently audit both architectures**

```bash
rm -rf build/phase-j-ci-download
mkdir -p build/phase-j-ci-download
gh run download "$run_id" --dir build/phase-j-ci-download
find build/phase-j-ci-download -type f -print | sort
python3 tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report build/phase-j-ci-download/phase-j-macos-arm64-reports/phase-j-performance/macos-arm64.json
python3 tools/gpui_parity/validate_performance_baseline.py \
  --policy docs/gpui-performance-baseline-policy.json \
  --report build/phase-j-ci-download/phase-j-macos-x86_64-reports/phase-j-performance/macos-x86_64.json
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report build/phase-j-ci-download/phase-j-macos-arm64-reports/phase-j-stress/macos-arm64.json
python3 tools/gpui_parity/validate_stress_report.py \
  --profile docs/gpui-stress-profile.json \
  --report build/phase-j-ci-download/phase-j-macos-x86_64-reports/phase-j-stress/macos-x86_64.json
shasum -a 256 \
  build/phase-j-ci-download/cgpui-macos-arm64-release/cgpui-macos-arm64-release.tar.gz \
  build/phase-j-ci-download/cgpui-macos-x86_64-release/cgpui-macos-x86_64-release.tar.gz
python3 -m json.tool \
  build/phase-j-ci-download/cgpui-macos-arm64-release/release-manifest.json >/dev/null
python3 -m json.tool \
  build/phase-j-ci-download/cgpui-macos-x86_64-release/release-manifest.json >/dev/null
```

Expected: all four reports pass, each stress report has exactly eight scenario
rows, and each Release artifact directory contains its named archive plus
`manifest.json`/`release-manifest.json` evidence.

- [ ] **Step 4: Change final authority only from observed successful evidence**

Use the exact run URL, head SHA, job names, report contents, archive SHA-256
values, and manifest counts printed by Step 3. Make these semantic changes:

```json
"phase_j_status": "complete",
"phase_j_steps_829_834_status": "complete",
"phase_j_steps_835_840_status": "complete",
"phase_j_required_gaps": 0,
"phase_j_macos_completion_status": "complete",
"phase_j_required_macos_gaps": 0
```

Set final verification `scope` to `all_desktop` and `status` to `passed`; set
both architecture records to `passed_native` with runner, deployment target,
workflow run URL/head SHA, Debug/examples, performance, stress, Release archive,
SHA-256, reproducibility, and manifest counts. Preserve every pre-existing
Windows/WSL/X11/Phase H value byte-for-byte.

- [ ] **Step 5: Tighten the final closeout guard to require complete authority**

Replace pending acceptance with:

```cpp
if (!contains(ledger, "\"phase_j_status\": \"complete\"") ||
    !contains(ledger, "\"phase_j_steps_829_834_status\": \"complete\"") ||
    !contains(ledger, "\"phase_j_steps_835_840_status\": \"complete\"") ||
    !contains(ledger, "\"phase_j_required_gaps\": 0") ||
    !contains(verification, "\"scope\": \"all_desktop\"") ||
    !contains(verification, "\"status\": \"passed\"") ||
    !contains(verification, "\"status\": \"passed_native\"")) return 30;
```

Keep the exact Phase H 380/380 assertion and the Windows/Linux evidence checks.

- [ ] **Step 6: Run the final local closeout and static audits**

```bash
xmake build -P . -y -j 1 phase_j_macos_completion_test
xmake build -P . -y -j 1 phase_j_macos_completion_structure_test
xmake build -P . -y -j 1 phase_j_final_closeout_test
xmake build -P . -y -j 1 phase_j_final_closeout_structure_test
xmake test -P . -y -j 1 -v phase_j_macos_completion_test/default \
  phase_j_macos_completion_structure_test/default \
  phase_j_final_closeout_test/default \
  phase_j_final_closeout_structure_test/default
python3 -m json.tool docs/gpui-complete-parity-ledger.json >/dev/null
python3 -m json.tool docs/gpui-phase-j-final-verification.json >/dev/null
python3 -m json.tool docs/gpui-official-example-matrix.json >/dev/null
python3 -m json.tool docs/gpui-phase-j-example-audit.json >/dev/null
python3 -m json.tool docs/gpui-performance-baseline-policy.json >/dev/null
python3 -m json.tool docs/gpui-stress-profile.json >/dev/null
python3 -m json.tool docs/gpui-release-artifact-policy.json >/dev/null
rg -n 'excluded_by_user|complete_non_macos|current Phase J task excludes' \
  README.md docs task_plan.md
git diff --check
```

Expected: focused `4/4`; every JSON parses; the `rg` output contains only
explicitly labeled historical non-macOS records, never current authority; diff
check is clean.

- [ ] **Step 7: Commit closeout, push it, and require the closeout run**

```bash
git add README.md docs task_plan.md findings.md progress.md \
  tests/api_parity/phase_j_final_closeout_test.cpp
git commit -m "test: close phase j macos completion"
git push origin codex/phase-j-macos-completion
closeout_run=$(gh run list --workflow phase-j-desktop.yml \
  --branch codex/phase-j-macos-completion --limit 1 \
  --json databaseId --jq '.[0].databaseId')
gh run watch "$closeout_run" --exit-status
```

Expected: the new head SHA receives another fully successful desktop workflow;
both native macOS matrices now test the checked-in `complete` authority.

- [ ] **Step 8: Merge to the remote default branch without force operations**

```bash
git fetch origin
git switch master
git pull --ff-only origin master
git merge --ff-only codex/phase-j-macos-completion
git push origin master
git status --short --branch
```

Expected: fast-forward merge and push succeed; local `master` equals
`origin/master`; worktree is clean. If upstream `master` advanced and the
fast-forward merge is impossible, stop and rebase/merge only after inspecting
the new commits and rerunning the complete closeout workflow.

## Self-Review Checklist

- [x] Every approved design goal maps to Tasks 2-10.
- [x] Both `arm64` and `x86_64` receive native Debug, examples, performance,
  stress, Release, and architecture/header jobs.
- [x] macOS 13.0 is checked by scripts, manifests, policy, CI, and final evidence.
- [x] Release artifacts are separate tar.gz files and Universal 2 is absent.
- [x] Package and archive output roots reject traversal, repository root, and
  paths outside the repository.
- [x] All eight stress scenarios appear exactly once.
- [x] Historical Phase H 380/380, Windows, Wayland, X11, and existing Release
  hashes/counts remain unchanged.
- [x] Phase J cannot become `complete` before successful native CI and artifact
  download audits for both architectures.
- [x] The workflow is pushed on the feature branch, rerun after final authority,
  and only then fast-forwarded to `master`.
- [x] No task changes the upstream revision, Steps 841+ policy, signing,
  notarization, DMG, Universal 2, iOS, visionOS, or Catalyst scope.
