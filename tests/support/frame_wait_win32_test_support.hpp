#pragma once

namespace cgpui_test::detail {

// Called only after the neutral helper has matched the explicit owner thread.
// Kept out of all cross-platform headers and mathematical support sources.
void pump_frame_wait_win32_messages(void*, unsigned maximum_messages);

} // namespace cgpui_test::detail
