#pragma once

#include "cgpui/ui/action.hpp"

#define CGPUI_DETAIL_ACTION_STRING_IMPL(value) #value
#define CGPUI_DETAIL_ACTION_STRING(value) CGPUI_DETAIL_ACTION_STRING_IMPL(value)

#define CGPUI_DETAIL_DECLARE_ACTION(type)                                      \
  struct type {                                                               \
    static constexpr std::string_view name =                                  \
        CGPUI_DETAIL_ACTION_STRING(type);                                      \
    friend constexpr bool operator==(const type&, const type&) noexcept =      \
        default;                                                              \
  };

#define CGPUI_DETAIL_DECLARE_ACTION_IN(namespace_name, type)                   \
  namespace namespace_name {                                                  \
  struct type {                                                               \
    static constexpr std::string_view name =                                  \
        CGPUI_DETAIL_ACTION_STRING(namespace_name) "::"                        \
            CGPUI_DETAIL_ACTION_STRING(type);                                 \
    friend constexpr bool operator==(const type&, const type&) noexcept =      \
        default;                                                              \
  };                                                                          \
  }

#define CGPUI_DETAIL_ACTIONS_1(macro, a1) macro(a1)
#define CGPUI_DETAIL_ACTIONS_2(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_1(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_3(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_2(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_4(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_3(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_5(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_4(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_6(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_5(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_7(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_6(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_8(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_7(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_9(macro, a1, ...)                                 \
  macro(a1) CGPUI_DETAIL_ACTIONS_8(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_10(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_9(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_11(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_10(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_12(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_11(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_13(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_12(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_14(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_13(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_15(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_14(macro, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_16(macro, a1, ...)                                \
  macro(a1) CGPUI_DETAIL_ACTIONS_15(macro, __VA_ARGS__)

#define CGPUI_DETAIL_ACTIONS_IN_1(macro, scope, a1) macro(scope, a1)
#define CGPUI_DETAIL_ACTIONS_IN_2(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_1(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_3(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_2(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_4(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_3(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_5(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_4(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_6(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_5(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_7(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_6(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_8(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_7(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_9(macro, scope, a1, ...)                       \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_8(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_10(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_9(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_11(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_10(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_12(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_11(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_13(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_12(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_14(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_13(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_15(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_14(macro, scope, __VA_ARGS__)
#define CGPUI_DETAIL_ACTIONS_IN_16(macro, scope, a1, ...)                      \
  macro(scope, a1) CGPUI_DETAIL_ACTIONS_IN_15(macro, scope, __VA_ARGS__)

#define CGPUI_DETAIL_ACTIONS_SELECT(                                           \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
    selected, ...)                                                            \
  selected
#define CGPUI_DETAIL_ACTIONS_APPLY_IMPL(selected, arguments) selected arguments
#define CGPUI_DETAIL_ACTIONS_APPLY(selected, arguments)                       \
  CGPUI_DETAIL_ACTIONS_APPLY_IMPL(selected, arguments)

#define CGPUI_ACTIONS(...)                                                    \
  CGPUI_DETAIL_ACTIONS_APPLY(                                                 \
      CGPUI_DETAIL_ACTIONS_SELECT(                                            \
          __VA_ARGS__, CGPUI_DETAIL_ACTIONS_16, CGPUI_DETAIL_ACTIONS_15,      \
          CGPUI_DETAIL_ACTIONS_14, CGPUI_DETAIL_ACTIONS_13,                   \
          CGPUI_DETAIL_ACTIONS_12, CGPUI_DETAIL_ACTIONS_11,                   \
          CGPUI_DETAIL_ACTIONS_10, CGPUI_DETAIL_ACTIONS_9,                    \
          CGPUI_DETAIL_ACTIONS_8, CGPUI_DETAIL_ACTIONS_7,                     \
          CGPUI_DETAIL_ACTIONS_6, CGPUI_DETAIL_ACTIONS_5,                     \
          CGPUI_DETAIL_ACTIONS_4, CGPUI_DETAIL_ACTIONS_3,                     \
          CGPUI_DETAIL_ACTIONS_2, CGPUI_DETAIL_ACTIONS_1),                    \
      (CGPUI_DETAIL_DECLARE_ACTION, __VA_ARGS__))

#define CGPUI_ACTIONS_IN(namespace_name, ...)                                 \
  CGPUI_DETAIL_ACTIONS_APPLY(                                                 \
      CGPUI_DETAIL_ACTIONS_SELECT(                                            \
          __VA_ARGS__, CGPUI_DETAIL_ACTIONS_IN_16,                            \
          CGPUI_DETAIL_ACTIONS_IN_15, CGPUI_DETAIL_ACTIONS_IN_14,             \
          CGPUI_DETAIL_ACTIONS_IN_13, CGPUI_DETAIL_ACTIONS_IN_12,             \
          CGPUI_DETAIL_ACTIONS_IN_11, CGPUI_DETAIL_ACTIONS_IN_10,             \
          CGPUI_DETAIL_ACTIONS_IN_9, CGPUI_DETAIL_ACTIONS_IN_8,               \
          CGPUI_DETAIL_ACTIONS_IN_7, CGPUI_DETAIL_ACTIONS_IN_6,               \
          CGPUI_DETAIL_ACTIONS_IN_5, CGPUI_DETAIL_ACTIONS_IN_4,               \
          CGPUI_DETAIL_ACTIONS_IN_3, CGPUI_DETAIL_ACTIONS_IN_2,               \
          CGPUI_DETAIL_ACTIONS_IN_1),                                         \
      (CGPUI_DETAIL_DECLARE_ACTION_IN, namespace_name, __VA_ARGS__))
