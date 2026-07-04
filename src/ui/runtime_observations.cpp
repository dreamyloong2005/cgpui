#include "ui_internal.hpp"

namespace cgpui {

namespace {

struct PendingWindowObserver {
  SubscriptionId subscription_id;
  WindowObservationCallback callback;
};

struct PendingViewObserver {
  SubscriptionId subscription_id;
  ViewObservationCallback callback;
};

} // namespace

void WindowRuntime::notify_window_observers() {
  std::vector<PendingWindowObserver> pending_observers;
  for (const WindowObserver& observer : window_observers_) {
    if (observer.runtime_id == root_window_runtime_id_ && observer.callback) {
      pending_observers.push_back(PendingWindowObserver{
          .subscription_id = observer.subscription_id,
          .callback = observer.callback,
      });
    }
  }

  for (const PendingWindowObserver& pending : pending_observers) {
    WindowObservationCallback callback = pending.callback;
    if (pending.subscription_id.value != 0) {
      callback = {};
      for (const WindowObserver& observer : window_observers_) {
        if (observer.subscription_id == pending.subscription_id &&
            observer.callback) {
          callback = observer.callback;
          break;
        }
      }
    }
    if (callback) {
      callback(context());
    }
  }
}

void WindowRuntime::notify_view_observers(ViewId view_id) {
  if (view_id.value == 0 || find_view(view_id) == nullptr) {
    return;
  }

  std::vector<PendingViewObserver> pending_observers;
  for (const ViewObserver& observer : view_observers_) {
    if (observer.view_id == view_id && observer.callback) {
      pending_observers.push_back(PendingViewObserver{
          .subscription_id = observer.subscription_id,
          .callback = observer.callback,
      });
    }
  }

  for (const PendingViewObserver& pending : pending_observers) {
    ViewObservationCallback callback = pending.callback;
    if (pending.subscription_id.value != 0) {
      callback = {};
      for (const ViewObserver& observer : view_observers_) {
        if (observer.subscription_id == pending.subscription_id &&
            observer.callback) {
          callback = observer.callback;
          break;
        }
      }
    }
    if (callback) {
      callback(context(), view_id);
    }
  }
}

void WindowRuntime::remove_observers_for_view(ViewId view_id) {
  if (view_id.value == 0) {
    return;
  }

  view_observers_.erase(
      std::remove_if(
          view_observers_.begin(),
          view_observers_.end(),
          [view_id](const ViewObserver& observer) {
            return observer.view_id == view_id;
          }),
      view_observers_.end());
}

} // namespace cgpui
