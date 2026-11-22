#pragma once

#include "runtime_state_core_internal.hpp"
#include "runtime_state_windows_internal.hpp"
#include "runtime_state_views_internal.hpp"
#include "runtime_state_elements_internal.hpp"
#include "runtime_state_input_internal.hpp"
#include "runtime_state_actions_internal.hpp"
#include "runtime_state_text_internal.hpp"
#include "runtime_state_entities_internal.hpp"
#include "runtime_state_scheduling_internal.hpp"
#include "runtime_state_animations_internal.hpp"
#include "runtime_state_tasks_internal.hpp"
#include "runtime_state_platform_internal.hpp"
#include "runtime_state_diagnostics_internal.hpp"

namespace cgpui {

struct WindowRuntime::RuntimeState {
  RuntimeState(WindowRuntime& owner, PlatformApplication& application, View& view, RendererFactory factory);
  ~RuntimeState();
  // Task metadata outlives user-bearing groups as a final shutdown safeguard.
  RuntimeTaskState tasks;
  RuntimeCoreState core;
  RuntimeWindowState windows;
  RuntimeViewState views;
  RuntimeElementState elements;
  RuntimeInputState input;
  RuntimeActionState actions;
  RuntimeTextState text;
  RuntimeEntityState entities;
  RuntimeScheduleState scheduling;
  RuntimeAnimationState animations;
  RuntimePlatformState platform;
  RuntimeDiagnosticState diagnostics;
};

} // namespace cgpui
