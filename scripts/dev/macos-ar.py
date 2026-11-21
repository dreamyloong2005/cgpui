#!/usr/bin/env python3
"""Allow Xcode's ar to read the object-list response file emitted by mcpp."""

import os
import shlex
import sys
from pathlib import Path

if __name__ == "__main__":
    args = []
    for arg in sys.argv[1:]:
        if arg.startswith("@"):
            args.extend(shlex.split(Path(arg[1:]).read_text(encoding="utf-8")))
        else:
            args.append(arg)
    os.execv("/usr/bin/ar", ["ar", *args])
