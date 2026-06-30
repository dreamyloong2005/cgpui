#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace cgpui {

class Clipboard {
 public:
  virtual ~Clipboard() = default;

  [[nodiscard]] virtual std::optional<std::string> read_text() const = 0;
  [[nodiscard]] virtual bool write_text(std::string_view text) = 0;
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

struct WaylandClipboardOptions {
  bool data_device_manager_available = false;
  bool seat_available = false;
};

class WaylandClipboard final : public Clipboard {
 public:
  WaylandClipboard();
  explicit WaylandClipboard(WaylandClipboardOptions options);

  [[nodiscard]] std::optional<std::string> read_text() const override;
  [[nodiscard]] bool write_text(std::string_view text) override;
  [[nodiscard]] WaylandClipboardSupport support() const;

 private:
  MemoryClipboard fallback_;
  WaylandClipboardSupport support_ = WaylandClipboardSupport::unsupported;
};
#endif

[[nodiscard]] std::unique_ptr<Clipboard> create_platform_clipboard();

} // namespace cgpui
