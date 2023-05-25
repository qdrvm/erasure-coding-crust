/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_WALSH_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_WALSH_HPP

#include <array>
#include <cstdint>
#include <stdlib.h>

namespace ec_cpp {

template <typename TDescriptor>
constexpr void walsh(std::array<typename TDescriptor::Multiplier,
                                TDescriptor::kFieldSize> &data) {
  const auto size = data.size();
  size_t depart_no = 1ull;

  using WideT = typename TDescriptor::Wide;
  while (depart_no < size) {
    size_t j = 0ull;
    const auto depart_no_next = (depart_no << 1ull);
    while (j < size) {
      for (size_t i = j; i < (depart_no + j); ++i) {
        const auto mask = WideT(TDescriptor::kOneMask);
        const WideT tmp2 = WideT(data[i]) + mask - WideT(data[i + depart_no]);
        const WideT tmp1 = WideT(data[i]) + WideT(data[i + depart_no]);
        data[i] = typename TDescriptor::Multiplier(
            (tmp1 & mask) + (tmp1 >> TDescriptor::kFieldBits));
        data[i + depart_no] = typename TDescriptor::Multiplier(
            (tmp2 & mask) + (tmp2 >> TDescriptor::kFieldBits));
      }
      j += depart_no_next;
    }
    depart_no = depart_no_next;
  }
}

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_WALSH_HPP
