#include "win32_internal.hpp"
#include "win32_native_menu_accelerator_internal.hpp"

namespace cgpui {
namespace {

std::wstring accelerator_key_label(std::uint32_t key_code) {
  if (key_code >= 'A' && key_code <= 'Z') {
    return std::wstring(1, static_cast<wchar_t>(key_code));
  }
  if (key_code >= '0' && key_code <= '9') {
    return std::wstring(1, static_cast<wchar_t>(key_code));
  }
  if (key_code >= VK_F1 && key_code <= VK_F24) {
    return L"F" + std::to_wstring(key_code - VK_F1 + 1U);
  }
  switch (key_code) {
    case VK_BACK: return L"Backspace";
    case VK_TAB: return L"Tab";
    case VK_RETURN: return L"Enter";
    case VK_ESCAPE: return L"Esc";
    case VK_SPACE: return L"Space";
    case VK_PRIOR: return L"Page Up";
    case VK_NEXT: return L"Page Down";
    case VK_END: return L"End";
    case VK_HOME: return L"Home";
    case VK_LEFT: return L"Left";
    case VK_UP: return L"Up";
    case VK_RIGHT: return L"Right";
    case VK_DOWN: return L"Down";
    case VK_INSERT: return L"Ins";
    case VK_DELETE: return L"Del";
    default: return {};
  }
}

void append_modifier(std::wstring& label, bool enabled, const wchar_t* name) {
  if (!enabled) return;
  label += name;
  label += L'+';
}

} // namespace

std::wstring win32_native_menu_display_title(const NativeMenuItem& item) {
  std::wstring title = widen(item.title);
  if (!item.accelerator.has_value()) return title;
  const std::wstring key = accelerator_key_label(item.accelerator->key_code);
  if (key.empty()) return title;

  title += L'\t';
  append_modifier(title, item.accelerator->modifiers.control, L"Ctrl");
  append_modifier(title, item.accelerator->modifiers.alt, L"Alt");
  append_modifier(title, item.accelerator->modifiers.shift, L"Shift");
  append_modifier(title, item.accelerator->modifiers.super, L"Win");
  title += key;
  return title;
}

} // namespace cgpui
