//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP

#include <assert.h>
#include <cstdint>
#include <stdlib.h>
#include <vector>

#include "errors.hpp"
#include "math.hpp"
#include "types.hpp"

namespace ec_cpp {

template <typename PolyEncoder> struct ReedSolomon final {
  using Shard = std::vector<uint8_t>;

  static Result<ReedSolomon> create(size_t n, size_t k,
                                    const PolyEncoder &poly_enc) {
    if (n < 2) {
      return Error::kWantedShardCountTooLow;
    }
    if (k < 1) {
      return Error::kWantedPayloadShardCountTooLow;
    }

    const auto k_po2 = math::nextLowPowerOf2(k);
    const auto n_po2 = math::nextHighPowerOf2(n);
    assert(n * k_po2 <= n_po2 * k);

    if (n_po2 > PolyEncoder::Descriptor::kFieldSize) {
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

      auto result = poly_enc_.encodeSub(data_piece, n_, k_);
      if (resultHasError(result)) {
        return resultGetError(std::move(result));
      }
      auto encoding_run = resultGetValue(std::move(result));
      for (size_t val_idx = 0ull; val_idx < validator_count; ++val_idx) {
        auto &shard = shards[val_idx];
        const auto src = encoding_run[val_idx]._0;
        PolyEncoder::Descriptor::toBEBytes((uint8_t *)&shard[chunk_idx * 2ull],
                                           src);
      }
    }
    return shards;
  }

private:
  ReedSolomon(size_t n, size_t k, size_t wanted_n, const PolyEncoder &poly_enc)
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
  const PolyEncoder &poly_enc_;
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
