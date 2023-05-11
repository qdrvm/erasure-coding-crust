//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_ERRORS_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_ERRORS_HPP

#include <variant>

namespace ec_cpp {

enum struct Error {
  kArgsMustBePowOf2,
  kWantedShardCountTooLow,
  kWantedShardCountTooHigh,
  kWantedPayloadShardCountTooLow,
  kPayloadSizeIsZero,
  kTooManyValidators,
  kNotEnoughValidators,
};

template <typename T> using Result = std::variant<T, Error>;

template <typename T> bool resultHasError(const Result<T> &r) {
  return (r.index() == 1ull);
}

template <typename T> Error resultGetError(Result<T> &&r) {
  return std::get<Error>(std::move(r));
}

template <typename T> T resultGetValue(Result<T> &&r) {
  return std::get<T>(std::move(r));
}
} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_ERRORS_HPP
