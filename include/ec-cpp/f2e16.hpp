/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP

#include <array>
#include <assert.h>
#include <cstdint>
#include <stdlib.h>
#include <tuple>
#include <vector>

#include <ec-cpp/additive_fft.hpp>
#include <ec-cpp/errors.hpp>
#include <ec-cpp/math.hpp>
#include <ec-cpp/poly_encoder.hpp>
#include <ec-cpp/types.hpp>
#include <ec-cpp/walsh.hpp>

namespace ec_cpp {

struct f2e16_Descriptor {
  /// @brief element type of the field
  using Elt = uint16_t;
  using Wide = uint32_t;
  using Multiplier = Elt;

  static constexpr size_t kFieldBits = 16ull;
  static constexpr size_t kFieldSize = (1ull << kFieldBits);

  static constexpr Elt kGenerator = 0x2D;
  static constexpr Elt kOneMask = Elt(kFieldSize - 1ull);
  static constexpr Elt kBase[kFieldBits] = {
      1,     44234, 15374, 5694,  50562, 60718, 37196, 16402,
      27800, 4312,  27250, 47360, 64952, 64308, 65336, 39198};

  /**
   * kLogTable = 0
   * kExpTable = 1
   * kLogWalsh = 2
   */
  using Tables =
      std::tuple<std::array<Elt, kFieldSize>, std::array<Elt, kFieldSize>,
                 std::array<Multiplier, kFieldSize>>;
  const Tables kTables = []() {
    std::array<Elt, kFieldSize> log_table = {0};
    std::array<Elt, kFieldSize> exp_table = {0};

    const Elt mas = (1 << (kFieldBits - 1)) - 1;
    size_t state = 1ull;
    for (size_t i = 0ull; i < size_t(kOneMask); ++i) {
      exp_table[state] = Elt(i);
      if ((state >> (kFieldBits - 1)) != 0) {
        state &= size_t(mas);
        state = (state << 1ull) ^ size_t(kGenerator);
      } else
        state = (state << 1ull);
    }

    exp_table[0] = kOneMask;
    log_table[0] = 0;

    for (size_t i = 0ull; i < kFieldBits; ++i)
      for (size_t j = 0ull; j < (1ull << i); ++j)
        log_table[j + (1ull << i)] = log_table[j] ^ kBase[i];

    for (size_t i = 0ull; i < kFieldSize; ++i)
      log_table[i] = exp_table[size_t(log_table[i])];

    for (size_t i = 0ull; i < kFieldSize; ++i)
      exp_table[size_t(log_table[i])] = Elt(i);

    exp_table[size_t(kOneMask)] = exp_table[0];

    std::array<Multiplier, kFieldSize> log_walsh{log_table};
    log_walsh[0] = 0;
    walsh<f2e16_Descriptor>(log_walsh);

    return std::make_tuple(std::move(log_table), std::move(exp_table),
                           std::move(log_walsh));
  }();

  static Elt fromBEBytes(const uint8_t *data) {
    return Elt(Elt(Elt(*data) << 8) | Elt(*(data + 1)));
  }

  static void toBEBytes(uint8_t *dst, Elt src) {
    dst[0] = (src >> 8ull);
    dst[1] = (src & 0xff);
  }
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
