#pragma once

#include <memory>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

class Clipboard {
 public:
  virtual ~Clipboard() = default;

  [[nodiscard]] virtual std::optional<std::string> read_text() const = 0;
  [[nodiscard]] virtual bool write_text(std::string_view text) = 0;
  [[nodiscard]] virtual std::optional<std::vector<std::string>>
  read_files() const;
  [[nodiscard]] virtual bool write_files(std::span<const std::string> paths);
};

class MemoryClipboard final : public Clipboard {
 public:
  [[nodiscard]] std::optional<std::string> read_text() const override;
  [[nodiscard]] bool write_text(std::string_view text) override;

 private:
  std::optional<std::string> text_;
};

#if defined(__linux__)
enum class WaylandClipboardSupport {
  unsupported,
  no_seat,
  available,
};

enum class WaylandClipboardOperation {
  none,
  read,
  write,
  send,
};

enum class WaylandClipboardFailure {
  none,
  unavailable,
  display_error,
  transfer_timeout,
  receiver_closed,
  io_error,
};

struct WaylandClipboardDiagnostics {
  WaylandClipboardOperation operation = WaylandClipboardOperation::none;
  WaylandClipboardFailure failure = WaylandClipboardFailure::none;
  std::size_t bytes_transferred = 0;
  std::uint64_t revision = 0;
};

struct WaylandClipboardOptions {
  bool data_device_manager_available = false;
  bool seat_available = false;
  bool connect_to_display = false;
  std::string display_name;
};

class WaylandClipboard final : public Clipboard {
 public:
  WaylandClipboard();
  explicit WaylandClipboard(WaylandClipboardOptions options);
  ~WaylandClipboard() override;

  [[nodiscard]] std::optional<std::string> read_text() const override;
  [[nodiscard]] bool write_text(std::string_view text) override;
  [[nodiscard]] WaylandClipboardSupport support() const;
  [[nodiscard]] WaylandClipboardDiagnostics diagnostics() const;

 private:
  struct Connection;

  MemoryClipboard fallback_;
  mutable std::unique_ptr<Connection> connection_;
  WaylandClipboardSupport support_ = WaylandClipboardSupport::unsupported;
};
#endif

[[nodiscard]] std::unique_ptr<Clipboard> create_platform_clipboard();

} // namespace cgpui
