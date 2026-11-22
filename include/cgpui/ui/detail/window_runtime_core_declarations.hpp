// Declaration fragment included inside WindowRuntime. No runtime state.
void fail_and_quit(Error error);
[[nodiscard]] WindowRuntimeContext context();
bool retire_runtime_user_resources();
