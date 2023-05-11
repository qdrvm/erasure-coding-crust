//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP

#include "errors.hpp"
#include "f2e16.hpp"
#include "reed-solomon.hpp"

namespace ec_cpp {

using PolyEncoder_f2e16 = PolyEncoder<f2e16_Descriptor>;

Result<ReedSolomon<PolyEncoder_f2e16>> create(size_t n_validators);

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
