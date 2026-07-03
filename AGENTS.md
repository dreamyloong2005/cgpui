# CGPUI Development Instructions

## Modular Development First

- New work must be implemented in its intended module from the first version,
  not piled into broad files and split later.
- Before adding an API or behavior, decide its ownership boundary: public leaf
  header, compatibility aggregate header, private/internal header, and focused
  implementation file.
- Keep aggregate headers thin. Public non-template implementation bodies should
  live in `.cpp` files unless there is a concrete template or constexpr reason.
- Do not use broad files such as `ui.cpp`, large `runtime_*` files, renderer
  entry files, or platform application files as temporary dumping grounds.
- Add structure tests alongside behavior tests when a change creates or depends
  on a module boundary. Tests may require leaf files, line-count limits, and
  absence of implementation details in aggregate headers.
- A change is not complete if it passes behavior tests but regresses the module
  layout. Keep the structure green throughout development instead of doing a
  cleanup pass after the fact.
