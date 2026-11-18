#include "win32_ime_composition_detail_internal.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <imm.h>

#include <iostream>
#include <string>

// Synthetic IMM32 composition data (GCS_COMPSTR, GCS_CURSORPOS,
// GCS_COMPATTR, GCS_COMPCLAUSE) through the pure conversion layer.
namespace {

int failures = 0;

void expect(bool value, const char* message) {
  if (!value) {
    std::cerr << "FAIL: " << message << '\n';
    ++failures;
  }
}

bool span_is(const cgpui::ImeComposition& composition, std::size_t index,
             std::uint32_t offset, std::uint32_t length,
             cgpui::ImeClauseStyle style) {
  if (index >= composition.preedit_style_count) return false;
  const auto& span = composition.preedit_styles[index];
  return span.byte_offset == offset && span.byte_length == length &&
         span.clause == style &&
         span.kind == cgpui::ImePreeditStyleKind::underline;
}

void test_attribute_mapping() {
  using cgpui::ImeClauseStyle;
  using cgpui::win32_ime_clause_style;
  expect(win32_ime_clause_style(ATTR_INPUT) == ImeClauseStyle::input, "input");
  expect(win32_ime_clause_style(ATTR_INPUT_ERROR) == ImeClauseStyle::input,
         "input error maps to input");
  expect(win32_ime_clause_style(ATTR_TARGET_CONVERTED) ==
             ImeClauseStyle::selected_converted, "target converted");
  expect(win32_ime_clause_style(ATTR_CONVERTED) == ImeClauseStyle::converted,
         "converted");
  expect(win32_ime_clause_style(ATTR_FIXEDCONVERTED) ==
             ImeClauseStyle::converted, "fixed converted");
  expect(win32_ime_clause_style(ATTR_TARGET_NOTCONVERTED) ==
             ImeClauseStyle::selected_unconverted, "target not converted");
  expect(win32_ime_clause_style(0x7F) == ImeClauseStyle::input, "unknown");
}

void test_utf16_to_utf8_offsets() {
  // "a" (1 byte), U+4E2D (3 bytes), U+1F600 (surrogate pair, 4 bytes).
  const std::wstring text = L"a\x4E2D\xD83D\xDE00";
  expect(cgpui::win32_utf8_offset_for_utf16(text, 0) == 0, "offset 0");
  expect(cgpui::win32_utf8_offset_for_utf16(text, 1) == 1, "after ascii");
  expect(cgpui::win32_utf8_offset_for_utf16(text, 2) == 4, "after CJK");
  expect(cgpui::win32_utf8_offset_for_utf16(text, 3) == 4,
         "inside a surrogate pair maps to its start");
  expect(cgpui::win32_utf8_offset_for_utf16(text, 4) == 8, "after pair");
  expect(cgpui::win32_utf8_offset_for_utf16(text, 9) == 8, "clamped");
}

void test_clauses_with_attributes() {
  using cgpui::ImeClauseStyle;
  // U+4ECA U+65E5 U+306F: two clauses, the first is the conversion target.
  const cgpui::Win32ImeCompositionDetail detail{
      .text = L"\x4ECA\x65E5\x306F",
      .cursor = 2,
      .attributes = {ATTR_TARGET_CONVERTED, ATTR_TARGET_CONVERTED,
                     ATTR_CONVERTED},
      .clauses = {0, 2, 3},
  };
  const auto composition = cgpui::win32_ime_composition_update(detail, {});
  expect(composition.phase == cgpui::ImeCompositionPhase::update, "phase");
  expect(composition.text == "\xE4\xBB\x8A\xE6\x97\xA5\xE3\x81\xAF", "text");
  expect(composition.preedit_cursor_begin == 6 &&
             composition.preedit_cursor_end == 6, "caret in UTF-8 bytes");
  expect(composition.preedit_style_count == 2, "two clauses");
  expect(span_is(composition, 0, 0, 6, ImeClauseStyle::selected_converted),
         "selected clause");
  expect(span_is(composition, 1, 6, 3, ImeClauseStyle::converted),
         "converted clause");
}

void test_attribute_runs_without_clauses() {
  using cgpui::ImeClauseStyle;
  const cgpui::Win32ImeCompositionDetail detail{
      .text = L"ab\x4E2D",
      .cursor = 1,
      .attributes = {ATTR_INPUT, ATTR_INPUT, ATTR_TARGET_NOTCONVERTED},
  };
  const auto composition = cgpui::win32_ime_composition_update(detail, {});
  expect(composition.preedit_cursor_end == 1, "caret after a");
  expect(composition.preedit_style_count == 2, "two attribute runs");
  expect(span_is(composition, 0, 0, 2, ImeClauseStyle::input), "input run");
  expect(span_is(composition, 1, 2, 3, ImeClauseStyle::selected_unconverted),
         "unconverted target run");
}

void test_plain_input_defaults() {
  using cgpui::ImeClauseStyle;
  // Pinyin letters with no attribute, clause or caret data.
  const cgpui::Win32ImeCompositionDetail detail{.text = L"zhong'guo"};
  const auto composition = cgpui::win32_ime_composition_update(detail, {});
  expect(composition.preedit_cursor_begin == 9 &&
             composition.preedit_cursor_end == 9, "caret defaults to the end");
  expect(composition.preedit_style_count == 1 &&
             span_is(composition, 0, 0, 9, ImeClauseStyle::input),
         "one input clause");
  const auto empty = cgpui::win32_ime_composition_update({}, {});
  expect(empty.text.empty() && empty.preedit_style_count == 0 &&
             empty.preedit_cursor_end == 0, "empty composition");
}

void test_clamping_and_capacity() {
  using cgpui::ImeClauseStyle;
  cgpui::Win32ImeCompositionDetail detail{.text = std::wstring(20, L'x'),
                                          .cursor = 40};
  for (std::uint32_t index = 0; index <= 20; ++index) {
    detail.clauses.push_back(index);
  }
  detail.clauses.push_back(30);  // past the end: clamped and dropped
  const auto composition = cgpui::win32_ime_composition_update(detail, {});
  expect(composition.preedit_cursor_end == 20, "caret clamped to the end");
  expect(composition.preedit_style_count ==
             cgpui::kImePreeditStyleSpanCapacity, "capacity reached");
  const std::size_t last = cgpui::kImePreeditStyleSpanCapacity - 1;
  expect(span_is(composition, last, 15, 5, ImeClauseStyle::input),
         "clauses beyond capacity merge into the last span");
}

} // namespace

int main() {
  test_attribute_mapping();
  test_utf16_to_utf8_offsets();
  test_clauses_with_attributes();
  test_attribute_runs_without_clauses();
  test_plain_input_defaults();
  test_clamping_and_capacity();
  if (failures != 0) {
    return 1;
  }
  std::cout << "Win32 IME composition detail conversion passed.\n";
  return 0;
}
