"""SDK packaging must work from its installed include tree, never src/."""

from contextlib import ExitStack
import importlib.util
import json
from pathlib import Path
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts/ci"))
SPEC = importlib.util.spec_from_file_location("sdk_package", ROOT / "scripts/ci/sdk-package.py")
PACKER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKER)


class SDKHeaderClosureTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name) / "SDK package"
        self.header("cgpui/public.hpp", "#pragma once\n")

    def header(self, name, contents):
        path = self.root / "include" / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(contents, encoding="utf-8")
        return path

    def validate(self):
        PACKER.validate_sdk_headers(self.root)

    def test_clean_transitive_relative_and_include_root_closure(self):
        self.header("cgpui/public.hpp", '#include "detail/bridge.hpp"\n')
        self.header("cgpui/detail/bridge.hpp", '#include "cgpui/core/types.hpp"\n')
        self.header("cgpui/core/types.hpp", '#include "../public.hpp"\n')
        before = sorted(path.relative_to(self.root) for path in self.root.rglob("*"))
        self.validate()
        self.assertEqual(before, sorted(path.relative_to(self.root) for path in self.root.rglob("*")))
        self.assertFalse((self.root / "src").exists())

    def test_local_quoted_include_takes_precedence(self):
        self.header("cgpui/public.hpp", '#include "types.inc"\n')
        self.header("cgpui/types.inc", "#pragma once\n")
        self.validate()

    def test_missing_header_in_transitive_closure_is_rejected(self):
        self.header("cgpui/public.hpp", '#include "detail/bridge.hpp"\n')
        self.header("cgpui/detail/bridge.hpp", '# include "missing.hpp"\n')
        with self.assertRaisesRegex(SystemExit, "missing quoted header.*missing.hpp"):
            self.validate()

    def test_relative_escape_is_rejected_even_when_target_exists(self):
        (self.root.parent / "external.hpp").write_text("#pragma once\n")
        for name in ("../../../external.hpp", r"..\..\..\external.hpp"):
            with self.subTest(name=name):
                self.header("cgpui/public.hpp", f'#include "{name}"\n')
                with self.assertRaisesRegex(SystemExit, "escapes.*include"):
                    self.validate()

    def test_absolute_header_names_are_rejected(self):
        for name in ("/tmp/external.hpp", "C:/private/header.hpp", r"C:\private\header.hpp"):
            with self.subTest(name=name):
                self.header("cgpui/public.hpp", f'#include "{name}"\n')
                with self.assertRaisesRegex(SystemExit, "absolute quoted header"):
                    self.validate()

    def test_stale_src_is_rejected_without_deleting_it(self):
        source = self.root / "src/ui/old.hpp"
        source.parent.mkdir(parents=True)
        source.write_text("user package contents")
        with self.assertRaisesRegex(SystemExit, "source directory.*src"):
            self.validate()
        self.assertEqual(source.read_text(), "user package contents")

    def test_empty_src_is_also_contamination(self):
        (self.root / "src").mkdir()
        with self.assertRaisesRegex(SystemExit, "source directory.*src"):
            self.validate()

    def test_conditional_headers_are_part_of_portable_closure(self):
        self.header("cgpui/public.hpp", '#if OTHER_PLATFORM\n#include "missing.hpp"\n#endif\n')
        with self.assertRaisesRegex(SystemExit, "missing quoted header"):
            self.validate()

    def test_utf8_bom_does_not_hide_the_first_dependency(self):
        self.header("cgpui/public.hpp", '\ufeff#include "missing.hpp"\n')
        with self.assertRaisesRegex(SystemExit, "missing quoted header"):
            self.validate()

    def test_comments_and_spliced_directives(self):
        self.header("cgpui/public.hpp", '/*\n#include "ignored.hpp"\n*/\n'
                    '// #include "ignored.hpp"\n# include \\\n "types.h"\n')
        self.header("cgpui/types.h", "#pragma once\n")
        self.validate()

    def test_missing_and_empty_include_trees_are_rejected(self):
        (self.root / "include/cgpui/public.hpp").unlink()
        with self.assertRaisesRegex(SystemExit, "no public headers"):
            self.validate()
        with self.assertRaisesRegex(SystemExit, "missing.*include"):
            PACKER.validate_sdk_headers(self.root / "absent")

    def test_header_symlinks_cannot_reach_the_checkout(self):
        outside = self.root.parent / "external.hpp"
        outside.write_text("#pragma once\n")
        try:
            (self.root / "include/cgpui/external.hpp").symlink_to(outside)
        except OSError as error:
            self.skipTest(f"symlink creation unavailable: {error}")
        with self.assertRaisesRegex(SystemExit, "symlink"):
            self.validate()

    def test_include_root_must_not_alias_another_tree(self):
        alias = self.root / "aliased package"
        alias.mkdir()
        try:
            (alias / "include").symlink_to(self.root / "include", target_is_directory=True)
        except OSError as error:
            self.skipTest(f"symlink creation unavailable: {error}")
        with self.assertRaisesRegex(SystemExit, "symlink"):
            PACKER.validate_sdk_headers(alias)

    def arguments(self, platform):
        return SimpleNamespace(repo_root=ROOT, package_root=self.root, platform=platform,
                               arch="arm64", mode="release", build_dir=self.root / "build",
                               workspace=ROOT, profile="release", mcpp="mcpp", runtime=None,
                               executable=[])

    def test_both_entrypoints_reject_contamination_before_copying(self):
        (self.root / "src").mkdir()
        with patch.object(PACKER, "check_clean"), patch.object(PACKER, "link_order") as links:
            for entry, platform in ((PACKER.finalize, "linux"), (PACKER.stage_macos, "macos")):
                with self.subTest(platform=platform):
                    with self.assertRaisesRegex(SystemExit, "source directory.*src"):
                        entry(self.arguments(platform))
            links.assert_not_called()

    def test_manifest_schemas_drop_private_header_workaround(self):
        replacements = {"check_clean": None, "link_order": [], "copy_libraries": None,
                        "link_edge": [], "system_libraries": ([], []), "copy_programs": [],
                        "write_cmake": None, "project_version": "0.1.0", "git": "commit\n",
                        "mcpp_info": {}, "compiler_info": {}, "pacman_versions": {},
                        "vendored_dependencies": [], "vulkan_sdk_version": "test"}
        with ExitStack() as stack:
            for name, result in replacements.items():
                stack.enter_context(patch.object(PACKER, name, return_value=result))
            for entry, platform, schema in ((PACKER.finalize, "windows", 4),
                                             (PACKER.finalize, "linux", 4),
                                             (PACKER.stage_macos, "macos", 2)):
                with self.subTest(platform=platform):
                    entry(self.arguments(platform))
                    manifest = json.loads((self.root / "manifest.json").read_text())
                    self.assertEqual(manifest["schema_version"], schema)
                    self.assertNotIn("private_headers", manifest)
                    self.assertNotIn("private_headers_reason", manifest)
                    self.assertFalse((self.root / "src").exists())


if __name__ == "__main__":
    unittest.main()
