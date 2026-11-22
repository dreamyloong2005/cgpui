// Declaration fragment included inside WindowRuntime. No runtime state.
void drain_cross_thread_entity_operations();
[[nodiscard]] bool notify_async_io(AsyncIoId id, AsyncIoResult result);
[[nodiscard]] bool cancel_async_io(AsyncIoId id, AsyncIoCompletionCallback& retired_callback);
[[nodiscard]] bool task_active(TaskId id) const;
[[nodiscard]] bool task_complete(TaskId id) const;
[[nodiscard]] bool task_cancelled(TaskId id) const;
[[nodiscard]] bool cancel_task(TaskId id);
[[nodiscard]] RuntimeTaskDiagnostics task_diagnostics() const;
[[nodiscard]] TaskHandle make_task_handle(TaskId id);
[[nodiscard]] TaskCancellationToken make_task_cancellation_token(std::shared_ptr<std::atomic_bool> state);
