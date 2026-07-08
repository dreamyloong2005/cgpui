#include "cgpui/ui/render.hpp"
#include "cgpui/ui/static_element_tree.hpp"

#include <array>
#include <cstddef>
#include <type_traits>

namespace {

struct StaticAuthorView {
  cgpui::StaticIntoElement render_static(
      cgpui::Context<StaticAuthorView>& context);
};

struct DynamicAuthorView {
  cgpui::IntoElement render(cgpui::Context<DynamicAuthorView>& context);
};

static_assert(cgpui::StaticRender<StaticAuthorView>);
static_assert(!cgpui::StaticRender<DynamicAuthorView>);
static_assert(cgpui::Render<DynamicAuthorView>);
static_assert(!std::is_same_v<cgpui::StaticIntoElement, cgpui::IntoElement>);
static_assert(std::is_trivially_copyable_v<cgpui::StaticElementChildRange>);
static_assert(std::is_trivially_copyable_v<cgpui::StaticElementNode>);
static_assert(!std::is_polymorphic_v<cgpui::StaticElementTreeView>);

const std::array<cgpui::StaticElementNode, 4> kNodes{
    cgpui::StaticElementNode{
        .id = cgpui::ElementId{1},
        .children = cgpui::StaticElementChildRange{.first = 0, .count = 2},
        .kind = cgpui::StaticElementKind::container,
        .bounds = cgpui::Rect{.origin = {0.0F, 0.0F},
                              .size = {100.0F, 100.0F}},
    },
    cgpui::StaticElementNode{
        .id = cgpui::ElementId{2},
        .parent_id = cgpui::ElementId{1},
        .children = cgpui::StaticElementChildRange{.first = 2, .count = 1},
        .kind = cgpui::StaticElementKind::row,
        .bounds = cgpui::Rect{.origin = {10.0F, 10.0F},
                              .size = {40.0F, 40.0F}},
    },
    cgpui::StaticElementNode{
        .id = cgpui::ElementId{3},
        .parent_id = cgpui::ElementId{1},
        .kind = cgpui::StaticElementKind::text,
        .bounds = cgpui::Rect{.origin = {20.0F, 20.0F},
                              .size = {50.0F, 50.0F}},
        .text = "overlay",
        .z_order = 5,
    },
    cgpui::StaticElementNode{
        .id = cgpui::ElementId{4},
        .parent_id = cgpui::ElementId{2},
        .kind = cgpui::StaticElementKind::text,
        .bounds = cgpui::Rect{.origin = {12.0F, 12.0F},
                              .size = {5.0F, 5.0F}},
        .text = "leaf",
    },
};

const std::array<cgpui::ElementId, 3> kChildIds{
    cgpui::ElementId{2},
    cgpui::ElementId{3},
    cgpui::ElementId{4},
};

cgpui::StaticElementTreeView make_tree() {
  return cgpui::StaticElementTreeView{
      kNodes,
      kChildIds,
      cgpui::ElementId{1},
  };
}

int test_static_tree_view_validates_dense_records() {
  const cgpui::StaticElementTreeView tree = make_tree();
  if (!tree.valid() || tree.empty() || tree.size() != 4 ||
      tree.root_id() != cgpui::ElementId{1}) {
    return 1;
  }

  const cgpui::StaticElementNode* root = tree.get(cgpui::ElementId{1});
  const cgpui::StaticElementNode* overlay = tree.get(cgpui::ElementId{3});
  if (root == nullptr || overlay == nullptr || overlay->text != "overlay") {
    return 2;
  }

  const std::span<const cgpui::ElementId> root_children =
      tree.children(root->id);
  const std::span<const cgpui::ElementId> row_children =
      tree.children(cgpui::ElementId{2});
  return root_children.size() == 2 &&
                 root_children[0] == cgpui::ElementId{2} &&
                 root_children[1] == cgpui::ElementId{3} &&
                 row_children.size() == 1 &&
                 row_children[0] == cgpui::ElementId{4}
             ? 0
             : 3;
}

int test_static_tree_preorder_traversal_uses_callback_templates() {
  const cgpui::StaticElementTreeView tree = make_tree();
  std::array<std::uint64_t, 4> visited{};
  std::size_t count = 0;
  tree.for_each_preorder([&](const cgpui::StaticElementNode& node) {
    visited[count] = node.id.value;
    count += 1;
  });

  return count == 4 && visited[0] == 1 && visited[1] == 2 &&
                 visited[2] == 4 && visited[3] == 3
             ? 0
             : 4;
}

int test_static_tree_hit_testing_prefers_deepest_highest_z_record() {
  const cgpui::StaticElementTreeView tree = make_tree();
  if (tree.hit_test(cgpui::Point{.x = 13.0F, .y = 13.0F}) !=
      cgpui::ElementId{4}) {
    return 5;
  }
  if (tree.hit_test(cgpui::Point{.x = 25.0F, .y = 25.0F}) !=
      cgpui::ElementId{3}) {
    return 6;
  }
  if (tree.hit_test(cgpui::Point{.x = 90.0F, .y = 90.0F}) !=
      cgpui::ElementId{1}) {
    return 7;
  }
  return tree.hit_test(cgpui::Point{.x = 120.0F, .y = 120.0F}) ==
                 cgpui::ElementId{}
             ? 0
             : 8;
}

int test_static_tree_rejects_sparse_id_records() {
  const std::array<cgpui::StaticElementNode, 1> sparse{
      cgpui::StaticElementNode{.id = cgpui::ElementId{2}},
  };
  const cgpui::StaticElementTreeView tree{
      sparse,
      {},
      cgpui::ElementId{2},
  };
  return !tree.valid() && tree.get(cgpui::ElementId{2}) == nullptr ? 0 : 9;
}

} // namespace

int main() {
  if (const int result = test_static_tree_view_validates_dense_records();
      result != 0) {
    return result;
  }
  if (const int result =
          test_static_tree_preorder_traversal_uses_callback_templates();
      result != 0) {
    return result;
  }
  if (const int result =
          test_static_tree_hit_testing_prefers_deepest_highest_z_record();
      result != 0) {
    return result;
  }
  return test_static_tree_rejects_sparse_id_records();
}
