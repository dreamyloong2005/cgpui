// The translation unit of an OS-specific library on the other OSes.
//
// Every workspace member is built on every OS, and mcpp refuses to archive a
// library with no translation unit. packages/platform-win32 (Windows) and
// packages/platform-linux-* (Linux) compile this file instead of their
// sources elsewhere; nothing links those libraries there.
