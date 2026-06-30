#include "cgpui/core/entity.hpp"

#include <string>
#include <type_traits>
#include <utility>

namespace {

struct Document {
  std::string title;
  int revision = 0;
};

struct Panel {
  int index = 0;
};

static_assert(!std::is_same_v<
              cgpui::EntityId<Document>,
              cgpui::EntityId<Panel>>);

static_assert(std::is_same_v<cgpui::Entity<Document>, cgpui::EntityId<Document>>);
static_assert(std::is_same_v<cgpui::Model<Document>, cgpui::EntityId<Document>>);
static_assert(!std::is_same_v<
              cgpui::WeakEntity<Document>,
              cgpui::WeakEntity<Panel>>);

int test_entity_store_insert_get_and_remove() {
  cgpui::EntityStore<Document> store;

  const cgpui::Entity<Document> first =
      store.insert(Document{.title = "first", .revision = 1});
  const cgpui::Model<Document> second =
      store.insert(Document{.title = "second", .revision = 2});

  if (first.value == 0 || second.value != first.value + 1) {
    return 1;
  }
  if (first == second) {
    return 2;
  }

  Document* first_document = store.get(first);
  const Document* second_document =
      std::as_const(store).get(second);
  if (first_document == nullptr || second_document == nullptr) {
    return 3;
  }
  if (first_document->title != "first" ||
      second_document->title != "second") {
    return 4;
  }

  first_document->revision = 3;
  if (std::as_const(store).get(first)->revision != 3) {
    return 5;
  }

  if (!store.remove(first) || store.get(first) != nullptr) {
    return 6;
  }
  if (store.remove(first)) {
    return 7;
  }
  if (store.get(second) == nullptr ||
      store.get(second)->title != "second") {
    return 8;
  }

  return 0;
}

int test_entity_store_keeps_ids_monotonic_after_remove() {
  cgpui::EntityStore<Panel> store;

  const cgpui::Model<Panel> first = store.insert(Panel{.index = 1});
  if (!store.remove(first)) {
    return 10;
  }

  const cgpui::Entity<Panel> second = store.insert(Panel{.index = 2});
  if (second.value <= first.value) {
    return 11;
  }
  if (store.get(second) == nullptr || store.get(second)->index != 2) {
    return 12;
  }

  return 0;
}

int test_weak_entity_stores_typed_id() {
  cgpui::EntityStore<Document> store;

  const cgpui::Entity<Document> entity =
      store.insert(Document{.title = "weak", .revision = 5});
  const cgpui::WeakEntity<Document> weak_entity(entity);

  if (weak_entity.empty()) {
    return 20;
  }
  if (weak_entity.id() != entity) {
    return 21;
  }

  const cgpui::WeakEntity<Document> empty;
  if (!empty.empty() || empty.id().value != 0) {
    return 22;
  }

  return 0;
}

} // namespace

int main() {
  if (const int result = test_entity_store_insert_get_and_remove();
      result != 0) {
    return result;
  }
  if (const int result = test_entity_store_keeps_ids_monotonic_after_remove();
      result != 0) {
    return result;
  }
  if (const int result = test_weak_entity_stores_typed_id(); result != 0) {
    return result;
  }
  return 0;
}
