//
// Created by iceseer on 5/11/23.
//

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_POLY_ENCODER_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_POLY_ENCODER_HPP

#include <array>
#include <assert.h>
#include <cstdint>
#include <optional>
#include <stdlib.h>
#include <tuple>
#include <vector>

#include "additive_fft.hpp"
#include "errors.hpp"
#include "math.hpp"
#include "types.hpp"
#include "walsh.hpp"

namespace ec_cpp {

template <typename TDescriptor> struct PolyEncoder final {
  using Descriptor = TDescriptor;
  PolyEncoder(const Descriptor &descriptor) : descriptor_{descriptor} {}

  struct Additive {
    typename Descriptor::Elt _0;

    typename Descriptor::Multiplier
    toMultiplier(const typename Descriptor::Tables &tables) const {
      const auto &log_table = std::get<0>(tables);
      return typename Descriptor::Multiplier(log_table[size_t(_0)]);
    }

    Additive mul(typename Descriptor::Multiplier other,
                 const typename Descriptor::Tables &tables) {
      if (_0 == typename Descriptor::Elt(0))
        return Additive{0};

      const auto &[log_table, exp_table, _] = tables;
      const auto log = (typename Descriptor::Wide(log_table[size_t(_0)])) +
                       typename Descriptor::Wide(other);
      const auto offset =
          (log & typename Descriptor::Wide(Descriptor::kOneMask)) +
          (log >> Descriptor::kFieldBits);
      return Additive{exp_table[size_t(offset)]};
    }

    void mulAssignSlice(Additive *selfy, size_t count,
                        typename Descriptor::Multiplier other,
                        const typename Descriptor::Tables &tables) {
      for (size_t ix = 0ull; ix < count; ++ix)
        selfy[ix] = selfy[ix].mul(other, tables);
    }
  };

  struct AdditiveFFT {
    typename Descriptor::Multiplier skews[size_t(Descriptor::kOneMask)];
    typename Descriptor::Multiplier B[Descriptor::kFieldSize >> 1ull];

    static AdditiveFFT initalize(const typename Descriptor::Tables &tables) {
      typename Descriptor::Elt base[Descriptor::kFieldBits - 1ull] = {0};
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
            skews_additive[j + s] = Additive{
                typename Descriptor::Elt(skews_additive[j]._0 ^ base[i])};
            j += step;
          }
        }

        const auto idx = Additive{base[m]}.mul(
            Additive{
                typename Descriptor::Elt(base[m] ^ typename Descriptor::Elt(1))}
                .toMultiplier(tables),
            tables);
        base[m] = Descriptor::kOneMask - idx.toMultiplier(tables);
        for (size_t i = (m + 1ull); i < (Descriptor::kFieldBits - 1ull); ++i) {
          const auto b =
              (Additive{typename Descriptor::Elt(base[i] ^ 1)}.toMultiplier(
                   tables) +
               typename Descriptor::Wide(base[m])) %
              typename Descriptor::Wide(Descriptor::kOneMask);
          base[i] = Additive{base[i]}
                        .mul(typename Descriptor::Multiplier(b), tables)
                        ._0;
        }
      }

      AdditiveFFT result;
      for (size_t i = 0ull; i < size_t(Descriptor::kOneMask); ++i)
        result.skews[i] = skews_additive[i].toMultiplier(tables);

      base[0] = Descriptor::kOneMask - base[0];
      for (size_t i = 1ull; i < (Descriptor::kFieldBits - 1ull); ++i)
        base[i] = typename Descriptor::Elt(
            (typename Descriptor::Wide(Descriptor::kOneMask) -
             typename Descriptor::Wide(base[i]) +
             typename Descriptor::Wide(base[i - 1])) %
            typename Descriptor::Wide(Descriptor::kOneMask));

