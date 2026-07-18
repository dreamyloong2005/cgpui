#!/usr/bin/env python3
"""Create a content-addressed, deterministic CGPUI release archive."""

from __future__ import annotations

import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import tarfile
import zipfile

DEFAULT_SOURCE_DATE_EPOCH = 1704067200
CONTENT_MANIFEST = "release-manifest.json"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--platform", choices=("windows", "linux", "macos"), required=True
    )
    parser.add_argument("--package-root", type=Path, required=True)
    parser.add_argument("--package-name", required=True)
    parser.add_argument("--archive-name", required=True)
    parser.add_argument("--output", type=Path, required=True)
    return parser.parse_args()


def normalized_mode(relative_path: str) -> int:
    path = PurePosixPath(relative_path)
    if path.parts and path.parts[0] == "bin":
        return 0o755
    return 0o644


def package_files(package_root: Path) -> list[tuple[str, Path]]:
    files = [
        (path.relative_to(package_root).as_posix(), path)
        for path in package_root.rglob("*")
        if path.is_file() and path.name != CONTENT_MANIFEST
    ]
    return sorted(files, key=lambda item: item[0].encode("utf-8"))


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_content_manifest(
    package_root: Path,
    package_name: str,
    platform: str,
    archive_name: str,
    epoch: int,
) -> Path:
    base_manifest = json.loads((package_root / "manifest.json").read_text("utf-8-sig"))
    if base_manifest.get("platform") != platform or base_manifest.get("mode") != "release":
        raise SystemExit("package manifest platform/mode does not match release request")
    architecture = base_manifest.get("architecture")
    if not isinstance(architecture, str) or not architecture:
        raise SystemExit("package manifest architecture is missing")
    if platform == "macos":
        if architecture not in {"arm64", "x86_64"}:
            raise SystemExit("unsupported macOS package architecture")
        if base_manifest.get("deployment_target") != "13.0":
            raise SystemExit("macOS package deployment target must be 13.0")
    entries = [
        {
            "path": relative,
            "size": path.stat().st_size,
            "sha256": sha256(path),
            "mode": f"{normalized_mode(relative):04o}",
        }
        for relative, path in package_files(package_root)
    ]
    manifest = {
        "schema_version": 1,
        "package": package_name,
        "platform": platform,
        "architecture": architecture,
        "archive": archive_name,
        "source_date_epoch": epoch,
        "files": entries,
    }
    if platform == "macos":
        manifest["deployment_target"] = "13.0"
    path = package_root / CONTENT_MANIFEST
    with path.open("w", encoding="utf-8", newline="\n") as output:
        output.write(json.dumps(manifest, indent=2) + "\n")
    return path


def archive_files(package_root: Path) -> list[tuple[str, Path]]:
    return sorted(
        ((path.relative_to(package_root).as_posix(), path)
         for path in package_root.rglob("*") if path.is_file()),
        key=lambda item: item[0].encode("utf-8"),
    )


def write_zip(output: Path, package_root: Path, package_name: str, epoch: int) -> None:
    timestamp = __import__("datetime").datetime.fromtimestamp(epoch, __import__("datetime").timezone.utc)
    date_time = (timestamp.year, timestamp.month, timestamp.day,
                 timestamp.hour, timestamp.minute, timestamp.second)
    with zipfile.ZipFile(output, "w", compression=zipfile.ZIP_STORED) as archive:
        for relative, path in archive_files(package_root):
            info = zipfile.ZipInfo(f"{package_name}/{relative}", date_time)
            info.create_system = 3
            info.external_attr = normalized_mode(relative) << 16
            info.compress_type = zipfile.ZIP_STORED
            archive.writestr(info, path.read_bytes())


def write_tar_gz(output: Path, package_root: Path, package_name: str, epoch: int) -> None:
    with output.open("wb") as raw:
        with gzip.GzipFile(filename="", mode="wb", fileobj=raw, mtime=epoch) as compressed:
            with tarfile.open(fileobj=compressed, mode="w", format=tarfile.USTAR_FORMAT) as archive:
                for relative, path in archive_files(package_root):
                    data = path.read_bytes()
                    info = tarfile.TarInfo(f"{package_name}/{relative}")
                    info.size = len(data)
                    info.mode = normalized_mode(relative)
                    info.uid = info.gid = 0
                    info.uname = info.gname = ""
                    info.mtime = epoch
                    archive.addfile(info, __import__("io").BytesIO(data))


def main() -> int:
    args = parse_args()
    package_root = args.package_root.resolve(strict=True)
    if not package_root.is_dir() or "/" in args.package_name or "\\" in args.package_name:
        raise SystemExit("invalid package root or package name")
    epoch = int(os.environ.get("SOURCE_DATE_EPOCH", DEFAULT_SOURCE_DATE_EPOCH))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_content_manifest(package_root, args.package_name, args.platform,
                           args.archive_name, epoch)
    if args.platform == "windows":
        write_zip(args.output, package_root, args.package_name, epoch)
    else:
        write_tar_gz(args.output, package_root, args.package_name, epoch)
    print(json.dumps({"archive": str(args.output), "sha256": sha256(args.output)}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
