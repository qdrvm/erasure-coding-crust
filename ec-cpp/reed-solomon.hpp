//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP

#include <stdlib.h>
#include <assert.h>

#include "math.hpp"
#include "errors.hpp"

namespace ec_cpp {

    template<typename PolyEncoder>
    struct ReedSolomon final {
        static Result<ReedSolomon> create(size_t n, size_t k, PolyEncoder &&poly_enc) {
            if (n < 2) {
                return Error::kWantedShardCountTooLow;
            }
            if (k < 1) {
                return Error::kWantedPayloadShardCountTooLow;
            }

            const auto k_po2 = math::nextLowPowerOf2(k);
            const auto n_po2 = math::nextHighPowerOf2(n);
            assert(n * k_po2 <= n_po2 * k);

            if (n_po2 > poly_enc.kFieldSize) {
                    return Error::kWantedShardCountTooHigh;
            }

            if (!math::isPowerOf2(n_po2) && !math::isPowerOf2(k_po2)) {
                return Error::kArgsMustBePowOf2;
            }
            return ReedSolomon{n_po2, k_po2, n, std::move(poly_enc)};
        }

        size_t shardLen(size_t payload_size) {
            const auto payload_symbols = (payload_size + 1) / 2;
            const auto shard_symbols_ceil = (payload_symbols + k_ - 1) / k_;
            const auto shard_bytes = shard_symbols_ceil * 2;
            return shard_bytes;
        }

    private:
        ReedSolomon(size_t n, size_t k, size_t wanted_n, PolyEncoder &&poly_enc)
                : n_(n), k_(k), wanted_n_(wanted_n), poly_enc_(std::move(poly_enc)) { }

        const size_t n_;
        const size_t k_;
        const size_t wanted_n_;
        PolyEncoder poly_enc_;
    };

}

#endif //NOVELPOLY_REED_SOLOMON_CRUST_REED_SOLOMON_HPP
