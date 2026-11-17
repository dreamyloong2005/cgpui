#include "wayland_fake_portal.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

namespace cgpui::test {
namespace {

constexpr const char* minimal_config =
    "<!DOCTYPE busconfig PUBLIC \"-//freedesktop//DTD D-Bus Bus Configuration "
    "1.0//EN\" \"http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd\">\n"
    "<busconfig>\n"
    "  <type>session</type>\n"
    "  <listen>unix:tmpdir=/tmp</listen>\n"
    "  <policy context=\"default\">\n"
    "    <allow send_destination=\"*\" eavesdrop=\"true\"/>\n"
    "    <allow eavesdrop=\"true\"/>\n"
    "    <allow own=\"*\"/>\n"
    "  </policy>\n"
    "</busconfig>\n";

std::string read_line(FILE* stream) {
  std::string line;
  for (int value = std::fgetc(stream); value != EOF && value != '\n';
       value = std::fgetc(stream)) {
    line.push_back(static_cast<char>(value));
  }
  return line;
}

} // namespace

PrivateSessionBus::~PrivateSessionBus() {
  if (pid_ > 0) kill(pid_, SIGTERM);
  if (!config_path_.empty()) std::remove(config_path_.c_str());
}

bool PrivateSessionBus::start() {
  char path[] = "/tmp/cgpui-dbus-XXXXXX";
  const int fd = mkstemp(path);
  if (fd == -1) return false;
  close(fd);
  config_path_ = path;
  std::ofstream(config_path_) << minimal_config;
  // --fork returns once the daemon listens; it prints the address and pid.
  const std::string command = "dbus-daemon --fork --nopidfile --config-file=" +
                              config_path_ +
                              " --print-address=1 --print-pid=1 2>/dev/null";
  FILE* stream = popen(command.c_str(), "r");
  if (stream == nullptr) return false;
  address_ = read_line(stream);
  const std::string pid = read_line(stream);
  const int status = pclose(stream);
  if (status != 0 || address_.empty() || pid.empty()) return false;
  pid_ = static_cast<pid_t>(std::atoi(pid.c_str()));
  return pid_ > 0;
}

const std::string& PrivateSessionBus::address() const {
  return address_;
}

} // namespace cgpui::test
