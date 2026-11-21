"""Check that unsupported macOS SDK requests fail without changing the output."""

import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
CI = ROOT / "scripts" / "ci"


class MacOSSDKArchTest(unittest.TestCase):
    def rejected(self, message, *args):
        result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn(message, result.stderr)

    def test_intel_requests_do_not_touch_package(self):
        # Use a valid in-tree output, so path confinement cannot mask a missing
        # architecture check before macos-package.sh cleans the output root.
        build = ROOT / "build"
        build.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=build) as temp:
            root = Path(temp)
            manifest = {"platform": "macos", "mode": "release", "architecture": "x86_64",
                        "deployment_target": "26.0"}
            (root / "manifest.json").write_text(json.dumps(manifest))
            self.rejected("requires native macOS arm64", CI / "macos-package.sh",
                          "x86_64", "release", root)
            self.rejected("requires native macOS arm64", CI / "macos-phase-j-release.sh",
                          "x86_64", root)
            self.rejected("invalid choice", sys.executable, CI / "sdk-package.py",
                          "stage-macos", "--repo-root", ROOT, "--build-dir", root,
                          "--package-root", root, "--mode", "release", "--arch", "x86_64")
            name = "cgpui-macos-x86_64-release"
            archive = root / "artifacts" / (name + ".tar.gz")
            self.rejected("must target arm64", sys.executable, CI / "create-release-artifact.py",
                          "--platform", "macos", "--package-root", root, "--package-name", name,
                          "--archive-name", archive.name, "--output", archive)
            self.rejected("invalid choice", sys.executable, CI / "macos-sdk-consumer.py",
                          archive, "x86_64")
            self.assertEqual(list(root.iterdir()), [root / "manifest.json"])
            self.assertEqual(json.loads((root / "manifest.json").read_text()), manifest)


if __name__ == "__main__":
    unittest.main()
