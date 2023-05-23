/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ec-cpp.hpp"

namespace ec_cpp {

f2e16_Descriptor field_descriptor;
PolyEncoder_f2e16 poly_encoder(field_descriptor);

constexpr size_t kMaxValidators = f2e16_Descriptor::kFieldSize;

Result<size_t> recoveryThreshold(size_t n_validators) {
  if (n_validators > kMaxValidators)
    return Error::kTooManyValidators;

  if (n_validators <= 1ull)
    return Error::kNotEnoughValidators;

  const auto needed = math::sat_sub_unsigned(n_validators, size_t(1ull)) / 3ull;
  return (needed + 1ull);
}

Result<ReedSolomon<PolyEncoder_f2e16>> create(size_t n_validators) {
  const auto n_wanted = n_validators;
  auto k_wanted_result = recoveryThreshold(n_wanted);
  if (resultHasError(k_wanted_result))
    return resultGetError(std::move(k_wanted_result));

  if (n_wanted > kMaxValidators)
    return Error::kTooManyValidators;

  return ReedSolomon<ec_cpp::PolyEncoder_f2e16>::create(
      n_wanted, resultGetValue(std::move(k_wanted_result)), poly_encoder);
}

} // namespace ec_cpp
