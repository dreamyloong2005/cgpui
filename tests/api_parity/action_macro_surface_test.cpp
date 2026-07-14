#include "cgpui/prelude.hpp"

#include <concepts>
#include <string_view>

CGPUI_ACTIONS(SaveDocumentAction, CloseDocumentAction)
CGPUI_ACTIONS_IN(editor, OpenDocumentAction, SelectAllAction)

static_assert(std::default_initializable<SaveDocumentAction>);
static_assert(std::copyable<SaveDocumentAction>);
static_assert(std::equality_comparable<SaveDocumentAction>);
static_assert(cgpui::Action<SaveDocumentAction>);
static_assert(cgpui::Action<CloseDocumentAction>);
static_assert(cgpui::action_name<SaveDocumentAction>() ==
              std::string_view{"SaveDocumentAction"});
static_assert(cgpui::action_name<CloseDocumentAction>() ==
              std::string_view{"CloseDocumentAction"});

static_assert(std::default_initializable<editor::OpenDocumentAction>);
static_assert(std::copyable<editor::OpenDocumentAction>);
static_assert(std::equality_comparable<editor::OpenDocumentAction>);
static_assert(cgpui::Action<editor::OpenDocumentAction>);
static_assert(cgpui::Action<editor::SelectAllAction>);
static_assert(cgpui::action_name<editor::OpenDocumentAction>() ==
              std::string_view{"editor::OpenDocumentAction"});
static_assert(cgpui::action_name<editor::SelectAllAction>() ==
              std::string_view{"editor::SelectAllAction"});

int main() {
  const SaveDocumentAction save;
  const editor::OpenDocumentAction open;
  return save == SaveDocumentAction{} &&
                 open == editor::OpenDocumentAction{}
             ? 0
             : 1;
}
