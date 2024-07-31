/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP

#include <assert.h>
#include <cstdint>
#include <optional>
#include <stdlib.h>
#include <vector>

#include <ec-cpp/errors.hpp>
#include <ec-cpp/math.hpp>
#include <ec-cpp/types.hpp>

namespace ec_cpp {

template <typename TPolyEncoder> struct ReedSolomon final {
  using Shard = std::vector<uint8_t>;

  static Result<ReedSolomon> create(size_t n, size_t k,
                                    const TPolyEncoder &poly_enc) {
    if (n < 2) {
      return Error::kWantedShardCountTooLow;
    }
    if (k < 1) {
      return Error::kWantedPayloadShardCountTooLow;
    }

    const auto k_po2 = math::nextLowPowerOf2(k);
    const auto n_po2 = math::nextHighPowerOf2(n);
    assert(n * k_po2 <= n_po2 * k);

    if (n_po2 > TPolyEncoder::Descriptor::kFieldSize) {
      return Error::kWantedShardCountTooHigh;
    }

    if (!math::isPowerOf2(n_po2) && !math::isPowerOf2(k_po2)) {
      return Error::kArgsMustBePowOf2;
    }
    return ReedSolomon{n_po2, k_po2, n, poly_enc};
  }

  Result<std::vector<Shard>> encode(const Slice<uint8_t> bytes) {
    if (bytes.empty())
      return Error::kPayloadSizeIsZero;

    const auto shard_len = shardLen(bytes.size());
    assert(shard_len > 0);

    const auto validator_count = wanted_n_;
    const auto k2 = k_ * 2;

    std::vector<Shard> shards;
    shards.assign(validator_count, Shard(shard_len));

    for (size_t i = 0ull; i < bytes.size(); i += k2) {
      const size_t chunk_idx = i / k2;
      const auto end = std::min(i + k2, bytes.size());
      assert(i != end);

      Slice<uint8_t> data_piece(&bytes[i], end - i);
      assert(!data_piece.empty());
      assert(data_piece.size() <= k2);

      auto result = poly_enc_.encodeSub(local(), data_piece, n_, k_);
      if (resultHasError(result)) {
        return resultGetError(std::move(result));
      }
      for (size_t val_idx = 0ull; val_idx < validator_count; ++val_idx) {
        auto &shard = shards[val_idx];
        const auto src = local()[val_idx].point_0;
        TPolyEncoder::Descriptor::toBEBytes((uint8_t *)&shard[chunk_idx * 2ull],
                                            src);
      }
    }
    return shards;
  }

  Result<std::vector<uint8_t>>
  reconstruct(const std::vector<Shard> &received_shards) {
    const auto gap = math::sat_sub_unsigned(n_, received_shards.size());

    size_t existential_count(0ull);
    std::optional<size_t> first_shard_len;
    for (size_t i = 0ull; i < std::min(n_, received_shards.size()); ++i) {
      if (!received_shards[i].empty()) {
        ++existential_count;
        if (!first_shard_len)
          first_shard_len = received_shards[i].size() / 2ull;
        else if (*first_shard_len != received_shards[i].size() / 2ull)
          return Error::kInconsistentShardLengths;
      }
    }

    if (existential_count < k_)
      return Error::kNeedMoreShards;

    std::array<typename TPolyEncoder::Descriptor::Multiplier,
               TPolyEncoder::Descriptor::kFieldSize>
        error_poly_in_log = {0};

    poly_enc_.evalErrorPolynomial(received_shards, gap, error_poly_in_log,
                                  TPolyEncoder::Descriptor::kFieldSize);
    const auto shard_len_in_syms = *first_shard_len;

    std::vector<uint8_t> acc;
    acc.reserve(shard_len_in_syms * 2ull * k_);

    local().clear();
    local().reserve(received_shards.size());

    for (size_t i = 0; i < shard_len_in_syms; ++i) {
      local().clear();

      for (const auto &s : received_shards) {
        if (s.empty())
          local().emplace_back(Additive<typename TPolyEncoder::Descriptor>{0});
        else
          local().emplace_back(Additive<typename TPolyEncoder::Descriptor>{
              TPolyEncoder::Descriptor::fromBEBytes(
                  &s[i * sizeof(typename TPolyEncoder::Descriptor::Elt)])});
      }

      assert(local().size() + gap == n_);
      auto result = poly_enc_.reconstructSub(acc, local(), received_shards, gap,
                                             n_, k_, error_poly_in_log);
      assert(!resultHasError(result));
    }
    return acc;
  }

  /// Reconstruct from the set of systematic chunks.
  /// Systematic chunks are the first `k` chunks, which contain the initial
  /// data.
  ///
  /// Provide a vector containing chunk data. If too few chunks are provided,
  /// recovery is not possible. The result may be padded with zeros. Truncate
  /// the output to the expected byte length.
  Result<std::vector<uint8_t>>
  reconstruct_from_systematic(const std::vector<Shard> &chunks) {
    if (chunks.empty()) {
      return Error::kNeedMoreShards;
    }
    auto &first_shard = chunks[0];

    if (chunks.size() < k_) {
      return Error::kNeedMoreShards;
    }

    const auto shard_len = first_shard.size() / 2;
    if (shard_len == 0) {
      return Error::kEmptyShard;
    }

    for (const auto &c : chunks) {
      const auto length = c.size() / 2;
      if (length != shard_len) {
        return Error::kInconsistentShardLengths;
      }
    }

    std::vector<uint8_t> systematic_bytes;
    systematic_bytes.resize(shard_len * 2 * k_);

    uint8_t *ptr = &systematic_bytes[0];
    for (size_t i = 0; i < shard_len; ++i) {
      for (size_t y = 0; y < k_; ++y) {
        const uint8_t *chunk = &chunks[y][i * 2];
        ptr[0] = chunk[0];
        ptr[1] = chunk[1];
        ptr += 2;
      }
    }
    return systematic_bytes;
  }

  /// Return the computed `n` value.
  size_t n() const { return n_; }

  /// Return the computed `k` value.
  size_t k() const { return k_; }

private:
  ReedSolomon(size_t n, size_t k, size_t wanted_n, const TPolyEncoder &poly_enc)
      : n_(n), k_(k), wanted_n_(wanted_n), poly_enc_(poly_enc) {}

  size_t shardLen(size_t payload_size) {
    const auto payload_symbols = (payload_size + 1) / 2;
    const auto shard_symbols_ceil = (payload_symbols + k_ - 1) / k_;
    const auto shard_bytes = shard_symbols_ceil * 2;
    return shard_bytes;
  }

  std::vector<Additive<typename TPolyEncoder::Descriptor>> &local() const {
    thread_local std::vector<Additive<typename TPolyEncoder::Descriptor>> data;
    return data;
  }

  const size_t n_;
  const size_t k_;
  const size_t wanted_n_;
  const TPolyEncoder &poly_enc_;
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
