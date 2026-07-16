#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
repo_root="$(cd "$script_dir/../.." && pwd -P)"
output_root="${1:-build/phase-j-release/linux}"
if [[ "$output_root" != /* ]]; then output_root="$repo_root/$output_root"; fi

"$script_dir/linux-package.sh" release "$output_root"
package_root="$output_root/package"
artifact_root="$output_root/artifacts"
archive="$artifact_root/cgpui-linux-release.tar.gz"
verification_archive="$artifact_root/cgpui-linux-release.verify.tar.gz"
mkdir -p "$artifact_root"
export SOURCE_DATE_EPOCH=1704067200

python3 "$script_dir/create-release-artifact.py" \
  --platform linux --package-root "$package_root" \
  --package-name cgpui-linux-release \
  --archive-name cgpui-linux-release.tar.gz --output "$archive"
python3 "$script_dir/create-release-artifact.py" \
  --platform linux --package-root "$package_root" \
  --package-name cgpui-linux-release \
  --archive-name cgpui-linux-release.tar.gz --output "$verification_archive"
if ! cmp -s "$archive" "$verification_archive"; then
  echo "Linux release archive is not reproducible" >&2
  exit 4
fi
rm -f -- "$verification_archive"
cp "$package_root/release-manifest.json" "$artifact_root/release-manifest.json"
printf '%s\n' "$archive"
