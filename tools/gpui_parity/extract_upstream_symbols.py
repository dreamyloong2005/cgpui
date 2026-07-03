#!/usr/bin/env python3
"""Extract a deterministic GPUI upstream parity snapshot.

This Phase A helper is deliberately conservative. It accepts local upstream
source files when available and otherwise emits the pinned source URLs that a
future richer extractor can fetch before parsing.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
from urllib.request import Request, urlopen


GPUI_UPSTREAM_REVISION = "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0"
GPUI_REPOSITORY = "https://github.com/zed-industries/zed"
RAW_ROOT = (
    "https://raw.githubusercontent.com/zed-industries/zed/"
    f"{GPUI_UPSTREAM_REVISION}/crates/gpui"
)


def read_optional(path: Path | None) -> str:
    if path is None or not path.exists():
        return ""
    return path.read_text(encoding="utf-8")


def read_url(url: str) -> str:
    request = Request(url, headers={"User-Agent": "cgpui-gpui-parity"})
    with urlopen(request, timeout=30) as response:
        return response.read().decode("utf-8")


def read_url_or_empty(url: str) -> str:
    try:
        return read_url(url)
    except Exception:
        return ""


def extract_public_reexports(gpui_rs: str) -> list[str]:
    exports: list[str] = []
    for line in gpui_rs.splitlines():
        stripped = line.strip()
        if stripped.startswith("pub use ") or stripped.startswith("pub mod "):
            exports.append(stripped.rstrip(";"))
    return exports


def extract_examples(cargo_toml: str) -> list[str]:
    examples: list[str] = []
    in_example = False
    for line in cargo_toml.splitlines():
      stripped = line.strip()
      if stripped == "[[example]]":
          in_example = True
          continue
      if in_example and stripped.startswith("["):
          in_example = False
      if in_example:
          match = re.match(r'name\s*=\s*"([^"]+)"', stripped)
          if match:
              examples.append(match.group(1))
    return examples


def default_source_urls() -> dict[str, str]:
    return {
        "readme": f"{RAW_ROOT}/README.md",
        "cargo_toml": f"{RAW_ROOT}/Cargo.toml",
        "crate_root": f"{RAW_ROOT}/src/gpui.rs",
        "contexts": f"{RAW_ROOT}/docs/contexts.md",
        "key_dispatch": f"{RAW_ROOT}/docs/key_dispatch.md",
        "hello_world": f"{RAW_ROOT}/examples/hello_world.rs",
    }


def build_snapshot(gpui_rs: str, cargo_toml: str) -> dict[str, object]:
    examples = extract_examples(cargo_toml)
    if not examples:
        examples = [
            "hello_world",
            "animation",
            "gif_viewer",
            "image",
            "input",
            "opacity",
            "set_menus",
            "shadow",
            "svg",
            "text_wrapper",
            "uniform_list",
            "window",
            "window_positioning",
            "window_shadow",
        ]
    return {
        "upstream_revision": GPUI_UPSTREAM_REVISION,
        "repository": GPUI_REPOSITORY,
        "source_urls": default_source_urls(),
        "public_reexports": extract_public_reexports(gpui_rs),
        "examples": examples,
    }


def write_json(path: Path, payload: dict[str, object]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--gpui-rs", type=Path)
    parser.add_argument("--cargo-toml", type=Path)
    parser.add_argument("--output", type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    urls = default_source_urls()
    gpui_rs = read_optional(args.gpui_rs)
    cargo_toml = read_optional(args.cargo_toml)
    if not gpui_rs:
        gpui_rs = read_url_or_empty(urls["crate_root"])
    if not cargo_toml:
        cargo_toml = read_url_or_empty(urls["cargo_toml"])
    snapshot = build_snapshot(
        gpui_rs,
        cargo_toml,
    )
    if args.output is not None:
        write_json(args.output, snapshot)
    else:
        print(json.dumps(snapshot, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
