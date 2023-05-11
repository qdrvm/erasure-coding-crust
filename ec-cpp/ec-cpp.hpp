//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP

#include "errors.hpp"
#include "f2e16.hpp"
#include "reed-solomon.hpp"

namespace ec_cpp {

constexpr size_t kMaxValidators =
    ec_cpp::PolyEncoder_f2e16::Descriptor::kFieldSize;

inline Result<size_t> recoveryThreshold(size_t n_validators) {
  if (n_validators > kMaxValidators)
    return Error::kTooManyValidators;

  if (n_validators <= 1ull)
    return Error::kNotEnoughValidators;

  const auto needed = math::sat_sub_unsigned(n_validators, size_t(1ull)) / 3ull;
  return (needed + 1ull);
}

Result<ReedSolomon<ec_cpp::PolyEncoder_f2e16>> create(size_t n_validators) {
  const auto n_wanted = n_validators;
  auto k_wanted_result = recoveryThreshold(n_wanted);
  if (resultHasError(k_wanted_result))
    return resultGetError(std::move(k_wanted_result));

  if (n_wanted > kMaxValidators)
    return Error::kTooManyValidators;

  return ReedSolomon<ec_cpp::PolyEncoder_f2e16>::create(
      n_wanted, resultGetValue(std::move(k_wanted_result)),
      ec_cpp::PolyEncoder_f2e16{});
}

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
