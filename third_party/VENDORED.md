# Vendored third-party code

| Directory | Project | Version | Upstream commit | License | Source archive SHA-256 |
|---|---|---|---|---|---|
| `lunasvg/` | [lunasvg](https://github.com/sammycage/lunasvg) | v3.5.0 | `83c58df8103dc7dca423dfd824992af94d49bed6` | MIT (`lunasvg/LICENSE`) | `1abf1472ee6c4d19797916e8cc3c2e4b628e0d81178ffac60bdb0d457e32c690` |
| `plutovg/` | [plutovg](https://github.com/sammycage/plutovg) | v1.3.3 | `bbd91f0d06a71491691b36330f29dffa4af87ccf` | MIT (`plutovg/LICENSE`; `source/FTL.TXT` for the FreeType-derived rasterizer) | `2b0d17a6e016f47b86f9c00e2cb82600041b1ea1f7d2a00c2d46ae542cbfed3c` |
| `stb/` | [stb_image](https://github.com/nothings/stb) | `stb_image.h` | `28d546d5eb77d4585506a20480f4de2e706dff4c` | MIT / public domain (in the header) | - |

lunasvg and plutovg are the versions `xmake-requires.lock` pinned before the
mcpp migration (xmake-repo `b9256335`). The archives are the GitHub tag
archives (`archive/refs/tags/<tag>.tar.gz`), whose SHA-256 matches the
xmake-repo recipes. Only `LICENSE`, `README.md`, `include/` and `source/`
are kept; lunasvg's bundled `plutovg/` copy, examples and build files are not.
Each directory's `mcpp.toml` builds it as a static library (the `static`
feature publishes `LUNASVG_BUILD_STATIC` / `PLUTOVG_BUILD_STATIC` to
consumers, as the CMake build does for a static library).

To update: download the new tag archive, check its SHA-256, replace the four
entries, update the version in `mcpp.toml` and this table.
