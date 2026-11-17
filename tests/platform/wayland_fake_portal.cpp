#include "wayland_fake_portal.hpp"

#include <chrono>

namespace cgpui::test {
namespace {

constexpr const char* settings_interface = "org.freedesktop.portal.Settings";
constexpr const char* portal_path = "/org/freedesktop/portal/desktop";

} // namespace

FakePortal::FakePortal(std::string address, bool supports_read_one)
    : address_(std::move(address)), supports_read_one_(supports_read_one) {}

FakePortal::~FakePortal() {
  stop_.store(true);
  if (thread_.joinable()) thread_.join();
  if (connection_ != nullptr) {
    dbus_connection_close(connection_);
    dbus_connection_unref(connection_);
  }
}

void FakePortal::set_value(
    std::string name_space, std::string key, PortalValue value) {
  const std::lock_guard lock(mutex_);
  values_[{std::move(name_space), std::move(key)}] = std::move(value);
}

bool FakePortal::start() {
  DBusError error;
  dbus_error_init(&error);
  connection_ = dbus_connection_open_private(address_.c_str(), &error);
  bool owned = false;
  const bool registered =
      connection_ != nullptr && dbus_bus_register(connection_, &error);
  // A previous fake portal may still hold the name until the bus notices
  // its disconnect.
  for (int attempt = 0; registered && !owned && attempt < 100; ++attempt) {
    if (dbus_error_is_set(&error)) dbus_error_free(&error);
    owned = dbus_bus_request_name(
                connection_, "org.freedesktop.portal.Desktop",
                DBUS_NAME_FLAG_DO_NOT_QUEUE, &error) ==
            DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER;
    if (!owned) std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  const bool ready = owned && dbus_connection_add_filter(
                                  connection_, &FakePortal::filter, this,
                                  nullptr);
  if (dbus_error_is_set(&error)) dbus_error_free(&error);
  if (!ready) return false;
  dbus_connection_set_exit_on_disconnect(connection_, FALSE);
  thread_ = std::thread([this] { run(); });
  return true;
}

void FakePortal::emit(std::string name_space, std::string key, PortalValue value) {
  set_value(name_space, key, value);
  const std::lock_guard lock(mutex_);
  emissions_.push_back(
      Emission{std::move(name_space), std::move(key), std::move(value)});
}

std::size_t FakePortal::read_one_calls() const {
  return read_one_calls_.load();
}

std::size_t FakePortal::read_calls() const {
  return read_calls_.load();
}

void FakePortal::run() {
  while (!stop_.load()) {
    (void)dbus_connection_read_write_dispatch(connection_, 10);
    send_emissions();
  }
}

void FakePortal::send_emissions() {
  std::vector<Emission> pending;
  {
    const std::lock_guard lock(mutex_);
    pending.swap(emissions_);
  }
  for (const Emission& emission : pending) {
    DBusMessage* signal = make_setting_changed(
        emission.name_space.c_str(), emission.key.c_str(), emission.value);
    dbus_connection_send(connection_, signal, nullptr);
    dbus_message_unref(signal);
  }
  if (!pending.empty()) dbus_connection_flush(connection_);
}

DBusHandlerResult FakePortal::handle(DBusMessage* message) {
  const bool read_one =
      dbus_message_is_method_call(message, settings_interface, "ReadOne");
  const bool read =
      dbus_message_is_method_call(message, settings_interface, "Read");
  if ((!read_one && !read) || !dbus_message_has_path(message, portal_path)) {
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  (read_one ? read_one_calls_ : read_calls_) += 1;
  const char* name_space = nullptr;
  const char* key = nullptr;
  DBusMessage* reply = nullptr;
  if (read_one && !supports_read_one_) {
    reply = dbus_message_new_error(
        message, DBUS_ERROR_UNKNOWN_METHOD, "ReadOne is not implemented");
  } else if (!dbus_message_get_args(
                 message, nullptr, DBUS_TYPE_STRING, &name_space,
                 DBUS_TYPE_STRING, &key, DBUS_TYPE_INVALID)) {
    reply = dbus_message_new_error(message, DBUS_ERROR_INVALID_ARGS, "ss");
  } else {
    const std::lock_guard lock(mutex_);
    const auto found = values_.find({name_space, key});
    if (found == values_.end()) {
      reply = dbus_message_new_error(
          message, "org.freedesktop.portal.Error.NotFound",
          "Requested setting not found");
    } else {
      reply = dbus_message_new_method_return(message);
      DBusMessageIter iterator;
      dbus_message_iter_init_append(reply, &iterator);
      append_portal_value(&iterator, found->second, read);
    }
  }
  dbus_connection_send(connection_, reply, nullptr);
  dbus_message_unref(reply);
  return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult FakePortal::filter(
    DBusConnection*, DBusMessage* message, void* data) {
  return static_cast<FakePortal*>(data)->handle(message);
}

} // namespace cgpui::test
