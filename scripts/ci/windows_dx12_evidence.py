"""Owned fresh command logs and receipts; no rerun-overwrite or environment dump."""
from pathlib import Path
import hashlib
import json
import subprocess
import time


def write_json(path, value):
    with path.open("x", encoding="utf-8") as stream:
        json.dump(value, stream, indent=2)
        stream.write("\n")


def source_record(root):
    def git(*arguments):
        return subprocess.check_output(["git", *arguments], cwd=root)
    names = git("ls-files", "-z", "--cached", "--others", "--exclude-standard").decode("utf-8").split("\0")
    entries = []
    for name in sorted(set(names)):
        if not name:
            continue
        path = root / name
        if path.is_symlink() or not path.resolve().is_relative_to(root):
            raise ValueError(f"source manifest refuses linked/out-of-root file: {name}")
        if path.is_file():
            entries.append({"path": name, "sha256": hashlib.sha256(path.read_bytes()).hexdigest()})
    return {"commit": git("rev-parse", "HEAD").decode().strip(),
            "dirty": git("status", "--porcelain").decode("utf-8"), "files": entries}


def run_command(root, output, label, arguments, environment):
    log = output / (label + ".log")
    receipt = output / (label + ".receipt.json")
    if log.exists() or receipt.exists():
        raise ValueError(f"refusing to overwrite original command evidence: {label}")
    command = ["bash", "scripts/ci/run-with-github-annotation.sh", log.as_posix(), "--", *arguments]
    started = time.time()
    record = {"label": label, "command": command, "started_unix": started,
              "exit_code": None, "elapsed_seconds": None,
              "selection": {name: environment.get(name) for name in ("MCPP_JOBS", "CGPUI_RENDERER",
                  "CGPUI_DX12_ADAPTER", "CGPUI_DX12_DEBUG", "CGPUI_DX12_GPU_VALIDATION")}}
    try:
        # The workflow's bounded job is the outer timeout; existing per-native-test
        # watchdogs remain unchanged. No broad process-tree killing is introduced.
        record["exit_code"] = subprocess.run(command, cwd=root, env=environment, check=False).returncode
    except OSError as error:
        record["launch_error"] = str(error)
        raise
    finally:
        record["elapsed_seconds"] = time.time() - started
        write_json(receipt, record)
    if type(record["exit_code"]) is not int or record["exit_code"] != 0:
        raise RuntimeError(f"{label} failed with exit {record['exit_code']}; keep first log/receipt")
    return log.read_text(encoding="utf-8", errors="strict")
