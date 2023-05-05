//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP

#include <type_traits>
#include <assert.h>
#include <stdlib.h>

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

}

#endif //NOVELPOLY_REED_SOLOMON_CRUST_MATH_HPP
