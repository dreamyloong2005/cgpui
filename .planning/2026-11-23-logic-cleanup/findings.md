# Findings

- Existing main build target contains warm MSVC objects; native popup/focus tests must run serially. Audit probes were outside the repository and did not modify source.
- At the baseline, the tracked migration directory contained only docs/migration/README.md; it has now been removed. docs/gpui-to-cpp23-migration.md is a separate current API migration guide, not implicitly authorized for deletion with the directory.
- Task group cancellation is a second path that must release callbacks, not only cancel_task. Releases must happen after unlocking because captured destructors may reenter the runtime.
- AsyncIoHook::cancel holds its state mutex while calling the registry; callback destruction under that lock can deadlock if the capture destructor invokes hook/runtime methods. Account for both registry and state locks.
