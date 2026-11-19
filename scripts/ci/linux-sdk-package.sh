#!/usr/bin/env bash
# CGPUI SDK package (docs/sdk-packaging.md): refuses a checkout that is not
# clean, builds linux-package.sh in place, adds the SDK parts with
# sdk-package.py, archives with create-release-artifact.py and removes the
# intermediate build output (keep it with CGPUI_SDK_KEEP_BUILD=1).
set -euo pipefail

if [[ "$(uname -s)" != "Linux" ]]; then
  echo "linux-sdk-package.sh requires Linux" >&2
  exit 2
fi
mode="${1:-}"
case "$mode" in
  debug|release) ;;
  *)
    echo "usage: linux-sdk-package.sh <debug|release>" >&2
    exit 2
    ;;
esac

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
script_dir="$repo_root/scripts/ci"
output_root="$repo_root/build/release/ide-workbench"
work_relative="build/release/ide-workbench/.build-linux-$mode"
work_root="$repo_root/$work_relative"
package_name="cgpui-sdk-linux-x86_64-$mode"
final_package="$output_root/linux-$mode"
archive="$output_root/$package_name.tar.gz"

python3 "$script_dir/sdk-package.py" check-clean --repo-root "$repo_root"
"$script_dir/linux-package.sh" "$mode" "$work_relative"

# Same environment as linux-package.sh, for the verbose example_workbench
# link line that gives the consumer link order.
source "$script_dir/linux-dependencies.sh"
cgpui_configure_linux_dependency_environment
export XMAKE_CONFIGDIR="$work_root/config"
if [[ -d "$work_root/python-tools/bin" ]]; then
  export PATH="$work_root/python-tools/bin:$PATH"
fi
link_log="$work_root/example_workbench-link.log"
if ! xmake build -P "$repo_root" -v example_workbench >"$link_log" 2>&1; then
  tail -20 "$link_log" >&2
  exit 3
fi

python3 "$script_dir/sdk-package.py" finalize --platform linux \
  --mode "$mode" --repo-root "$repo_root" \
  --binary-root "$work_root/build-root/linux/x86_64/$mode" \
  --link-log "$link_log" --package-root "$work_root/package" \
  --executable hello_window --executable example_workbench
rm -rf -- "$final_package"
cp -R "$work_root/package" "$final_package"

# xmake may rewrite the lock file while resolving packages; the package
# manifest records it, and the committed file is restored.
git -C "$repo_root" checkout -q -- xmake-requires.lock
if [[ "${CGPUI_SDK_KEEP_BUILD:-}" != "1" ]]; then
  rm -rf -- "$work_root"
fi

SOURCE_DATE_EPOCH=1704067200 python3 "$script_dir/create-release-artifact.py" \
  --platform linux --mode "$mode" --package-root "$final_package" \
  --package-name "$package_name" --archive-name "$package_name.tar.gz" \
  --output "$archive"
(cd "$output_root" && sha256sum "$package_name.tar.gz" >"$package_name.tar.gz.sha256")
cat "$archive.sha256"
