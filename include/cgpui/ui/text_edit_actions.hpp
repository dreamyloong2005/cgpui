#pragma once

namespace cgpui {

enum class TextEditAction {
  move_previous,
  move_next,
  extend_previous,
  extend_next,
  move_previous_word,
  move_next_word,
  extend_previous_word,
  extend_next_word,
  move_line_start,
  move_line_end,
  move_previous_line,
  move_next_line,
  extend_line_start,
  extend_line_end,
  extend_previous_line,
  extend_next_line,
  undo,
  redo,
  backspace,
  delete_forward,
};

} // namespace cgpui
