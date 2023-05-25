/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP

#include "errors.hpp"
#include "f2e16.hpp"
#include "reed-solomon.hpp"

namespace ec_cpp {

using PolyEncoder_f2e16 = PolyEncoder<f2e16_Descriptor>;

/// Creates erasure-coding core.
/// @param n_validators determines the number of validators to shard data for
///
Result<ReedSolomon<PolyEncoder_f2e16>> create(size_t n_validators);

/// Obtain a threshold of chunks that should be enough to recover the data.
/// @param n_validators determines the number of validators to shard data for
/// @return recovery threshold value
///
Result<size_t> getRecoveryThreshold(size_t n_validators);

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
