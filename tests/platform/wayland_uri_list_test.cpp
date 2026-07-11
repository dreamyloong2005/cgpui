#include "wayland_uri_list_internal.hpp"

#include <string>
#include <vector>

int main() {
  const auto files = cgpui::parse_uri_list(
      "# comment\r\n"
      "FILE:///tmp/one.txt\r\n"
      "file://LOCALHOST/home/two%20words.txt\n"
      "file:/var/log/app.log\n"
      "file://remote.example/share/secret.txt\n"
      "https://example.com/not-a-file\n"
      "file:relative.txt\n"
      "file:///tmp/bad%2\n"
      "file:///tmp/nul%00suffix\n"
      "file:///tmp/query?value\n");
  const std::vector<std::string> expected{
      "/tmp/one.txt", "/home/two words.txt", "/var/log/app.log"};
  if (files != expected) return 1;
  if (!cgpui::parse_uri_list("file://server/path\nftp:///tmp/file\n").empty())
    return 2;
  return 0;
}
