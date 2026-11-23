"""Isolated PowerShell driver fixture; never invokes mcpp, Python, or MSVC."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).with_name("fixtures") / "windows_test_driver"
POWERSHELL = os.environ.get("CGPUI_DRIVER_TEST_POWERSHELL") or shutil.which("pwsh") or shutil.which("powershell")


HOST_DETAILS = {}
if sys.platform == "win32" and POWERSHELL:
    host = subprocess.run([POWERSHELL, "-NoProfile", "-NonInteractive", "-Command",
                           "@{ executable = (Get-Process -Id $PID).Path; version = $PSVersionTable.PSVersion.ToString(); "
                           "edition = $PSVersionTable.PSEdition; native_exit_preference = "
                           "[bool](Get-Variable PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) } | ConvertTo-Json -Compress"],
                          text=True, capture_output=True, timeout=15, check=True)
    HOST_DETAILS = json.loads(host.stdout)
    print("PowerShell fixture host: " + json.dumps(HOST_DETAILS, sort_keys=True), file=sys.stderr)


def run_driver(arguments, **environment):
    parent = os.environ.get("CGPUI_DRIVER_TEST_TEMP")
    # PowerShell expands 8.3 aliases in PSScriptRoot/Get-Location. Keep the
    # fixture's expected paths canonical too, including a short-path TEMP.
    run = Path(tempfile.mkdtemp(prefix="driver-", dir=parent)).resolve()
    repo = run / "fake repo"
    scripts = repo / "scripts/dev"
    scripts.mkdir(parents=True)
    source = Path(os.environ.get("CGPUI_DRIVER_TEST_SOURCE", ROOT))
    for name in ("test.ps1", "test-renderer.ps1", "mcpp-env.ps1"):
        if (source / "scripts/dev" / name).exists():
            shutil.copyfile(source / "scripts/dev" / name, scripts / name)
    for name in ("mcpp.ps1", "mcpp.cmd", "msvc-asan-runtime.ps1", "invoke.ps1"):
        shutil.copyfile(FIXTURES / name, scripts / name)
    env = os.environ.copy()
    for key in list(env):
        if key.startswith(("CGPUI_", "MCPP", "MOCK_")):
            env.pop(key)
    env.update({"MCPP": str(scripts / "mcpp.ps1"), "MCPP_HOME": "fixture home",
                "MOCK_DRIVER": str(scripts / "test.ps1"), "MOCK_POWERSHELL": POWERSHELL,
                "MOCK_LOG": str(run / "calls.jsonl"),
                "MOCK_STATE": str(run / "state.json"),
                "MOCK_ASAN_RUNTIME": str(repo / "asan runtime")})
    for key, value in environment.items():
        if value is None:
            env.pop(key, None)
        else:
            env[key] = value
    if environment.get("MOCK_NATIVE"):
        env["MCPP"] = str(scripts / "mcpp.cmd")
    process = subprocess.run([POWERSHELL, "-NoLogo", "-NoProfile", "-NonInteractive",
                              "-ExecutionPolicy", "Bypass", "-File",
                              str(scripts / "invoke.ps1"), *arguments],
                             cwd=run, env=env, text=True, capture_output=True, timeout=30)
    (run / "stdout.log").write_text(process.stdout, encoding="utf-8")
    (run / "stderr.log").write_text(process.stderr, encoding="utf-8")
    log = run / "calls.jsonl"
    calls = [json.loads(line) for line in log.read_text("utf-8-sig").splitlines()] if log.exists() else []
    state_path = run / "state.json"
    state = json.loads(state_path.read_text("utf-8-sig")) if state_path.exists() else None
    return process, calls, state, repo
