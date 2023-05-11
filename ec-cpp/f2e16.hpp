//
// Created by iceseer on 5/4/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP

#include <array>
#include <assert.h>
#include <cstdint>
#include <stdlib.h>
#include <tuple>
#include <vector>

#include "additive_fft.hpp"
#include "errors.hpp"
#include "math.hpp"
#include "types.hpp"

namespace ec_cpp {

struct f2e16_Descriptor {
  using Elt = uint16_t;
  using Wide = uint32_t;
  using Multiplier = Elt;

  static constexpr size_t kFieldBits = 16ull;
  static constexpr size_t kFieldSize = (1ull << kFieldBits);

  static constexpr Elt kGenerator = 0x2D;
  static constexpr Elt kOneMask = Elt(kFieldSize - 1ull);
  static constexpr Elt kBase[kFieldBits] = {
      1,     44234, 15374, 5694,  50562, 60718, 37196, 16402,
      27800, 4312,  27250, 47360, 64952, 64308, 65336, 39198};

  static Elt fromBEBytes(const uint8_t *data) {
    return Elt(Elt(Elt(*data) << 8) | Elt(*(data + 1)));
  }

  static void toBEBytes(uint8_t *dst, Elt src) {
    dst[0] = (src >> 8ull);
    dst[1] = (src & 0xff);
  }
};

template <typename TDescriptor>
constexpr void walsh(std::array<typename TDescriptor::Multiplier,
                                TDescriptor::kFieldSize> &data) {
  const auto size = data.size();
  size_t depart_no = 1ull;

  while (depart_no < size) {
    size_t j = 0ull;
    const auto depart_no_next = (depart_no << 1ull);
    while (j < size) {
      for (size_t i = j; i < (depart_no + j); ++i) {
        const auto mask = typename TDescriptor::Wide(TDescriptor::kOneMask);
        const typename TDescriptor::Wide tmp2 =
            typename TDescriptor::Wide(data[i]) + mask -
            typename TDescriptor::Wide(data[i + depart_no]);
        const typename TDescriptor::Wide tmp1 =
            typename TDescriptor::Wide(data[i]) +
            typename TDescriptor::Wide(data[i + depart_no]);
        data[i] = typename TDescriptor::Multiplier(
            (tmp1 & mask) + (tmp1 >> TDescriptor::kFieldBits));
        data[i + depart_no] = typename TDescriptor::Multiplier(
            (tmp2 & mask) + (tmp2 >> TDescriptor::kFieldBits));
      }
      j += depart_no_next;
    }
    depart_no = depart_no_next;
  }
}

struct PolyEncoder_f2e16 final {
  using Descriptor = f2e16_Descriptor;
  using Tables =
      std::tuple<std::array<Descriptor::Elt, Descriptor::kFieldSize>,
                 std::array<Descriptor::Elt, Descriptor::kFieldSize>,
                 std::array<Descriptor::Multiplier, Descriptor::kFieldSize>>;

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

    std::array<Descriptor::Multiplier, Descriptor::kFieldSize> log_walsh{
        log_table};
    log_walsh[0] = 0;
    walsh<Descriptor>(log_walsh);

    return std::make_tuple(std::move(log_table), std::move(exp_table),
                           std::move(log_walsh));
  }();

  struct Additive {
    Descriptor::Elt _0;

    Descriptor::Multiplier toMultiplier(const Tables &tables) const {
      const auto &log_table = std::get<0>(tables);
      return Descriptor::Multiplier(log_table[size_t(_0)]);
    }

    Additive mul(Descriptor::Multiplier other, const Tables &tables) {
      if (_0 == Descriptor::Elt(0))
        return Additive{0};

      const auto &[log_table, exp_table, _] = tables;
      const auto log =
          (Descriptor::Wide(log_table[size_t(_0)])) + Descriptor::Wide(other);
      const auto offset = (log & Descriptor::Wide(Descriptor::kOneMask)) +
                          (log >> Descriptor::kFieldBits);
      return Additive{exp_table[size_t(offset)]};
    }

    void mulAssignSlice(Additive *selfy, size_t count,
                        Descriptor::Multiplier other, const Tables &tables) {
      for (size_t ix = 0ull; ix < count; ++ix)
        selfy[ix] = selfy[ix].mul(other, tables);
    }
  };

  struct AdditiveFFT {
    Descriptor::Multiplier skews[size_t(Descriptor::kOneMask)];
    Descriptor::Multiplier B[Descriptor::kFieldSize >> 1ull];

