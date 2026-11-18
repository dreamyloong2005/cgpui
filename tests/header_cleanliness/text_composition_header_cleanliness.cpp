#include "cgpui/ui/text_composition.hpp"

int main() {
  const cgpui::TextComposition composition =
      cgpui::text_composition_from_text("ime");
  const cgpui::TextCompositionClause clause{
      .style = cgpui::ImeClauseStyle::selected_converted};
  return composition.clauses.size() == 1 &&
                 clause.style != cgpui::ImeClauseStyle::input
             ? 0
             : 1;
}
