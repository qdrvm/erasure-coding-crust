//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP

#include <cstdint>
#include <stdlib.h>
#include <array>
#include <tuple>
#include <vector>
#include <assert.h>

#include "types.hpp"
#include "errors.hpp"
#include "math.hpp"

namespace ec_cpp {

    struct f2e16_Descriptor {
        using Elt = uint16_t;
        using Wide = uint32_t;
        using Multiplier = Elt;

        static constexpr size_t kFieldBits = 16ull;
        static constexpr size_t kFieldSize = (1ull << kFieldBits);

        static constexpr Elt kGenerator = 0x2D;
        static constexpr Elt kOneMask = Elt(kFieldSize - 1ull);
        static constexpr Elt kBase[kFieldBits] = {1, 44234, 15374, 5694, 50562, 60718, 37196, 16402, 27800, 4312, 27250,
                                                  47360, 64952, 64308, 65336, 39198};
    };

    template<typename TDescriptor>
    constexpr void walsh(std::array<typename TDescriptor::Multiplier, TDescriptor::kFieldSize> &data) {
        const auto size = data.size();
        size_t depart_no = 1ull;

        while (depart_no < size) {
            size_t j = 0ull;
            const auto depart_no_next = (depart_no << 1ull);
            while (j < size) {
                for (size_t i = j; i < (depart_no + j); ++i) {
                    const auto mask = typename TDescriptor::Wide(TDescriptor::kOneMask);
                    const typename TDescriptor::Wide tmp2 = typename TDescriptor::Wide(data[i]) + mask - typename TDescriptor::Wide(data[i + depart_no]);
                    const typename TDescriptor::Wide tmp1 = typename TDescriptor::Wide(data[i]) + typename TDescriptor::Wide(data[i + depart_no]);
                    data[i] = typename TDescriptor::Multiplier((tmp1 & mask) + (tmp1 >> TDescriptor::kFieldBits));
                    data[i + depart_no] = typename TDescriptor::Multiplier((tmp2 & mask) + (tmp2 >> TDescriptor::kFieldBits));
                }
                j += depart_no_next;
            }
            depart_no = depart_no_next;
        }
    }

    struct PolyEncoder_f2e16 final {
        using Descriptor = f2e16_Descriptor;
        using Tables = std::tuple<std::array<Descriptor::Elt, Descriptor::kFieldSize>, std::array<Descriptor::Elt, Descriptor::kFieldSize>, std::array<Descriptor::Multiplier, Descriptor::kFieldSize>>;  

        /**
         * kLogTable = 0
         * kExpTable = 1
         * kLogWalsh = 2
         */
        const Tables kTables = []() {
            std::array<Descriptor::Elt, Descriptor::kFieldSize> log_table = {0};
            std::array<Descriptor::Elt, Descriptor::kFieldSize> exp_table = {0};

            const Descriptor::Elt mas = (1 << (Descriptor::kFieldBits - 1)) - 1;
            size_t state = 1ull;
            for (size_t i = 0ull; i < size_t(Descriptor::kOneMask); ++i) {
                exp_table[state] = Descriptor::Elt(i);
                if ((state >> (Descriptor::kFieldBits - 1)) != 0) {
                    state &= size_t(mas);
                    state = (state << 1ull) ^ size_t(Descriptor::kGenerator);
                } else
                    state = (state << 1ull);
            }

            exp_table[0] = Descriptor::kOneMask;
            log_table[0] = 0;

            for (size_t i = 0ull; i < Descriptor::kFieldBits; ++i)
                for (size_t j = 0ull; j < (1ull << i); ++j)
                    log_table[j + (1ull << i)] = log_table[j] ^ Descriptor::kBase[i];

            for (size_t i = 0ull; i < Descriptor::kFieldSize; ++i)
                log_table[i] = exp_table[size_t(log_table[i])];

            for (size_t i = 0ull; i < Descriptor::kFieldSize; ++i)
                exp_table[size_t(log_table[i])] = Descriptor::Elt(i);

            exp_table[size_t(Descriptor::kOneMask)] = exp_table[0];

            std::array<Descriptor::Multiplier, Descriptor::kFieldSize> log_walsh{log_table};
            log_walsh[0] = 0;
            walsh<Descriptor>(log_walsh);

            return std::make_tuple(
                    std::move(log_table),
                    std::move(exp_table),
                    std::move(log_walsh)
            );
        }();

        struct Additive {
            Descriptor::Elt _0;

            Descriptor::Multiplier toMultiplier(const Tables &tables) {
                const auto &log_table = std::get<0>(tables);
                return Descriptor::Multiplier(log_table[size_t(_0)]);
            }

            Additive mul(Descriptor::Multiplier other, const Tables &tables) {
                if (_0 == Descriptor::Elt(0))
                    return Additive{0};
                
                const auto &[log_table, exp_table, _] = tables;
                const auto log = (Descriptor::Wide(log_table[size_t(_0)])) + Descriptor::Wide(other);
                const auto offset = (log & Descriptor::Wide(Descriptor::kOneMask)) + (log >> Descriptor::kFieldBits);
                return Additive{exp_table[size_t(offset)]};
            }

            void mulAssignSlice(Additive *selfy, size_t count, Descriptor::Multiplier other, const Tables &tables) {
                for (size_t ix = 0ull; ix < count; ++ix)
                    selfy[ix] = selfy[ix].mul(other, tables);
            }

        };

        Result<std::vector<Additive>> encodeSub(Slice<uint8_t> bytes, size_t n, size_t k) {
            assert(math::isPowerOf2(n));
            assert(math::isPowerOf2(k));
            assert(bytes.size() <= (k << 1));
            assert(k <= n / 2);

            const auto dl = bytes.size();

            const auto l = [dl] {
                if (math::isPowerOf2(dl))
                    return dl;
                else {
                    const size_t l = (1ull << math::log2(dl));
                    if (l >= dl)
                        return l; 
                    return (l << 1ull);
                }
            }();
            assert(math::isPowerOf2(l));
            assert(l >= dl);

            
        }
    };

}

#endif //NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