    static AdditiveFFT initalize(const Tables &tables) {
      Descriptor::Elt base[Descriptor::kFieldBits - 1ull] = {0};
      Additive skews_additive[Descriptor::kOneMask] = {0};

      for (size_t i = 1; i < Descriptor::kFieldBits; ++i)
        base[i - 1] = 1 << i;

      for (size_t m = 0ull; m < (Descriptor::kFieldBits - 1); ++m) {
        const auto step = 1ull << (m + 1ull);
        skews_additive[(1ull << m) - 1ull] = Additive{0};
        for (size_t i = m; i < (Descriptor::kFieldBits - 1); ++i) {
          const auto s = 1ull << (i + 1ull);
          auto j = (1ull << m) - 1ull;
          while (j < s) {
            skews_additive[j + s] =
                Additive{Descriptor::Elt(skews_additive[j]._0 ^ base[i])};
            j += step;
          }
        }

        const auto idx = Additive{base[m]}.mul(
            Additive{Descriptor::Elt(base[m] ^ Descriptor::Elt(1))}
                .toMultiplier(tables),
            tables);
        base[m] = Descriptor::kOneMask - idx.toMultiplier(tables);
        for (size_t i = (m + 1ull); i < (Descriptor::kFieldBits - 1ull); ++i) {
          const auto b =
              (Additive{Descriptor::Elt(base[i] ^ 1)}.toMultiplier(tables) +
               Descriptor::Wide(base[m])) %
              Descriptor::Wide(Descriptor::kOneMask);
          base[i] = Additive{base[i]}.mul(Descriptor::Multiplier(b), tables)._0;
        }
      }

      AdditiveFFT result;
      for (size_t i = 0ull; i < size_t(Descriptor::kOneMask); ++i)
        result.skews[i] = skews_additive[i].toMultiplier(tables);

      base[0] = Descriptor::kOneMask - base[0];
      for (size_t i = 1ull; i < (Descriptor::kFieldBits - 1ull); ++i)
        base[i] = Descriptor::Elt((Descriptor::Wide(Descriptor::kOneMask) -
                                   Descriptor::Wide(base[i]) +
                                   Descriptor::Wide(base[i - 1])) %
                                  Descriptor::Wide(Descriptor::kOneMask));

      result.B[0] = Descriptor::Multiplier(0);
      for (size_t i = 0ull; i < (Descriptor::kFieldBits - 1); ++i) {
        const auto depart = 1ull << i;
        for (size_t j = 0ull; j < depart; ++j)
          result.B[j + depart] = Descriptor::Multiplier(
              (Descriptor::Wide(result.B[j]) + Descriptor::Wide(base[i])) %
              Descriptor::Wide(Descriptor::kOneMask));
      }
      return result;
    }

    void inverse_afft(Additive *data, size_t size, size_t index,
                      const Tables &tables) {
      size_t depart_no(1ull);
      while (depart_no < size) {
        size_t j(depart_no);
        while (j < size) {
          for (size_t i = (j - depart_no); i < j; ++i)
            data[i + depart_no]._0 = (data[i + depart_no]._0 ^ data[i]._0);

          const auto skew = skews[j + index - 1ull];
          if (skew != Descriptor::kOneMask)
            for (size_t i = (j - depart_no); i < j; ++i)
              data[i]._0 =
                  (data[i]._0 ^ data[i + depart_no].mul(skew, tables)._0);

          j += (depart_no << 1ull);
        }
        depart_no = (depart_no << 1ull);
      }
    }

    void afft(Additive *data, size_t size, size_t index, const Tables &tables) {
      size_t depart_no(size >> 1ull);
      while (depart_no > 0) {
        size_t j(depart_no);
        while (j < size) {
          const auto skew = skews[j + index - 1ull];
          if (skew != Descriptor::kOneMask)
            for (size_t i = (j - depart_no); i < j; ++i)
              data[i]._0 =
                  data[i]._0 ^ data[i + depart_no].mul(skew, tables)._0;

          for (size_t i = (j - depart_no); i < j; ++i)
            data[i + depart_no]._0 = data[i + depart_no]._0 ^ data[i]._0;

          j += (depart_no << 1ull);
        }
        depart_no = (depart_no >> 1ull);
      }
    }
  };

  Result<std::vector<Additive>> encodeSub(Slice<uint8_t> bytes, size_t n,
                                          size_t k) {
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

    auto zero_bytes_to_add = n * 2 - dl;
    std::vector<Additive> data;
    data.reserve((bytes.size() + 1) / sizeof(Descriptor::Elt) +
                 zero_bytes_to_add / sizeof(Descriptor::Elt));

    const auto *current = &bytes[0];
    const auto *end = &bytes[bytes.size()];
    while (end - current >= sizeof(Descriptor::Elt)) {
      data.emplace_back(Additive{Descriptor::fromBEBytes(current)});
      current += sizeof(Descriptor::Elt);
    }
    if (end != current) {
      uint8_t b[sizeof(Descriptor::Elt)] = {0};
      memcpy(b, current, (end - current) * sizeof(current[0]));

      zero_bytes_to_add -= (sizeof(Descriptor::Elt) - size_t(end - current));
      data.emplace_back(Additive{Descriptor::fromBEBytes(b)});
    }
    assert((zero_bytes_to_add % sizeof(Descriptor::Elt)) == 0ull);
    data.insert(data.end(), zero_bytes_to_add / sizeof(Descriptor::Elt),
                Additive{0ull});

    const auto l_0 = data.size();
    assert(l_0 == n);

    auto codeword = data;
    assert(codeword.size() == n);

    encodeLow(data, k, codeword, n);
    return codeword;
  }

private:
  AdditiveFFT AFFT{AdditiveFFT::initalize(kTables)};

  void encodeLow(const std::vector<Additive> &data, size_t k,
                 std::vector<Additive> &codeword, size_t n) {
    assert(k + k <= n);
    assert(codeword.size() == n);
    assert(data.size() == n);
    assert(math::isPowerOf2(n));
    assert(math::isPowerOf2(k));
    assert((n / k) * k == n);

    /// TODO(iceseer): try to remove
    codeword.assign(data.begin(), data.end());
    auto *codeword_first_k = codeword.data();
    auto *codeword_skip_first_k = &codeword[k];

    AFFT.inverse_afft(codeword_first_k, k, 0, kTables);
    for (size_t shift = k; shift < n; shift += k) {
      auto *codeword_at_shift = &codeword_skip_first_k[(shift - k)];
      [[maybe_unused]] auto *codeword_at_shift_end =
          &codeword_skip_first_k[shift];

      memcpy(codeword_at_shift, codeword_first_k,
             k * sizeof(codeword_first_k[0]));
      AFFT.afft(codeword_at_shift, k, shift, kTables);
    }

    memcpy(&codeword[0], &data[0], k * sizeof(data[0]));
  }
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_F2E16_HPP
