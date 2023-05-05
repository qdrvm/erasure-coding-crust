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
    };

    template<typename T>
    using Result = std::variant<T, Error>;
}

#endif //NOVELPOLY_REED_SOLOMON_CRUST_ERRORS_HPP
