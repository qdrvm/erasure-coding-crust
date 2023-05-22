/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP

#include <limits>
#include <stdlib.h>
#include <type_traits>

static_assert(sizeof(size_t) == 8ull, "Math available for x86-64");

namespace ec_cpp::math {

inline size_t log2(size_t x) {
  return x == 0ull ? 0ull : 63ull - __builtin_clzll(x);
}

inline bool isPowerOf2(size_t x) {
  return ((x > 0ull) && ((x & (x - 1ull)) == 0));
}

inline size_t nextHighPowerOf2(size_t k) {
  if (isPowerOf2(k)) {
    return k;
  }
  const auto p = k == 0ull ? 0ull : 64ull - __builtin_clzll(k);
  return (1ull << p);
}

inline size_t nextLowPowerOf2(size_t k) {
  const auto p = k == 0ull ? 0ull : 64ull - __builtin_clzll(k >> 1ull);
  return (1ull << p);
}

/// @brief https://doc.rust-lang.org/std/intrinsics/fn.saturating_sub.html
template <typename T> inline constexpr T sat_sub_unsigned(T x, T y) {
  static_assert(std::numeric_limits<T>::is_integer &&
                    !std::numeric_limits<T>::is_signed,
                "Value must be an unsigned integer!");
  auto res = x - y;
  res &= -(res <= x);
  return res;
}

} // namespace ec_cpp::math

#endif // NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP
