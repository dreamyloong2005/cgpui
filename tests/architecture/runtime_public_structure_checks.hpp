#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>

namespace runtime_public_structure {
inline void require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}

inline std::string read(const std::filesystem::path& root, const std::string& path,
                        std::size_t cap) {
  std::ifstream input(root / path);
  require(static_cast<bool>(input), "missing runtime module: " + path);
  const std::string source{std::istreambuf_iterator<char>(input), {}};
  require(static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) <= cap,
          "runtime module exceeded its line budget: " + path);
  return source;
}

inline void check_declarations(const std::filesystem::path& root) {
  const std::string prefix = "include/cgpui/ui/detail/window_runtime_";
  const auto aggregate = read(root, prefix + "declarations.hpp", 40);
  struct Leaf { const char* concern; std::size_t cap; };
  for (const auto& leaf : {
           Leaf{"opaque", 55}, Leaf{"input", 45}, Leaf{"windows", 30},
           Leaf{"text", 20}, Leaf{"tasks", 20}, Leaf{"actions", 15},
           Leaf{"scheduling", 15}, Leaf{"entities", 15}, Leaf{"animations", 10},
           Leaf{"platform", 10}, Leaf{"rendering", 10}, Leaf{"core", 10},
           Leaf{"diagnostics", 5}, Leaf{"template_bridge", 25},
           Leaf{"window_input", 30}, Leaf{"frame_scheduling", 10},
           Leaf{"window_scheduling", 12}, Leaf{"animation_frame_pacing", 20},
           Leaf{"window_accessibility", 8}, Leaf{"window_lifetime", 12},
           Leaf{"window_reclamation", 3}, Leaf{"static_rendering", 25},
           Leaf{"window_event", 20}, Leaf{"window_element_host", 45},
           Leaf{"window_action_dispatch", 5}, Leaf{"system_appearance", 5},
           Leaf{"theme_scheduling", 5}, Leaf{"window_callback_binding", 12}}) {
    const std::string path = prefix + leaf.concern + "_declarations.hpp";
    const auto source = read(root, path, leaf.cap);
    require(aggregate.find("#include \"" + path.substr(8) + "\"") != std::string::npos,
            "runtime declaration leaf is not included: " + path);
    require(source.find_first_of("{}") == std::string::npos,
            "runtime declaration leaf contains a definition: " + path);
    require(source.find("#define") == std::string::npos &&
                source.find("using ") == std::string::npos &&
                source.find("typedef ") == std::string::npos,
            "runtime declaration leaf must not alias private storage: " + path);
  }
  const auto residue = std::regex_replace(aggregate,
      std::regex(R"(//[^\n]*|#include[^\n]*|std::unique_ptr<RuntimeState> state_;)"), "");
  require(residue.find_first_not_of(" \t\r\n") == std::string::npos,
          "runtime declaration aggregate must contain only leaf includes and the state owner");
  require(aggregate.find("std::unique_ptr<RuntimeState> state_;") != std::string::npos,
          "runtime declaration aggregate must own exactly one opaque state");
  const auto public_header = read(root, "include/cgpui/ui/window_runtime.hpp", 263);
  require(public_header.find("#include \"cgpui/ui/detail/window_runtime_declarations.hpp\"") !=
              std::string::npos, "WindowRuntime must use the installed declaration aggregate");
}

inline void check_bridges(const std::filesystem::path& root) {
  struct Module {
    const char* path;
    std::size_t cap;
    std::initializer_list<const char*> symbols;
    std::initializer_list<const char*> storage;
  };
  const auto declarations = read(root,
      "include/cgpui/ui/detail/window_runtime_template_bridge_declarations.hpp", 25);
  std::ifstream input(root / "include/cgpui/ui/runtime_templates.hpp");
  require(static_cast<bool>(input), "missing runtime templates");
  const std::string templates{std::istreambuf_iterator<char>(input), {}};
  require(templates.find("state_->") == std::string::npos &&
              templates.find("#define") == std::string::npos,
          "public runtime templates must use bridges, not storage access or macros");
  require(!std::regex_search(templates,
      std::regex(R"(\b(?:EntityObserver|WindowObserver|ViewObserver)\s*\{)")),
      "public runtime templates must erase observer callbacks, not construct storage records");
  for (const auto& module : {
           Module{"src/ui/runtime_entity_storage.cpp", 50,
               {"ensure_entity_store_value", "entity_store_value", "did_insert_entity",
                "did_remove_entity", "subscribe_view_to_entity_value"},
               {"state_->entities.entity_stores_", "state_->entities.entity_count_",
                "state_->entities.entity_subscriptions_"}},
           Module{"src/ui/runtime_global_storage.cpp", 30,
               {"set_global_value", "global_value"},
               {"state_->entities.globals_", "request_render()"}},
           Module{"src/ui/runtime_observer_registration.cpp", 40,
               {"append_entity_observer", "append_window_observer", "append_view_observer"},
               {"state_->entities.entity_observers_", "state_->entities.window_observers_",
                "state_->entities.view_observers_", "state_->entities.next_subscription_id_",
                "state_->windows.root_window_runtime_id_"}},
           Module{"src/ui/runtime_template_access.cpp", 20,
               {"mutable_element_tree", "cross_thread_entity_queue"},
               {"state_->elements.owned_element_tree_.get()",
                "state_->tasks.cross_thread_entity_queue_"}}}) {
    const auto source = read(root, module.path, module.cap);
    for (const char* symbol : module.symbols) {
      const std::string call = std::string(symbol) + "(";
      require(source.find("WindowRuntime::" + call) != std::string::npos &&
                  declarations.find(call) != std::string::npos &&
                  templates.find(call) != std::string::npos,
              "missing declaration/template/focused implementation bridge: " + call);
    }
    for (const char* storage : module.storage) {
      require(source.find(storage) != std::string::npos,
              std::string("bridge lost its owning storage: ") + module.path + ": " + storage);
    }
  }
}

inline void check(const std::filesystem::path& root) {
  check_declarations(root);
  check_bridges(root);
}
} // namespace runtime_public_structure
