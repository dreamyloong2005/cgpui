#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
void require(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

std::string read(const std::string& path, std::size_t cap) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  require(static_cast<bool>(file), "missing focused owner: " + path);
  const std::string source{std::istreambuf_iterator<char>(file), {}};
  require(static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) <= cap,
          "focused owner exceeded line budget: " + path);
  return source;
}

// Only inspect the named, migrated declarations, not arbitrary C++ syntax.
// Balanced parameters allow constructors with defaults such as Style = {}.
void declarations_only(const std::string& header, const std::string& path,
                       std::initializer_list<const char*> names) {
  const auto source = std::regex_replace(header,
      std::regex(R"(//[^\n]*|/\*[\s\S]*?\*/)"), " ");
  for (const char* name : names) {
    const std::regex call(std::string(R"(\b)") + name + R"(\s*\()");
    bool found = false;
    for (auto it = std::sregex_iterator(source.begin(), source.end(), call);
         it != std::sregex_iterator(); ++it) {
      found = true;
      auto end = static_cast<std::size_t>(it->position() + it->length());
      unsigned depth = 1;
      for (; end < source.size() && depth != 0; ++end) {
        if (source[end] == '(') ++depth;
        else if (source[end] == ')') --depth;
      }
      require(depth == 0, path + ": incomplete declaration: " + name);
      const auto terminator = source.find_first_of(";{", end);
      require(terminator != source.npos, path + ": missing declaration end: " + name);
      const auto start = static_cast<std::size_t>(it->position());
      const auto previous = source.find_last_of(";{}", start);
      const auto prefix = source.substr(previous == source.npos ? 0 : previous + 1,
          start - (previous == source.npos ? 0 : previous + 1));
      const bool header_definition = std::regex_search(prefix,
          std::regex(R"(\b(?:constexpr|consteval|template)\b)"));
      require(source[terminator] == ';' || header_definition,
          path + ": non-template runtime body returned to public header: " + name);
    }
    require(found, path + ": missing public declaration: " + name);
  }
}

struct Owner {
  const char* path;
  std::initializer_list<const char*> symbols;
};
} // namespace

int main() {
  try {
    const std::array paths{"include/cgpui/renderer/glyph_cache.hpp",
        "include/cgpui/renderer/renderer_types.hpp", "include/cgpui/ui/element_core.hpp",
        "include/cgpui/ui/element_text_nodes.hpp"};
    const std::array<std::size_t, 4> caps{110, 190, 280, 175};
    std::array<std::string, 4> headers;
    for (std::size_t index = 0; index < paths.size(); ++index)
      headers[index] = read(paths[index], caps[index]);
    declarations_only(headers[0], paths[0], {"lookup", "allocate", "store", "lookup_count",
        "lookups", "entries", "atlas_pages", "upload_records", "find_entry", "make_page",
        "allocate_from_page"});
    declarations_only(headers[1], paths[1], {"empty", "operator==", "renderer_clip_stack_record",
        "renderer_composition_stack_record", "describe_image_asset"});
    declarations_only(headers[2], paths[2], {"node", "id", "key", "layout", "layout_bounds",
        "hit_test", "paint", "inherit_text_style", "enabled", "set_enabled", "flex_grow",
        "set_flex_grow", "flex_shrink", "set_flex_shrink", "position", "set_position", "inset",
        "set_inset", "z_index", "set_z_index", "layer", "set_layer", "z_order", "focusable",
        "accessibility_role", "accessibility_name", "accessibility_text", "accessibility_value",
        "accessibility_patterns", "handle_accessibility_action", "focus", "on_mount", "on_update",
        "on_unmount", "handle_event", "set_layout_bounds", "assign_id", "set_key",
        "offset_layout_children", "absolute_origin"});
    declarations_only(headers[3], paths[3], {"TextElement", "TextInputElement", "LabelElement",
        "model", "text", "style", "effective_style", "font", "font_size", "glyph_width",
        "shape_run", "accessibility_role", "accessibility_name", "accessibility_text",
        "focusable", "accessibility_value", "handle_event", "z_index", "layer"});

    for (const auto& owner : {
        Owner{"src/renderer/glyph_cache.cpp", {"GlyphCache::lookup", "GlyphCache::store", "GlyphCache::find_entry"}},
        Owner{"src/renderer/glyph_atlas_allocation.cpp", {"GlyphCache::allocate"}},
        Owner{"src/renderer/glyph_atlas_packing.cpp", {"GlyphCache::make_page", "GlyphCache::allocate_from_page"}},
        Owner{"src/renderer/renderer_clip_stack_record.cpp", {"operator==", "renderer_clip_stack_record"}},
        Owner{"src/renderer/renderer_composition_stack_record.cpp", {"renderer_composition_stack_record"}},
        Owner{"src/renderer/image_asset_descriptor.cpp", {"describe_image_asset"}},
        Owner{"src/ui/accessibility_tree_snapshot.cpp", {"AccessibilityTreeSnapshot::node"}},
        Owner{"src/ui/element_properties.cpp", {"Element::id", "Element::z_order", "Element::set_key"}},
        Owner{"src/ui/element_layout_defaults.cpp", {"Element::layout", "Element::hit_test", "absolute_origin"}},
        Owner{"src/ui/element_behavior_defaults.cpp", {"Element::paint", "Element::handle_event", "Element::on_mount"}},
        Owner{"src/ui/element_accessibility_defaults.cpp", {"Element::accessibility_patterns", "Element::handle_accessibility_action"}},
        Owner{"src/ui/element_text_properties.cpp", {"TextElement::TextElement", "TextElement::shape_run"}},
        Owner{"src/ui/element_label_properties.cpp", {"LabelElement::LabelElement", "LabelElement::shape_run"}},
        Owner{"src/ui/element_text_input.cpp", {"TextInputElement::TextInputElement", "TextInputElement::handle_event"}}}) {
      const auto source = read(owner.path, 130);
      for (const char* symbol : owner.symbols)
        require(std::regex_search(source, std::regex(std::string(symbol) + R"(\s*\()")),
                std::string("missing owning definition: ") + owner.path + ": " + symbol);
      if (std::string_view(owner.path).starts_with("src/renderer/"))
        for (const auto* forbidden : {"cgpui/ui/", "cgpui/app/", "vulkan_internal.hpp", "dx12_internal.hpp", "metal_internal.hpp"})
          require(source.find(forbidden) == source.npos,
                  std::string("neutral runtime leaf acquired a higher-level dependency: ") + owner.path);
    }
    require(read("packages/renderer/mcpp.toml", 140).find("../../src/renderer/*.cpp") != std::string::npos &&
                read("packages/ui/mcpp.toml", 90).find("../../src/ui/*.cpp") != std::string::npos,
            "owning packages must compile the focused runtime implementation leaves");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
