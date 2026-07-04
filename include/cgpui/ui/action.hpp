#pragma once

#include <concepts>
#include <string_view>
#include <type_traits>

namespace cgpui {
namespace detail {

template <typename T>
using ActionType = std::remove_cvref_t<T>;

} // namespace detail

template <typename T>
concept Action = requires {
  { std::string_view{detail::ActionType<T>::name} } ->
      std::same_as<std::string_view>;
};

template <Action T>
inline constexpr std::string_view action_name_v =
    std::string_view{detail::ActionType<T>::name};

template <Action T>
[[nodiscard]] constexpr std::string_view action_name() noexcept {
  return action_name_v<T>;
}

} // namespace cgpui
