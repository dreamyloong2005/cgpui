"""Validate installed SDK headers without borrowing source-tree dependencies."""

from pathlib import Path, PureWindowsPath
import re


def quoted_includes(text: str) -> list[str]:
    """Scan all platform branches, preserving strings while removing comments."""
    text = re.sub(r"\\\r?\n", "", text)
    tokens = r'''"(?:[^"\\]|\\.)*"|'(?:[^'\\]|\\.)*'|//[^\n]*|/\*[\s\S]*?\*/'''
    text = re.sub(tokens, lambda match: " " + "\n" * match[0].count("\n")
                  if match[0].startswith(("//", "/*")) else match[0], text)
    return re.findall(r'^\s*#\s*include\s*"([^"\r\n]+)"', text, re.M)


def resolve_quoted_header(header: Path, name: str, include_root: Path) -> Path:
    # Recognize Windows paths even when packaging on a POSIX host. Both slash
    # styles must have the same confinement checks, not platform-dependent gaps.
    name = name.replace("\\", "/")
    if Path(name).is_absolute() or PureWindowsPath(name).drive or name.startswith("/"):
        raise SystemExit(f"absolute quoted header {name!r} in {header}")
    candidates = [(header.parent / name).resolve(), (include_root / name).resolve()]
    for target in candidates:
        if not target.is_relative_to(include_root):
            raise SystemExit(f"quoted header {name!r} in {header} escapes SDK include/")
        if target.is_file():
            return target
    raise SystemExit(f"missing quoted header {name!r} in {header}")


def validate_sdk_headers(package_root: Path) -> None:
    """Reject stale src/ and require every quoted dependency inside include/.

    This is a read-only packaging preflight, not a C++ compiler. System angle
    includes remain toolchain dependencies; all quoted platform branches are
    checked so a package cannot accidentally depend on the checkout.
    """
    package_root = package_root.resolve()
    source_root = package_root / "src"
    if source_root.exists() or source_root.is_symlink():
        raise SystemExit(f"SDK source directory is not allowed: {source_root}; use a clean output")
    include_root = package_root / "include"
    if include_root.is_symlink() or include_root.resolve() != include_root:
        raise SystemExit(f"SDK include tree must not be a symlink: {include_root}")
    if not include_root.is_dir():
        raise SystemExit(f"missing SDK include directory: {include_root}")
    entries = sorted(include_root.rglob("*"))
    for entry in entries:
        if entry.is_symlink() or entry.resolve() != entry:
            raise SystemExit(f"SDK include tree contains a symlink: {entry}")
    pending = [entry for entry in entries if entry.is_file()]
    if not pending:
        raise SystemExit(f"no public headers in SDK include directory: {include_root}")
    visited: set[Path] = set()
    while pending:
        header = pending.pop()
        if header in visited:
            continue
        visited.add(header)
        for name in quoted_includes(header.read_text("utf-8-sig")):
            pending.append(resolve_quoted_header(header, name, include_root))
