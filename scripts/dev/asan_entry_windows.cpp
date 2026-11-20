// The entry of every program of the Windows ASan build (test.ps1 -Asan,
// docs/getting-started.md section 7). test.ps1 compiles this file to
// build/asan/asan_entry.obj; asan_workspace.py compiles the workspace with
// /Dmain=cgpui_asan_main and links the object into every program, so a
// program's own main runs here as cgpui_asan_main.
//
// Why: ExitProcess terminates every other thread before it detaches the
// DLLs and frees the threads' FLS data. A GPU driver thread (Intel igvk64,
// NVIDIA nvgpucomp64) that held a lock of the ASan runtime's allocator when
// it was terminated leaves the lock held forever, and the next free() of the
// detach (ucrtbase destroy_fls, a driver's DllMain) waits on it: the process
// hangs in exit and cannot be killed until reboot. Measured: 1 of 8 parallel
// copies of vulkan_frame_lifetime_test in the first round of 20, with
// ASAN_OPTIONS windows_hook_legacy_allocators on and off. After main returns,
// _cexit() runs the atexit handlers and static destructors with every thread
// alive, and TerminateProcess ends the process without the DLL detach.
// Windows ASan has no LeakSanitizer, so no check runs in what is skipped.
//
// No headers: this is compiled with the toolset's cl.exe alone (no INCLUDE).
extern "C" {
__declspec(dllimport) void* __stdcall GetCurrentProcess();
__declspec(dllimport) int __stdcall TerminateProcess(void* process, unsigned code);
void __cdecl _cexit();
}
#pragma comment(lib, "kernel32.lib")

// A program defines one of the two forms of main; the other resolves to the
// stub below, recognised by its address.
int cgpui_asan_main();
int cgpui_asan_main(int argc, char** argv);
int cgpui_asan_no_main_void() { return -1; }
int cgpui_asan_no_main_args(int, char**) { return -1; }
#pragma comment(linker, "/alternatename:?cgpui_asan_main@@YAHXZ=?cgpui_asan_no_main_void@@YAHXZ")
#pragma comment(linker, "/alternatename:?cgpui_asan_main@@YAHHPEAPEAD@Z=?cgpui_asan_no_main_args@@YAHHPEAPEAD@Z")

int main(int argc, char** argv) {
  int (*volatile without_args)() = &cgpui_asan_main;
  const int result = without_args != &cgpui_asan_no_main_void
                         ? without_args()
                         : cgpui_asan_main(argc, argv);
  _cexit();
  TerminateProcess(GetCurrentProcess(), static_cast<unsigned>(result));
  return result;
}
