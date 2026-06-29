#include "cgpui/ui/element.hpp"

#include <concepts>
#include <memory>
#include <span>

namespace {

class TestElement final : public cgpui::Element {};

class NamedElement final : public cgpui::Element {
 public:
  explicit NamedElement(int value) : value_(value) {}

  [[nodiscard]] int value() const {
    return value_;
  }

 private:
  int value_ = 0;
};

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

int test_element_tree_stores_root_and_children() {
  cgpui::ElementTree tree;

  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(10));
  if (root_id.value == 0 || tree.root_id() != root_id) {
    return 6;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(root_id));
  if (root == nullptr || root->value() != 10 || root->id() != root_id) {
    return 7;
  }

  const cgpui::ElementId first_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(20));
  const cgpui::ElementId second_child_id =
      tree.append_child(root_id, std::make_unique<NamedElement>(30));
  if (first_child_id.value <= root_id.value ||
      second_child_id.value <= first_child_id.value) {
    return 8;
  }

  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  if (children.size() != 2 || children[0] != first_child_id ||
      children[1] != second_child_id) {
    return 9;
  }

  if (!tree.parent(first_child_id).has_value() ||
      *tree.parent(first_child_id) != root_id ||
      !tree.parent(second_child_id).has_value() ||
      *tree.parent(second_child_id) != root_id ||
      tree.parent(root_id).has_value()) {
    return 10;
  }

  auto* second_child = dynamic_cast<NamedElement*>(tree.get(second_child_id));
  if (second_child == nullptr || second_child->value() != 30 ||
      second_child->id() != second_child_id) {
    return 11;
  }

  return 0;
}

int test_element_tree_rejects_unknown_parent() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId missing =
      tree.append_child(cgpui::ElementId{999}, std::make_unique<NamedElement>(2));
  if (missing.value != 0) {
    return 12;
  }
  if (tree.children(cgpui::ElementId{999}).size() != 0 ||
      tree.parent(cgpui::ElementId{999}).has_value() ||
      tree.get(cgpui::ElementId{999}) != nullptr) {
    return 13;
  }
  return tree.children(root_id).size() == 0 ? 0 : 14;
}

int test_element_tree_replaces_root_with_fresh_tree() {
  cgpui::ElementTree tree;
  const cgpui::ElementId old_root =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId old_child =
      tree.append_child(old_root, std::make_unique<NamedElement>(2));

  const cgpui::ElementId new_root =
      tree.set_root(std::make_unique<NamedElement>(3));
  if (new_root.value <= old_child.value || tree.root_id() != new_root) {
    return 15;
  }
  if (tree.get(old_root) != nullptr || tree.get(old_child) != nullptr ||
      tree.children(old_root).size() != 0 ||
      tree.parent(old_child).has_value()) {
    return 16;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(new_root));
  return root != nullptr && root->value() == 3 ? 0 : 17;
}

int test_element_tree_reconciles_root_in_place() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.set_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId reconciled_root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(10));

  if (reconciled_root_id != root_id || tree.root_id() != root_id) {
    return 18;
  }

  const auto* root = dynamic_cast<const NamedElement*>(tree.get(root_id));
  return root != nullptr && root->value() == 10 && root->id() == root_id
      ? 0
      : 19;
}

int test_element_tree_reconciles_children_by_index() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(2));
  const cgpui::ElementId second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(3));

  const cgpui::ElementId reconciled_first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(20));
  const cgpui::ElementId reconciled_second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(30));
  if (reconciled_first_child_id != first_child_id ||
      reconciled_second_child_id != second_child_id) {
    return 20;
  }

  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  if (children.size() != 2 || children[0] != first_child_id ||
      children[1] != second_child_id) {
    return 21;
  }

  const auto* first_child =
      dynamic_cast<const NamedElement*>(tree.get(first_child_id));
  const auto* second_child =
      dynamic_cast<const NamedElement*>(tree.get(second_child_id));
  if (first_child == nullptr || first_child->value() != 20 ||
      first_child->id() != first_child_id || second_child == nullptr ||
      second_child->value() != 30 || second_child->id() != second_child_id) {
    return 22;
  }

  if (!tree.parent(first_child_id).has_value() ||
      *tree.parent(first_child_id) != root_id ||
      !tree.parent(second_child_id).has_value() ||
      *tree.parent(second_child_id) != root_id) {
    return 23;
  }

  return 0;
}

int test_element_tree_reconcile_appends_new_child_index() {
  cgpui::ElementTree tree;
  const cgpui::ElementId root_id =
      tree.reconcile_root(std::make_unique<NamedElement>(1));
  const cgpui::ElementId first_child_id =
      tree.reconcile_child(root_id, 0, std::make_unique<NamedElement>(2));
  const cgpui::ElementId second_child_id =
      tree.reconcile_child(root_id, 1, std::make_unique<NamedElement>(3));

  if (second_child_id.value <= first_child_id.value) {
    return 24;
  }
  const std::span<const cgpui::ElementId> children = tree.children(root_id);
  return children.size() == 2 && children[0] == first_child_id &&
          children[1] == second_child_id
      ? 0
      : 25;
}

int test_element_tree_reconcile_rejects_unknown_parent() {
  cgpui::ElementTree tree;
  const cgpui::ElementId missing = tree.reconcile_child(
      cgpui::ElementId{123},
      0,
      std::make_unique<NamedElement>(1));
  return missing.value == 0 && tree.get(cgpui::ElementId{123}) == nullptr ? 0
                                                                          : 26;
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
  if (const int result = test_element_tree_stores_root_and_children();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_rejects_unknown_parent();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_replaces_root_with_fresh_tree();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconciles_root_in_place();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconciles_children_by_index();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconcile_appends_new_child_index();
      result != 0) {
    return result;
  }
  if (const int result = test_element_tree_reconcile_rejects_unknown_parent();
      result != 0) {
    return result;
  }
  return 0;
}
