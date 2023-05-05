//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP

#include "reed-solomon.hpp"
#include "f2e16.hpp"

namespace ec_cpp {

    void create() {
        auto result = ReedSolomon<ec_cpp::PolyEncoder_f2e16>::create(8, 4, ec_cpp::PolyEncoder_f2e16{});
    }

}

#endif //NOVELPOLY_REED_SOLOMON_CRUST_EC_CPP_HPP
