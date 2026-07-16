#include "cgpui/platform/platform.hpp"

#include <atomic>
#include <array>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>
#include <xcb/xcb.h>

namespace {
xcb_atom_t atom(xcb_connection_t* connection, const char* name) {
  const auto cookie = xcb_intern_atom(
      connection, false, static_cast<std::uint16_t>(std::char_traits<char>::length(name)),
      name);
  xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, nullptr);
  if (reply == nullptr) return XCB_ATOM_NONE;
  const xcb_atom_t value = reply->atom;
  std::free(reply);
  return value;
}

bool wait_for(const std::atomic_int& value, int expected) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
  while (value.load() < expected && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return value.load() >= expected;
}

class XdndSource {
 public:
  bool initialize() {
    int screen_index = 0;
    connection_ = xcb_connect(nullptr, &screen_index);
    if (connection_ == nullptr || xcb_connection_has_error(connection_) != 0) return false;
    xcb_screen_iterator_t screens = xcb_setup_roots_iterator(xcb_get_setup(connection_));
    while (screen_index-- > 0) xcb_screen_next(&screens);
    if (screens.rem == 0) return false;
    window_ = xcb_generate_id(connection_);
    const std::uint32_t mask = XCB_EVENT_MASK_PROPERTY_CHANGE;
    xcb_create_window(
        connection_, XCB_COPY_FROM_PARENT, window_, screens.data->root, 0, 0, 1, 1,
        0, XCB_WINDOW_CLASS_INPUT_OUTPUT, screens.data->root_visual,
        XCB_CW_EVENT_MASK, &mask);
    selection_ = atom(connection_, "XdndSelection");
    xcb_set_selection_owner(connection_, window_, selection_, XCB_CURRENT_TIME);
    xcb_flush(connection_);
    worker_ = std::thread([this] { run(); });
    return true;
  }

  ~XdndSource() {
    stopping_.store(true);
    if (worker_.joinable()) worker_.join();
    if (connection_ != nullptr) {
      if (window_ != XCB_WINDOW_NONE) xcb_destroy_window(connection_, window_);
      xcb_disconnect(connection_);
    }
  }

  void payload(std::string value) {
    std::scoped_lock lock(mutex_);
    payload_ = std::move(value);
  }

  void enter(xcb_window_t target, xcb_atom_t offered_type) {
    send(target, "XdndEnter", {window_, 5U << 24U, offered_type, 0, 0});
  }

  void position(xcb_window_t target) {
    send(target, "XdndPosition",
         {window_, 0, (12U << 16U) | 22U, XCB_CURRENT_TIME,
          atom(connection_, "XdndActionCopy")});
  }

  void drop(xcb_window_t target) {
    send(target, "XdndDrop", {window_, 0, XCB_CURRENT_TIME, 0, 0});
  }

  void leave(xcb_window_t target) {
    send(target, "XdndLeave", {window_, 0, 0, 0, 0});
  }

  [[nodiscard]] int status_count() const { return status_count_.load(); }
  [[nodiscard]] int finished_count() const { return finished_count_.load(); }

 private:
  void send(xcb_window_t target, const char* type, std::array<std::uint32_t, 5> data) {
    xcb_client_message_event_t event{};
    event.response_type = XCB_CLIENT_MESSAGE;
    event.format = 32;
    event.window = target;
    event.type = atom(connection_, type);
    for (std::size_t index = 0; index < data.size(); ++index) {
      event.data.data32[index] = data[index];
    }
    xcb_send_event(connection_, false, target, XCB_EVENT_MASK_NO_EVENT,
                   reinterpret_cast<const char*>(&event));
    xcb_flush(connection_);
  }

