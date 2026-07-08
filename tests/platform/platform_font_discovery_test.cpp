#include "cgpui/platform/platform.hpp"

#include <expected>
#include <memory>
#include <span>
#include <utility>
#include <vector>

namespace {

class EmptyApplication : public cgpui::PlatformApplication {
 public:
  cgpui::Result<std::unique_ptr<cgpui::PlatformWindow>> create_window(
      const cgpui::WindowDescriptor&,
      cgpui::PlatformEventCallback) override {
    return std::unexpected(cgpui::Error{
        .code = cgpui::ErrorCode::window_creation_failed,
        .message = "test application does not create windows"});
  }

  int run() override {
    return 0;
  }

  void quit() override {}
};

class RecordingApplication final : public EmptyApplication {
 public:
  [[nodiscard]] cgpui::PlatformFontDiscoveryResult discover_font_discovery()
      const override {
    std::vector<cgpui::FontFaceDescriptor> records{
        cgpui::FontFaceDescriptor{
            .font = cgpui::FontDescriptor{.family = "Platform Sans"},
            .postscript_name = "PlatformSans-Regular",
            .source = cgpui::FontSource::platform,
            .path = "platform://sans",
        },
    };
    return cgpui::make_platform_font_discovery_result(
        cgpui::PlatformFontDiscoveryBackend::direct_write,
        cgpui::PlatformFontDiscoveryStatus::native_available,
        std::move(records));
  }
};

int test_font_discovery_result_summarizes_records() {
  std::vector<cgpui::FontFaceDescriptor> records{
      cgpui::FontFaceDescriptor{
          .font = cgpui::FontDescriptor{.family = "DirectWrite Sans"},
          .postscript_name = "DirectWriteSans-Regular",
          .source = cgpui::FontSource::platform,
          .path = "directwrite://DirectWrite Sans",
      },
  };

  const cgpui::PlatformFontDiscoveryResult result =
      cgpui::make_platform_font_discovery_result(
          cgpui::PlatformFontDiscoveryBackend::direct_write,
          cgpui::PlatformFontDiscoveryStatus::native_available,
          std::move(records));

  if (result.empty() || result.font_faces().size() != 1 ||
      result.diagnostics.face_count != 1) {
    return 1;
  }
  if (result.diagnostics.backend !=
          cgpui::PlatformFontDiscoveryBackend::direct_write ||
      result.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::native_available ||
      !result.diagnostics.native_backend ||
      result.diagnostics.fallback_used) {
    return 2;
  }
  if (result.font_faces()[0].font.family != "DirectWrite Sans" ||
      result.font_faces()[0].postscript_name != "DirectWriteSans-Regular") {
    return 3;
  }

  std::vector<cgpui::FontFaceDescriptor> fallback_records{
      cgpui::FontFaceDescriptor{
          .font = cgpui::FontDescriptor{.family = "sans-serif"},
          .postscript_name = "fontconfig:sans-serif",
          .source = cgpui::FontSource::platform,
          .path = "fontconfig://sans-serif",
      },
  };
  const cgpui::PlatformFontDiscoveryResult fallback =
      cgpui::make_platform_font_discovery_result(
          cgpui::PlatformFontDiscoveryBackend::fontconfig,
          cgpui::PlatformFontDiscoveryStatus::deterministic_fallback,
          std::move(fallback_records));
  if (fallback.diagnostics.native_backend ||
      !fallback.diagnostics.fallback_used ||
      fallback.diagnostics.face_count != 1) {
    return 4;
  }

  return 0;
}

int test_application_bridges_discovery_result_to_records_and_database() {
  RecordingApplication app;

  const cgpui::PlatformFontDiscoveryResult discovery =
      app.discover_font_discovery();
  if (discovery.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::native_available ||
      discovery.font_faces().size() != 1) {
    return 5;
  }

  const std::vector<cgpui::FontFaceDescriptor> records =
      app.discover_font_records();
  if (records.size() != 1 || records[0].font.family != "Platform Sans") {
    return 6;
  }

  const cgpui::FontDatabase database = app.discover_fonts();
  const cgpui::FontFaceDescriptor* resolved =
      database.resolve(cgpui::FontDescriptor{.family = "Platform Sans"});
  if (database.face_count() != 1 || resolved == nullptr ||
      resolved->path != "platform://sans") {
    return 7;
  }

  EmptyApplication empty;
  const cgpui::PlatformFontDiscoveryResult empty_discovery =
      empty.discover_font_discovery();
  if (!empty_discovery.empty() ||
      empty_discovery.diagnostics.status !=
          cgpui::PlatformFontDiscoveryStatus::unsupported ||
      empty_discovery.diagnostics.face_count != 0 ||
      !empty.discover_font_records().empty() ||
      !empty.discover_fonts().empty()) {
    return 8;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_font_discovery_result_summarizes_records();
      result != 0) {
    return result;
  }
  if (const int result =
          test_application_bridges_discovery_result_to_records_and_database();
      result != 0) {
    return result;
  }
  return 0;
}
