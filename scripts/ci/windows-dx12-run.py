"""Explicit experimental WARP lanes. Source/driver fixtures are not GPU acceptance."""
from pathlib import Path
import argparse
import json
import os
import subprocess
import sys

from windows_dx12_evidence import run_command, source_record, write_json
from windows_dx12_inventory import load_inventory
from windows_dx12_log import check_both, check_run


def execute(root, output, lane):
    preflight = json.loads((output / "prerequisites.json").read_text(encoding="utf-8-sig"))
    if preflight.get("lane") != lane or preflight.get("passed") is not True:
        raise ValueError("required host preflight did not pass for this exact lane")
    inventory = load_inventory(root)
    write_json(output / "inventory.json", inventory)
    before = source_record(root)
    write_json(output / "source-before.json", before)
    environment = dict(os.environ)
    environment.pop("CGPUI_RENDERER", None)
    environment.update(CGPUI_DX12_ADAPTER="warp", CGPUI_DX12_DEBUG="1", CGPUI_DX12_GPU_VALIDATION="1")
    write_json(output / "selection.json", {name: environment.get(name) for name in (
        "CGPUI_RENDERER", "CGPUI_DX12_ADAPTER", "CGPUI_DX12_DEBUG", "CGPUI_DX12_GPU_VALIDATION")})
    driver = ["pwsh", "-NoProfile", "-File", "scripts/dev/test.ps1"]
    results = []
    environment["MCPP_JOBS"] = "4"
    for suite in (["renderer"] if lane == "hosted-common" else ["renderer", "renderer-dx12"]):
        run_command(root, output, suite + "-build", [*driver, "--no-run", "-p", "tests/" + suite], environment)
    environment["MCPP_JOBS"] = "1"
    # Identity is always first; every shared scene carries its own native identity.
    for name in inventory["common"]:
        log = run_command(root, output, name,
            [*driver, "-Renderer", "dx12", "-p", "tests/renderer", name], environment)
        results.append(check_run(log, [name], "dx12", [name]))
    if lane == "desktop":
        log = run_command(root, output, "native",
            [*driver, "-Renderer", "dx12", "-p", "tests/renderer-dx12"], environment)
        results.append(check_run(log, inventory["suites"]["renderer-dx12"], "dx12"))
        log = run_command(root, output, "shared-both",
            [*driver, "-Renderer", "both", "-p", "tests/renderer"], environment)
        results.extend(check_both(log, inventory["suites"]["renderer"], inventory["common"]))
    after = source_record(root)
    write_json(output / "source-after.json", after)
    if before != after:
        raise ValueError("source changed during CI commands; results are not one frozen tree")
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lane", choices=["hosted-common", "desktop"], required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = args.output.resolve()
    if output.is_relative_to(root):
        parser.error("evidence must be outside the repository, e.g. RUNNER_TEMP")
    output.mkdir(parents=True, exist_ok=True)
    if (output / "result.json").exists():
        parser.error("refusing to reuse an earlier run's final result; choose a fresh evidence directory")
    result = {"lane": args.lane, "passed": False, "execution_scope": "this actual host only"}
    try:
        if sys.platform != "win32":
            raise ValueError("native DX12 CI requires Windows")
        result["runs"] = execute(root, output, args.lane)
        result["passed"] = True
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        result["error"] = str(error)
    write_json(output / "result.json", result)
    print(json.dumps(result, indent=2))
    return 0 if result["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
