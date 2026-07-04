#pragma once

#include "cgpui/ui/view.hpp"

#include <functional>
#include <memory>

namespace cgpui {

class WindowRuntime;
struct AppContext;
struct EventDispatchRecord;
struct RenderRecord;
struct WindowRuntimeContext;

using ActionHandler =
    std::function<EventResult(const WindowRuntimeContext&)>;
using FocusedTextModelMutation = std::function<void(TextModel&)>;
using DeferredCallback = std::function<void(const WindowRuntimeContext&)>;
using TimerCallback = std::function<void(const WindowRuntimeContext&)>;
using TaskCompletionCallback =
    std::function<void(const WindowRuntimeContext&)>;
using UpdateBatchCallback =
    std::function<void(const WindowRuntimeContext&)>;

template <typename T>
using ModelObserver =
    std::function<void(const WindowRuntimeContext&, Model<T>)>;

using RendererFactory =
    std::function<Result<Renderer*>(const RenderSurfaceDescriptor&)>;
using AppRendererFactory = std::function<Result<std::unique_ptr<Renderer>>(
    const RenderSurfaceDescriptor&)>;
using WindowRuntimeFrameCallback =
    std::function<void(const WindowRuntimeContext&)>;
using WindowRuntimeRenderCallback =
    std::function<void(const WindowRuntimeContext&, const RenderRecord&)>;
using WindowRuntimeEventCallback =
    std::function<void(const WindowRuntimeContext&, const EventDispatchRecord&)>;
using WindowRuntimeErrorCallback = std::function<void(const Error&)>;
using AppSetupCallback = std::function<void(WindowRuntime&)>;
using AppContextSetupCallback = std::function<void(AppContext&)>;
using WindowObservationCallback =
    std::function<void(const WindowRuntimeContext&)>;
using ViewObservationCallback =
    std::function<void(const WindowRuntimeContext&, ViewId)>;

} // namespace cgpui
