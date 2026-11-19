"""Dependency provenance for the SDK manifest (docs/sdk-packaging.md).

Reads xmake-requires.lock as committed at HEAD (`git show HEAD:...`), never
the working copy or the effective git / xmake configuration, so a package
mirror or a lock rewritten by the build cannot leak into the manifest.
"""

from __future__ import annotations

import hashlib
import re
import subprocess
from pathlib import Path

LOCK_FILE = "xmake-requires.lock"
SECTIONS = {"windows": "windows|x64", "linux": "linux|x86_64"}
_ENTRY = re.compile(
    r'\["(?P<key>(?P<name>[^"#]+)#(?P<hash>[0-9a-f]+))"\]\s*=\s*\{\s*'
    r'repo\s*=\s*\{\s*branch\s*=\s*"(?P<branch>[^"]*)",\s*'
    r'commit\s*=\s*"(?P<commit>[0-9a-f]+)",\s*url\s*=\s*"(?P<url>[^"]*)"\s*\},'
    r'\s*version\s*=\s*"(?P<version>[^"]*)"')


def committed_lock(repo: Path) -> bytes:
    return subprocess.run(
        ["git", "-C", str(repo), "show", f"HEAD:{LOCK_FILE}"],
        check=True, capture_output=True).stdout


def platform_section(text: str, platform: str) -> str:
    start = text.find(f'["{SECTIONS[platform]}"]')
    if start < 0:
        raise SystemExit(f"{LOCK_FILE} has no {SECTIONS[platform]} section")
    following = re.search(r'\n    \["[^"]+\|[^"]+"\]', text[start + 1:])
    return text[start:start + 1 + following.start()] if following else text[start:]


def lock_provenance(repo: Path, platform: str, bundled: set[str]) -> dict:
    """Every package the committed lock pins for this platform, with
    `bundled` true for the ones copied into the package's lib/."""
    raw = committed_lock(repo)
    section = platform_section(raw.decode("utf-8"), platform)
    packages = [
        {"name": match["name"], "version": match["version"],
         "lock_key": match["key"], "package_hash": match["hash"],
         "repo_url": match["url"], "repo_branch": match["branch"],
         "repo_commit": match["commit"],
         "bundled": match["name"] in bundled}
        for match in _ENTRY.finditer(section)]
    missing = bundled - {package["name"] for package in packages}
    if missing:
        raise SystemExit(f"bundled packages missing from {LOCK_FILE}: {missing}")
    return {"lock_file": LOCK_FILE, "lock_source": "git show HEAD",
            "lock_section": SECTIONS[platform],
            "lock_sha256": hashlib.sha256(raw).hexdigest(),
            "packages": packages}
