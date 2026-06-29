#include "cgpui/ui/element.hpp"

#include <concepts>
#include <memory>

namespace {

class TestElement final : public cgpui::Element {};

static_assert(std::same_as<decltype(cgpui::ElementId{}.value), std::uint64_t>);
static_assert(std::equality_comparable<cgpui::ElementId>);

int test_element_id_defaults_to_invalid() {
  const cgpui::ElementId id{};
  return id.value == 0 ? 0 : 1;
}

int test_element_stores_assigned_id() {
  TestElement element;
  if (element.id().value != 0) {
    return 2;
  }

  element.assign_id(cgpui::ElementId{42});
  if (element.id() != cgpui::ElementId{42}) {
    return 3;
  }

  element.assign_id(cgpui::ElementId{43});
  return element.id() == cgpui::ElementId{43} ? 0 : 4;
}

int test_element_is_polymorphic() {
  std::unique_ptr<cgpui::Element> element = std::make_unique<TestElement>();
  element->assign_id(cgpui::ElementId{7});
  return element->id() == cgpui::ElementId{7} ? 0 : 5;
}

} // namespace

int main() {
  if (const int result = test_element_id_defaults_to_invalid(); result != 0) {
    return result;
  }
  if (const int result = test_element_stores_assigned_id(); result != 0) {
    return result;
  }
  if (const int result = test_element_is_polymorphic(); result != 0) {
    return result;
  }
  return 0;
}
