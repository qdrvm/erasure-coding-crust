/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_TYPES_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_TYPES_HPP

#include <string_view>

namespace ec_cpp {

template <typename T>
using Slice = std::basic_string_view<std::remove_reference_t<T>>;

}

#endif // NOVELPOLY_REED_SOLOMON_CRUST_TYPES_HPP
