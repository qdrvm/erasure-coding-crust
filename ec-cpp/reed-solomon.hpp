//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP

#include <assert.h>
#include <cstdint>
#include <optional>
#include <stdlib.h>
#include <vector>

#include "errors.hpp"
#include "math.hpp"
#include "types.hpp"

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
    shards.reserve(validator_count);
    for (size_t ix = 0; ix < validator_count; ++ix)
      shards.emplace_back(shard_len);

    for (size_t i = 0ull; i < bytes.size(); i += k2) {
      const size_t chunk_idx = i / k2;
      const auto end = std::min(i + k2, bytes.size());
      assert(i != end);

      Slice<uint8_t> data_piece(&bytes[i], &bytes[end]);
      assert(!data_piece.empty());
      assert(data_piece.size() <= k2);

      thread_local std::vector<typename TPolyEncoder::Additive> encoding_run{};
      auto result = poly_enc_.encodeSub(encoding_run, data_piece, n_, k_);
      if (resultHasError(result)) {
        return resultGetError(std::move(result));
      }
      for (size_t val_idx = 0ull; val_idx < validator_count; ++val_idx) {
        auto &shard = shards[val_idx];
        const auto src = encoding_run[val_idx]._0;
        TPolyEncoder::Descriptor::toBEBytes((uint8_t *)&shard[chunk_idx * 2ull],
                                            src);
      }
    }
    return shards;
  }

  Result<std::vector<uint8_t>>
  reconstruct(std::vector<Shard> &received_shards) {
    // const auto gap = math::sat_sub_unsigned(n_, received_shards.size());

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

    received_shards.resize(n_);
    std::array<typename TPolyEncoder::Descriptor::Multiplier,
               TPolyEncoder::Descriptor::kFieldSize>
        error_poly_in_log = {0};

    poly_enc_.eval_error_polynomial(received_shards, error_poly_in_log,
                                    TPolyEncoder::Descriptor::kFieldSize);
    const auto shard_len_in_syms = *first_shard_len;

    std::vector<uint8_t> acc;
    acc.reserve(shard_len_in_syms * 2ull * k_);

    for (size_t i = 0; i < shard_len_in_syms; ++i) {
      thread_local static std::vector<
          std::optional<typename TPolyEncoder::Additive>>
          decoding_run;
      decoding_run.clear();
      decoding_run.reserve(received_shards.size());

      for (const auto &s : received_shards) {
        if (s.empty())
          decoding_run.emplace_back(std::nullopt);
        else
          decoding_run.emplace_back(
              typename TPolyEncoder::Descriptor::fromBEBytes(
                  &s[i * sizeof(typename TPolyEncoder::Elt)]));

        assert(decoding_run.size() == n_);
        auto result = poly_enc_.reconstruct_sub(
            acc, decoding_run, received_shards, n_, k_, error_poly_in_log);
        assert(!resultHasError(result));
      }
    }
  }

private:
  ReedSolomon(size_t n, size_t k, size_t wanted_n, const TPolyEncoder &poly_enc)
      : n_(n), k_(k), wanted_n_(wanted_n), poly_enc_(poly_enc) {}

  size_t shardLen(size_t payload_size) {
    const auto payload_symbols = (payload_size + 1) / 2;
    const auto shard_symbols_ceil = (payload_symbols + k_ - 1) / k_;
    const auto shard_bytes = shard_symbols_ceil * 2;
    return shard_bytes;
  }

  const size_t n_;
  const size_t k_;
  const size_t wanted_n_;
  const TPolyEncoder &poly_enc_;
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
