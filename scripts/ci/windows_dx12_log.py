"""Fail-closed mcpp execution/result and real common-renderer identity checks."""
from collections import Counter
import re


ANSI = re.compile(r"\x1b\[[0-?]*[ -/]*[@-~]")
CASE = re.compile(r"^([a-z0-9_]+) \.\.\. (ok|FAIL) \([^\r\n]*\)\s*$", re.MULTILINE)
SUMMARY = re.compile(r"^\s*test result (ok|FAILED)\. (\d+) passed; (\d+) failed;[^\r\n]*$", re.MULTILINE)
MARKER = re.compile(r"^CGPUI test run: renderer=([^\r\n]+)$", re.MULTILINE)
IDENTITY = re.compile(
    r"^pixel renderer: requested=(\S+) expected=(\S+) actual=(\S+) selection=(\d+) "
    r"adapter=(.+) vendor=(\d+) luid=(\d+) software=([01]) fallback=(.*)$", re.MULTILINE)


def _reject_skips(text):
    if re.search(r"\bSKIP(?:PED)?\b", text, re.IGNORECASE):
        raise ValueError("unexpected skip: this lane cannot claim omitted coverage")


def check_run(log, expected, renderer, common=()):
    """One invocation, one summary. Success requires every exact named case once."""
    if not expected or renderer not in {"vulkan", "dx12"}:
        raise ValueError("a nonempty reviewed test inventory and explicit supported backend are required")
    text = ANSI.sub("", log).replace("\r\n", "\n")
    _reject_skips(text)
    markers = list(MARKER.finditer(text))
    if [marker[1] for marker in markers] != [renderer]:
        raise ValueError("missing, duplicated or wrong explicit renderer-run marker")
    prefix = text[:markers[0].start()]
    if CASE.search(prefix) or SUMMARY.search(prefix):
        raise ValueError("unowned test result before the explicit backend marker")
    matches = list(CASE.finditer(text))
    names = [match[1] for match in matches]
    if Counter(names) != Counter(expected) or len(set(expected)) != len(expected):
        raise ValueError(f"exact test inventory mismatch: expected {len(expected)}, observed {len(names)}; "
                         f"missing={sorted(set(expected)-set(names))}, "
                         f"extra={sorted(set(names)-set(expected))}, "
                         f"duplicate={[name for name,n in Counter(names).items() if n != 1]}")
    if any(match[2] != "ok" for match in matches):
        raise ValueError("named test failure")
    summaries = list(SUMMARY.finditer(text))
    if len(summaries) != 1 or summaries[0].groups() != ("ok", str(len(expected)), "0"):
        raise ValueError("missing/duplicate/inconsistent success summary")
    if matches and summaries[0].start() < matches[-1].end():
        raise ValueError("summary precedes the final named test")
    identities = 0
    required = set(common) & set(expected)
    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else summaries[0].start()
        block = text[match.end():end]
        raw = [line for line in block.splitlines() if line.startswith("pixel renderer:")]
        reports = list(IDENTITY.finditer(block))
        if len(reports) != len(raw):
            raise ValueError(f"{match[1]}: malformed pixel identity report")
        if match[1] in required and not reports:
            raise ValueError(f"{match[1]}: no actual renderer identity evidence")
        for report in reports:
            requested, wanted, actual, selection, adapter, vendor, luid, software, fallback = report.groups()
            if (requested != renderer or wanted != renderer or actual != renderer or
                    selection != ("2" if renderer == "dx12" else "1") or not adapter.strip() or
                    int(luid) == 0 or fallback.strip() or (renderer == "dx12" and software != "1")):
                raise ValueError(f"{match[1]}: wrong native backend/selection/LUID/software/fallback")
            identities += 1
    return {"test_count": len(names), "identity_reports": identities, "renderer": renderer,
            "names": names, "skips": 0}


def check_both(log, expected, common):
    text = ANSI.sub("", log).replace("\r\n", "\n")
    # Validate the entire command output BEFORE slicing; an initial skip cannot
    # disappear merely because it precedes the first backend's explicit marker.
    _reject_skips(text)
    markers = list(MARKER.finditer(text))
    if [marker[1] for marker in markers] != ["vulkan", "dx12"]:
        raise ValueError("both must contain exactly Vulkan then DX12 runs")
    prefix = text[:markers[0].start()]
    if CASE.search(prefix) or SUMMARY.search(prefix):
        raise ValueError("unowned test result before the first explicit backend marker")
    return [check_run(text[marker.start():markers[index + 1].start() if index == 0 else len(text)],
                      expected, marker[1], common) for index, marker in enumerate(markers)]
