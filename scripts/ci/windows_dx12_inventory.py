"""Derive exact Windows suite inventories from the current owning manifests."""
from pathlib import Path
import hashlib
import re
import tomllib


WINDOWS_CFG = {
    'cfg(os = "windows")': True,
    'cfg(any(os = "windows", os = "linux"))': True,
    'cfg(os = "linux")': False,
    'cfg(os = "macos")': False,
    'cfg(not(os = "windows"))': False,
}

# This narrow hosted lane deliberately selects these shared native scenes. It is
# not the full suite inventory: the complete suites below come from mcpp.toml.
COMMON = (
    "renderer_pixel_backend_identity_test",
    "renderer_backdrop_blur_pixel_test",
    "renderer_box_shadow_pixel_test",
    "renderer_clip_pixel_test",
    "renderer_color_encoding_pixel_test",
    "renderer_frame_pixel_capture_test",
    "renderer_image_pixel_test",
    "renderer_opacity_pixel_test",
    "renderer_resize_pixel_test",
    "renderer_rounded_rect_pixel_test",
    "renderer_scale_pixel_test",
    "renderer_text_pixel_test",
    "renderer_text_ink_extent_pixel_test",
    "renderer_transform_pixel_test",
)


def unique_names(names, label):
    if (not isinstance(names, list) or not names or
            any(not isinstance(name, str) or not re.fullmatch(r"[a-z0-9_]+", name) for name in names)):
        raise ValueError(f"{label}: expected a nonempty explicit test-name list")
    if len(names) != len(set(names)):
        raise ValueError(f"{label}: duplicate test names")
    return names


def windows_tests(root, suite):
    if suite not in {"renderer", "renderer-dx12"}:
        raise ValueError("unsupported suite")
    directory = root / "tests" / suite
    manifest = tomllib.loads((directory / "mcpp.toml").read_text(encoding="utf-8"))
    sources = list(manifest.get("test", {}).get("discover", []))
    for expression, table in manifest.get("target", {}).items():
        if "test" not in table:
            continue
        if expression not in WINDOWS_CFG:
            raise ValueError(f"review Windows applicability of new test condition: {expression}")
        if WINDOWS_CFG[expression]:
            sources.extend(table["test"].get("discover", []))
    names = []
    for source in sources:
        # Current Windows suites use explicit files. New globs need policy review.
        if not isinstance(source, str) or not re.fullmatch(r"[a-z0-9_]+\.cpp", source):
            raise ValueError(f"{suite}: unsupported/nonlocal test entry: {source!r}")
        if not (directory / source).is_file():
            raise ValueError(f"{suite}: test source missing: {source}")
        names.append(Path(source).stem)
    return sorted(unique_names(names, suite))


def load_inventory(root):
    suites = {suite: windows_tests(root, suite) for suite in ("renderer", "renderer-dx12")}
    common = unique_names(list(COMMON), "common")
    if common[0] != "renderer_pixel_backend_identity_test" or not set(common) <= set(suites["renderer"]):
        raise ValueError("hosted common selection is absent from the owning Windows suite")
    manifests = {}
    for suite in suites:
        relative = f"tests/{suite}/mcpp.toml"
        manifests[relative] = hashlib.sha256((root / relative).read_bytes()).hexdigest()
    # No historical total, static full-name snapshot, or positive-count shortcut.
    return {"version": 2, "common": common, "suites": suites, "manifest_sha256": manifests}
