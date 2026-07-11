#include "win32_native_menu_accelerator_table_internal.hpp"

#include <limits>
#include <utility>
#include <vector>

namespace cgpui {
namespace {

void append_accelerators(
    const std::vector<NativeMenuItem>& items,
    UINT& next_command_id,
    std::vector<ACCEL>& entries) {
  for (const NativeMenuItem& item : items) {
    if (item.kind == NativeMenuItemKind::separator) continue;
    if (item.kind == NativeMenuItemKind::submenu) {
      append_accelerators(item.children, next_command_id, entries);
      continue;
    }

    const UINT command_id = next_command_id++;
    if (!item.accelerator.has_value() ||
        item.accelerator->action != KeyAction::pressed ||
        item.accelerator->modifiers.super ||
        item.accelerator->key_code > (std::numeric_limits<WORD>::max)() ||
        command_id > (std::numeric_limits<WORD>::max)()) {
      continue;
    }

    BYTE flags = FVIRTKEY;
    if (item.accelerator->modifiers.control) flags |= FCONTROL;
    if (item.accelerator->modifiers.alt) flags |= FALT;
    if (item.accelerator->modifiers.shift) flags |= FSHIFT;
    entries.push_back(ACCEL{
        .fVirt = flags,
        .key = static_cast<WORD>(item.accelerator->key_code),
        .cmd = static_cast<WORD>(command_id),
    });
  }
}

} // namespace

Win32NativeMenuAcceleratorTable::Win32NativeMenuAcceleratorTable(
    HACCEL table,
    std::size_t registered_count)
    : table_(table), registered_count_(registered_count) {}

Win32NativeMenuAcceleratorTable::~Win32NativeMenuAcceleratorTable() {
  if (table_ != nullptr) DestroyAcceleratorTable(table_);
}

Win32NativeMenuAcceleratorTable::Win32NativeMenuAcceleratorTable(
    Win32NativeMenuAcceleratorTable&& other) noexcept
    : table_(std::exchange(other.table_, nullptr)),
      registered_count_(std::exchange(other.registered_count_, 0)) {}

Win32NativeMenuAcceleratorTable&
Win32NativeMenuAcceleratorTable::operator=(
    Win32NativeMenuAcceleratorTable&& other) noexcept {
  if (this == &other) return *this;
  if (table_ != nullptr) DestroyAcceleratorTable(table_);
  table_ = std::exchange(other.table_, nullptr);
  registered_count_ = std::exchange(other.registered_count_, 0);
  return *this;
}

std::optional<Win32NativeMenuAcceleratorTable>
Win32NativeMenuAcceleratorTable::build(const NativeMenuModel& model) {
  std::vector<ACCEL> entries;
  entries.reserve(native_menu_accelerator_count(model));
  UINT next_command_id = win32_native_menu_first_command_id;
  append_accelerators(model.items, next_command_id, entries);
  if (entries.empty()) return Win32NativeMenuAcceleratorTable{};

  HACCEL table = CreateAcceleratorTableW(
      entries.data(),
      static_cast<int>(entries.size()));
  if (table == nullptr) return std::nullopt;
  return Win32NativeMenuAcceleratorTable(table, entries.size());
}

bool Win32NativeMenuAcceleratorTable::translate(MSG& message) const {
  return table_ != nullptr && message.hwnd != nullptr &&
      TranslateAcceleratorW(message.hwnd, table_, &message) != 0;
}

std::size_t Win32NativeMenuAcceleratorTable::registered_count() const {
  return registered_count_;
}

} // namespace cgpui
