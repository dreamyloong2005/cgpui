#pragma once

#include "cgpui/platform/platform.hpp"
#include "cgpui/ui/text.hpp"
#include "wayland_output_scale.hpp"
#include "wayland_input_internal.hpp"
#include "wayland_protocol_internal.hpp"
#include "wayland_services_internal.hpp"
#include "wayland_window_api.hpp"

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include <algorithm>
#include <array>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <expected>
#include <fcntl.h>
#include <functional>
#include <memory>
#include <optional>
#include <poll.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <sys/mman.h>
#include <unistd.h>
