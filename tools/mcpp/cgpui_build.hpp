// Shared helpers for the build.mcpp programs of the cgpui workspace.
//
// A build.mcpp is compiled and run by mcpp before its package is built; it
// configures the package by printing mcpp: directives (docs: build.mcpp).
// Each package's build.mcpp includes this header and calls the helpers it
// needs, so the system-dependency logic lives in one place.
#pragma once

#include <array>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui_build {

inline std::string env(const char* name) {
  const char* value = std::getenv(name);
  return value == nullptr ? std::string{} : std::string{value};
}

inline bool target_is(std::string_view os) {
  return env("MCPP_TARGET_OS") == os;
}

inline void directive(std::string_view key, std::string_view value) {
  std::printf("mcpp:%.*s=%.*s\n", static_cast<int>(key.size()), key.data(),
              static_cast<int>(value.size()), value.data());
}

inline void warning(std::string_view text) {
  directive("warning", text);
}

// Defines CGPUI_SOURCE_ROOT as the repository root, `levels` directories
// above the package's manifest, as a string literal with forward slashes.
// Tests that read repository files use it instead of a run-time variable.
inline void source_root_define(int levels) {
  std::filesystem::path dir{env("MCPP_MANIFEST_DIR")};
  for (int level = 0; level < levels; ++level) {
    dir = dir.parent_path();
  }
  std::string root = dir.generic_string();
  while (root.size() > 1 && root.back() == '/') {
    root.pop_back();
  }
  directive("cfg", "CGPUI_SOURCE_ROOT=\"" + root + "\"");
}

inline std::vector<std::string> split_words(const std::string& text) {
  std::vector<std::string> words;
  std::istringstream stream{text};
  for (std::string word; stream >> word;) {
    words.push_back(word);
  }
  return words;
}

// Runs a command and returns its standard output; `ok` reports exit status 0.
inline std::string capture(const std::string& command, bool& ok) {
  std::string output;
#if defined(_WIN32)
  FILE* pipe = _popen(command.c_str(), "r");
#else
  FILE* pipe = popen(command.c_str(), "r");
#endif
  if (pipe == nullptr) {
    ok = false;
    return output;
  }
  std::array<char, 4096> buffer{};
  for (std::size_t read = 0; (read = std::fread(buffer.data(), 1, buffer.size(), pipe)) > 0;) {
    output.append(buffer.data(), read);
  }
#if defined(_WIN32)
  ok = _pclose(pipe) == 0;
#else
  ok = pclose(pipe) == 0;
#endif
  return output;
}

// Configures the package from pkg-config modules (Linux system libraries):
// -I words become private include directories, other compile words C/C++
// flags; -L, -l and other link words become link search dirs, libraries and
// flags, which reach every consumer's link. `link` = false takes only the
// compile side (a consumer that includes the headers but links through a
// dependency).
inline void pkg_config(const std::vector<std::string>& modules, bool link = true) {
  std::string names;
  for (const std::string& module : modules) {
    names += " " + module;
  }
  directive("rerun-if-env-changed", "PKG_CONFIG_PATH");
  bool ok = false;
  // System include directories are kept (PKG_CONFIG_ALLOW_SYSTEM_CFLAGS):
  // mcpp compiles against its own C library sysroot, so /usr/include is not
  // searched unless a package names it. It is searched after the sysroot
  // (include-dir-after) so the sysroot's C library headers stay first.
  const std::string cflags = capture(
      "PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 pkg-config --cflags" + names, ok);
  if (!ok) {
    warning("pkg-config could not find" + names);
    return;
  }
  for (const std::string& word : split_words(cflags)) {
    if (word == "-I/usr/include") {
      directive("include-dir-after", word.substr(2));
    } else if (word.starts_with("-I")) {
      directive("include-dir", word.substr(2));
    } else {
      directive("cxxflag", word);
      directive("cflag", word);
    }
  }
  if (!link) {
    return;
  }
  const std::string libs = capture("pkg-config --libs" + names, ok);
  for (const std::string& word : split_words(libs)) {
    if (word.starts_with("-l")) {
      directive("link-lib", word.substr(2));
    } else if (word.starts_with("-L")) {
      directive("link-search", word.substr(2));
    } else {
      directive("link-flag", word);
    }
  }
}

// Configures the package for the LunarG Vulkan SDK named by VULKAN_SDK
// (Windows): its Include directory and, when `link`, vulkan-1 from its Lib
// directory.
inline void vulkan_sdk(bool link) {
  directive("rerun-if-env-changed", "VULKAN_SDK");
  const std::string sdk = env("VULKAN_SDK");
  if (sdk.empty()) {
    warning("VULKAN_SDK is not set; install the LunarG Vulkan SDK");
    return;
  }
  const std::filesystem::path root{sdk};
  directive("include-dir", (root / "Include").generic_string());
  if (link) {
    directive("link-search", (root / "Lib").generic_string());
    directive("link-lib", "vulkan-1");
  }
}

}  // namespace cgpui_build