      result.B[0] = typename Descriptor::Multiplier(0);
      for (size_t i = 0ull; i < (Descriptor::kFieldBits - 1); ++i) {
        const auto depart = 1ull << i;
        for (size_t j = 0ull; j < depart; ++j)
          result.B[j + depart] = typename Descriptor::Multiplier(
              (typename Descriptor::Wide(result.B[j]) +
               typename Descriptor::Wide(base[i])) %
              typename Descriptor::Wide(Descriptor::kOneMask));
      }
      return result;
    }

    void inverse_afft(Additive *data, size_t size, size_t index,
                      const typename Descriptor::Tables &tables) const {
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

    void afft(Additive *data, size_t size, size_t index,
              const typename Descriptor::Tables &tables) const {
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

  Result<bool> encodeSub(std::vector<Additive> &codeword, Slice<uint8_t> bytes,
                         size_t n, size_t k) const {
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
    thread_local std::vector<Additive> data;
    data.clear();
    data.reserve((bytes.size() + 1) / sizeof(typename Descriptor::Elt) +
                 zero_bytes_to_add / sizeof(typename Descriptor::Elt));

    const auto *current = &bytes[0];
    const auto *end = &bytes[bytes.size()];
    while (end - current >= sizeof(typename Descriptor::Elt)) {
      data.emplace_back(Additive{Descriptor::fromBEBytes(current)});
      current += sizeof(typename Descriptor::Elt);
    }
    if (end != current) {
      uint8_t b[sizeof(typename Descriptor::Elt)] = {0};
      memcpy(b, current, (end - current) * sizeof(current[0]));

      zero_bytes_to_add -=
          (sizeof(typename Descriptor::Elt) - size_t(end - current));
      data.emplace_back(Additive{Descriptor::fromBEBytes(b)});
    }
    assert((zero_bytes_to_add % sizeof(typename Descriptor::Elt)) == 0ull);
    data.insert(data.end(),
                zero_bytes_to_add / sizeof(typename Descriptor::Elt),
                Additive{0ull});

    const auto l_0 = data.size();
    assert(l_0 == n);

    codeword.assign(data.begin(), data.end());
    assert(codeword.size() == n);

    encodeLow(data, k, codeword, n);
    return true;
  }

  /// [101...001] erasures are bit-array representation, where 1 - is empty and
  /// 0 - is full.
  template <typename Shard>
  void eval_error_polynomial(const std::vector<Shard> &erasure,
                             std::array<typename TDescriptor::Multiplier,
                                        TDescriptor::kFieldSize> &log_walsh2,
                             size_t n) const {
    const auto z = std::min(n, erasure.size());
    for (size_t i = 0; i < z; ++i)
      log_walsh2[i] = typename Descriptor::Multiplier(erasure[i].empty());

    /*memset()
    for (size_t i = z; i < n; ++i) {
        log_walsh2[i] = typename Descriptor::Multiplier(0);
    }*/
    walsh<Descriptor>(log_walsh2);
    const auto &[_, __, log_walsh] = descriptor_.kTables;
    for (size_t i = 0; i < n; ++i) {
      const auto tmp = typename Descriptor::Wide(log_walsh2[i]) *
                       typename Descriptor::Wide(log_walsh[i]);
      log_walsh2[i] = typename Descriptor::Multiplier(
          tmp % (typename Descriptor::Wide(Descriptor::kOneMask)));
    }
    walsh<Descriptor>(log_walsh2);
    for (size_t i = 0; i < z; ++i)
      if (erasure[i].empty())
        log_walsh2[i] = typename Descriptor::Multiplier(Descriptor::kOneMask) -
                        log_walsh2[i];
  }

  template <typename Shard>
  Result<bool>
  reconstruct_sub(std::vector<uint8_t> &recovered_bytes,
                  const std::vector<std::optional<Additive>> &codewords,
                  const std::vector<Shard> &erasures, size_t n, size_t k,
                  const std::array<typename Descriptor::Multiplier,
                                   Descriptor::kFieldSize> &error_poly) const {
    assert(math::isPowerOf2(n));
    assert(math::isPowerOf2(k));
    assert(codewords.size() == n);
    assert(k <= n / 2);

    const auto recover_up_to = k;
    std::vector<Additive> recovered;
    recovered.assign(recover_up_to, Additive{0});

    /// TODO(iceseer): try to remove
    thread_local std::vector<Additive> codeword;
    codeword.clear();
    codeword.reserve(codewords.size());
    for (size_t idx = 0ull; idx < codewords.size(); ++idx) {
      const auto value = codewords[idx] ? *codewords[idx] : Additive{0};
      if (idx < recovered.size()) {
        recovered[idx] = value;
      }
      codeword.emplace_back(value);
    }

    assert(codeword.size() == n);
    decode_main(codeword, recover_up_to, erasures, error_poly, n);

    for (size_t idx = 0ull; idx < recover_up_to; ++idx)
      if (erasures[idx].empty())
        recovered[idx] = codeword[idx];

    const auto was = recovered_bytes.size();
    recovered_bytes.resize(was +
                           recover_up_to * sizeof(typename Descriptor::Elt));

    for (size_t i = 0; i < k; ++i)
      Descriptor::toBEBytes(
          &recovered_bytes[was + i * sizeof(typename Descriptor::Elt)],
          recovered[i]._0);

    return true;
  }

private:
  const Descriptor &descriptor_;
  const AdditiveFFT AFFT{AdditiveFFT::initalize(descriptor_.kTables)};

  template <typename Shard>
  void decode_main(std::vector<Additive> &codeword, size_t recover_up_to,
                   const std::vector<Shard> &erasure,
                   const std::array<typename Descriptor::Multiplier,
                                    Descriptor::kFieldSize> &log_walsh2,
                   size_t n) const {
    assert(codeword.size() == n);
    assert(n >= recover_up_to);
    assert(erasure.size() == n);

    for (size_t i = 0ull; i < n; ++i)
      codeword[i] = erasure[i].empty()
                        ? Additive{0}
                        : codeword[i].mul(log_walsh2[i], descriptor_.kTables);

    AFFT.inverse_afft(codeword.data(), n, 0, descriptor_.kTables);
    tweaked_formal_derivative(codeword, n);

    AFFT.afft(codeword.data(), n, 0, descriptor_.kTables);

    for (size_t i = 0ull; i < recover_up_to; ++i)
      codeword[i] = erasure[i].empty()
                        ? codeword[i].mul(log_walsh2[i], descriptor_.kTables)
                        : Additive{0};
  }

  void tweaked_formal_derivative(std::vector<Additive> &codeword,
                                 size_t n) const {
    formal_derivative(codeword, n);
  }

  void formal_derivative(std::vector<Additive> &cos, size_t size) const {
    auto swallow = [&](size_t j, size_t offset) {
      const auto index = j + offset;
      const auto in_range = index < cos.size();
      cos[j]._0 =
          cos[j]._0 ^ (in_range ? cos[index]._0 : typename Descriptor::Elt(0));
    };

    for (size_t i = 1ull; i < size; ++i) {
      const auto length = ((i ^ (i - 1ull)) + 1ull) >> 1ull;
      for (size_t j = (i - length); j < i; ++j)
        swallow(j, length);
    }
    auto i = size;
    while (i < Descriptor::kFieldSize && i < cos.size()) {
      for (size_t j = 0ull; j < size; ++j)
        swallow(j, i);
      i = (i << 1ull);
    }
  }

  void encodeLow(const std::vector<Additive> &data, size_t k,
                 std::vector<Additive> &codeword, size_t n) const {
    assert(k + k <= n);
    assert(codeword.size() == n);
    assert(data.size() == n);
    assert(math::isPowerOf2(n));
    assert(math::isPowerOf2(k));
    assert((n / k) * k == n);

    auto *codeword_first_k = codeword.data();
    auto *codeword_skip_first_k = &codeword[k];

    AFFT.inverse_afft(codeword_first_k, k, 0, descriptor_.kTables);
    for (size_t shift = k; shift < n; shift += k) {
      auto *codeword_at_shift = &codeword_skip_first_k[(shift - k)];
      [[maybe_unused]] auto *codeword_at_shift_end =
          &codeword_skip_first_k[shift];

      memcpy(codeword_at_shift, codeword_first_k,
             k * sizeof(codeword_first_k[0]));
      AFFT.afft(codeword_at_shift, k, shift, descriptor_.kTables);
    }

    memcpy(&codeword[0], &data[0], k * sizeof(data[0]));
  }
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_POLY_ENCODER_HPP
