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
if [[ "$output_input" == /* ]]; then
  output_candidate="$output_input"
else
  output_candidate="$repo_root/$output_input"
fi
output_probe="$output_candidate"
output_suffix=""
while [[ ! -e "$output_probe" ]]; do
  output_suffix="/$(basename "$output_probe")$output_suffix"
  output_probe="$(dirname "$output_probe")"
done
[[ -d "$output_probe" ]] || exit 2
output_root="$(cd "$output_probe" && pwd -P)$output_suffix"
[[ "$output_root" != "$repo_root" && "$output_root/" == "$repo_root/"* ]] || exit 2

"$script_dir/macos-package.sh" "$arch" release "$output_root"
package_root="$output_root/package"
artifact_root="$output_root/artifacts"
package_name="cgpui-macos-$arch-release"
archive_name="$package_name.tar.gz"
archive="$artifact_root/$archive_name"
verification_archive="$artifact_root/$package_name.verify.tar.gz"
mkdir -p "$artifact_root"
export SOURCE_DATE_EPOCH=1704067200

python3 "$script_dir/create-release-artifact.py" --platform macos \
  --package-root "$package_root" --package-name "$package_name" \
  --archive-name "$archive_name" --output "$archive"
python3 "$script_dir/create-release-artifact.py" --platform macos \
  --package-root "$package_root" --package-name "$package_name" \
  --archive-name "$archive_name" --output "$verification_archive"
cmp -s "$archive" "$verification_archive" || exit 4

python3 - "$archive" "$package_root/release-manifest.json" \
  "$package_name" "$arch" <<'PY'
import hashlib
import json
import pathlib
import sys
import tarfile

archive_path = pathlib.Path(sys.argv[1])
manifest_path = pathlib.Path(sys.argv[2])
package_name = sys.argv[3]
architecture = sys.argv[4]
manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
if manifest.get("platform") != "macos":
    raise SystemExit("release manifest platform mismatch")
if manifest.get("architecture") != architecture:
    raise SystemExit("release manifest architecture mismatch")
if manifest.get("deployment_target") != "13.0":
    raise SystemExit("release manifest deployment target mismatch")
base = json.loads((manifest_path.parent / "manifest.json").read_text(encoding="utf-8"))
if base.get("mode") != "release":
    raise SystemExit("package manifest mode mismatch")

rows = {row["path"]: row for row in manifest.get("files", [])}
if len(rows) != len(manifest.get("files", [])):
    raise SystemExit("duplicate release manifest path")
expected = {f"{package_name}/{path}" for path in rows}
expected.add(f"{package_name}/release-manifest.json")
with tarfile.open(archive_path, mode="r:gz") as archive:
    members = archive.getmembers()
    if len(members) != len(expected) or {member.name for member in members} != expected:
        raise SystemExit("archive entry set mismatch")
    for relative, row in rows.items():
        member = archive.getmember(f"{package_name}/{relative}")
        source = archive.extractfile(member)
        if not member.isfile() or source is None:
            raise SystemExit(f"archive entry is not a file: {relative}")
        data = source.read()
        if member.size != row["size"] or len(data) != row["size"]:
            raise SystemExit(f"archive size mismatch: {relative}")
        if hashlib.sha256(data).hexdigest() != row["sha256"]:
            raise SystemExit(f"archive digest mismatch: {relative}")
        if f"{member.mode:04o}" != row["mode"]:
            raise SystemExit(f"archive mode mismatch: {relative}")
PY

rm -f -- "$verification_archive"
cp "$package_root/release-manifest.json" "$artifact_root/release-manifest.json"
printf '%s\n' "$archive"