  void run() {
    while (!stopping_.load()) {
      while (xcb_generic_event_t* event = xcb_poll_for_event(connection_)) {
        const std::uint8_t type = event->response_type & 0x7f;
        if (type == XCB_SELECTION_REQUEST) {
          const auto& request =
              reinterpret_cast<const xcb_selection_request_event_t&>(*event);
          std::string payload;
          {
            std::scoped_lock lock(mutex_);
            payload = payload_;
          }
          xcb_change_property(
              connection_, XCB_PROP_MODE_REPLACE, request.requestor,
              request.property, request.target, 8,
              static_cast<std::uint32_t>(payload.size()), payload.data());
          xcb_selection_notify_event_t notify{};
          notify.response_type = XCB_SELECTION_NOTIFY;
          notify.time = request.time;
          notify.requestor = request.requestor;
          notify.selection = request.selection;
          notify.target = request.target;
          notify.property = request.property;
          xcb_send_event(connection_, false, request.requestor,
                         XCB_EVENT_MASK_NO_EVENT,
                         reinterpret_cast<const char*>(&notify));
          xcb_flush(connection_);
        } else if (type == XCB_CLIENT_MESSAGE) {
          const auto& message = reinterpret_cast<const xcb_client_message_event_t&>(*event);
          if (message.type == atom(connection_, "XdndStatus")) status_count_.fetch_add(1);
          if (message.type == atom(connection_, "XdndFinished")) finished_count_.fetch_add(1);
        }
        std::free(event);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
  }

  xcb_connection_t* connection_ = nullptr;
  xcb_window_t window_ = XCB_WINDOW_NONE;
  xcb_atom_t selection_ = XCB_ATOM_NONE;
  std::mutex mutex_;
  std::string payload_;
  std::thread worker_;
  std::atomic_bool stopping_{false};
  std::atomic_int status_count_{0};
  std::atomic_int finished_count_{0};
};
}  // namespace

int main() {
  if (std::getenv("DISPLAY") == nullptr) return 0;
  setenv("CGPUI_LINUX_BACKEND", "x11", 1);
  unsetenv("WAYLAND_DISPLAY");
  XdndSource source;
  if (!source.initialize()) return 1;

  auto app = cgpui::create_platform_application();
  if (!app) return 2;
  std::atomic_int file_entered{0};
  std::atomic_int file_dropped{0};
  std::atomic_int text_entered{0};
  std::atomic_int exited{0};
  auto window = (*app)->create_window(
      {.title = "CGPUI X11 XDND", .size = {320.0F, 240.0F}},
      [&](const cgpui::PlatformEvent& event) {
        if (const auto* drag = std::get_if<cgpui::DragEntered>(&event)) {
          if (drag->payload.kind == cgpui::DragDropPayloadKind::files &&
              drag->payload.files == std::vector<std::string>{"/tmp/cgpui one.txt"}) {
            file_entered.fetch_add(1);
          } else if (drag->payload.kind == cgpui::DragDropPayloadKind::text &&
                     drag->payload.text == "CGPUI text drag") {
            text_entered.fetch_add(1);
          }
        } else if (const auto* drag = std::get_if<cgpui::DragDropped>(&event);
                   drag && drag->payload.kind == cgpui::DragDropPayloadKind::files) {
          file_dropped.fetch_add(1);
        } else if (std::holds_alternative<cgpui::DragExited>(event)) {
          exited.fetch_add(1);
        }
      });
  if (!window) return 3;
  const auto native = (*window)->native_surface();
  const auto* x11 = std::get_if<cgpui::X11SurfaceHandle>(&native);
  if (x11 == nullptr) return 4;

  std::thread loop([&] { (*app)->run(); });
  source.payload("file:///tmp/cgpui%20one.txt\r\n");
  source.enter(x11->window, atom(static_cast<xcb_connection_t*>(x11->display), "text/uri-list"));
  source.position(x11->window);
  if (!wait_for(file_entered, 1)) { (*app)->quit(); loop.join(); return 5; }
  source.drop(x11->window);
  if (!wait_for(file_dropped, 1)) { (*app)->quit(); loop.join(); return 6; }

  source.payload("CGPUI text drag");
  source.enter(x11->window, atom(static_cast<xcb_connection_t*>(x11->display), "UTF8_STRING"));
  source.position(x11->window);
  if (!wait_for(text_entered, 1)) { (*app)->quit(); loop.join(); return 7; }
  source.leave(x11->window);
  if (!wait_for(exited, 1)) { (*app)->quit(); loop.join(); return 8; }
  (*app)->quit();
  loop.join();
  return source.status_count() >= 2 && source.finished_count() >= 1 ? 0 : 9;
}
