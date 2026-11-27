#pragma once

namespace cgpui_test {
// The visible child signals after acquiring its own lease. Its unchanged 20s
// execution deadline starts then; queue/startup failure has a separate bound.
unsigned long loader_wait_for_child(void* process, void* desktop_ready);
} // namespace cgpui_test
