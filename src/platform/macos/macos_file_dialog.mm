#include "macos_platform_services_internal.hpp"

#import <UniformTypeIdentifiers/UniformTypeIdentifiers.h>

#include <string>
#include <vector>

namespace cgpui {
namespace {

NSString* file_string(std::string_view value) {
  return [[NSString alloc] initWithBytes:value.data()
                                  length:value.size()
                                encoding:NSUTF8StringEncoding];
}

NSArray<UTType*>* allowed_types(const NativeFileDialogOptions& options) {
  NSMutableArray<UTType*>* types = [NSMutableArray array];
  for (const auto& filter : options.filters) {
    for (const auto& extension : filter.extensions) {
      NSString* value = file_string(extension);
      UTType* type = value == nil ? nil
          : [UTType typeWithFilenameExtension:value];
      if (type != nil) [types addObject:type];
    }
  }
  return types;
}

void configure_panel(
    NSSavePanel* panel,
    const NativeFileDialogOptions& options) {
  NSString* title = file_string(options.title);
  [panel setTitle:title == nil ? @"" : title];
  if (!options.default_directory.empty()) {
    NSString* directory = file_string(options.default_directory);
    if (directory != nil) [panel setDirectoryURL:[NSURL fileURLWithPath:directory]];
  }
  [panel setAllowedContentTypes:allowed_types(options)];
}

}  // namespace

void macos_configure_open_panel(
    NSOpenPanel* panel,
    const NativeFileDialogOptions& options) {
  configure_panel(panel, options);
  [panel setAllowsMultipleSelection:
      options.kind == NativeFileDialogKind::open_files];
  [panel setCanChooseDirectories:
      options.kind == NativeFileDialogKind::pick_directory];
  [panel setCanChooseFiles:
      options.kind != NativeFileDialogKind::pick_directory];
}

void macos_configure_save_panel(
    NSSavePanel* panel,
    const NativeFileDialogOptions& options) {
  configure_panel(panel, options);
  NSString* name = file_string(options.suggested_name);
  [panel setNameFieldStringValue:name == nil ? @"" : name];
}

NativeFileDialogResult macos_file_dialog_result(
    const NativeFileDialogOptions& options,
    NSModalResponse response,
    NSArray<NSURL*>* urls) {
  std::vector<std::string> paths;
  if (response == NSModalResponseOK) {
    for (NSURL* url in urls) {
      const char* value = [[url path] UTF8String];
      if (value != nullptr) paths.emplace_back(value);
    }
  }
  return NativeFileDialogResult{
      .supported = true,
      .accepted = response == NSModalResponseOK && !paths.empty(),
      .backend = "macos",
      .kind = options.kind,
      .paths = std::move(paths),
      .filter_count = options.filters.size()};
}

NativeFileDialogResult macos_show_native_file_dialog(
    const NativeFileDialogOptions& options) {
  if (options.kind == NativeFileDialogKind::save_file) {
    NSSavePanel* panel = [NSSavePanel savePanel];
    macos_configure_save_panel(panel, options);
    const NSModalResponse response = [panel runModal];
    NSArray<NSURL*>* urls = [panel URL] == nil ? @[] : @[[panel URL]];
    return macos_file_dialog_result(options, response, urls);
  }
  NSOpenPanel* panel = [NSOpenPanel openPanel];
  macos_configure_open_panel(panel, options);
  const NSModalResponse response = [panel runModal];
  return macos_file_dialog_result(options, response, [panel URLs]);
}

}  // namespace cgpui
