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
#include <cstring>

#include "additive_fft.hpp"
#include "errors.hpp"
#include "math.hpp"
#include "types.hpp"
#include "walsh.hpp"

namespace ec_cpp {

template <typename TDescriptor> struct PolyEncoder final {
  using Descriptor = TDescriptor;
  PolyEncoder(const Descriptor &descriptor) : descriptor_{descriptor} {}

  Result<bool> encodeSub(std::vector<Additive<Descriptor>> &codeword,
                         Slice<uint8_t> bytes, size_t n, size_t k) const {
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
    local().clear();
    local().reserve((bytes.size() + 1) / sizeof(typename Descriptor::Elt) +
                    zero_bytes_to_add / sizeof(typename Descriptor::Elt));

    const auto *current = &bytes[0];
    const auto *end = &bytes[bytes.size()];
    while (end - current >= sizeof(typename Descriptor::Elt)) {
      local().emplace_back(
          Additive<Descriptor>{Descriptor::fromBEBytes(current)});
      current += sizeof(typename Descriptor::Elt);
    }
    if (end != current) {
      uint8_t b[sizeof(typename Descriptor::Elt)] = {0};
      memcpy(b, current, (end - current) * sizeof(current[0]));

      zero_bytes_to_add -=
          (sizeof(typename Descriptor::Elt) - size_t(end - current));
      local().emplace_back(Additive<Descriptor>{Descriptor::fromBEBytes(b)});
    }
    assert((zero_bytes_to_add % sizeof(typename Descriptor::Elt)) == 0ull);
    local().insert(local().end(),
                   zero_bytes_to_add / sizeof(typename Descriptor::Elt),
                   Additive<Descriptor>{0ull});

    const auto l_0 = local().size();
    assert(l_0 == n);

    codeword.assign(local().begin(), local().end());
    assert(codeword.size() == n);

    encodeLow(local(), k, codeword, n);
    return true;
  }

  /// [101...001] erasures are bit-array representation, where 1 - is empty and
  /// 0 - is full.
  template <typename Shard>
  void evalErrorPolynomial(const std::vector<Shard> &erasure, size_t gap,
                           std::array<typename TDescriptor::Multiplier,
                                      TDescriptor::kFieldSize> &log_walsh2,
                           size_t n) const {
    auto is_erasured = [&](size_t i) -> bool {
      return i >= erasure.size() || erasure[i].empty();
    };

    const auto z = std::min(n, erasure.size() + gap);
    for (size_t i = 0; i < z; ++i)
      log_walsh2[i] = typename Descriptor::Multiplier(is_erasured(i));

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
      if (is_erasured(i))
        log_walsh2[i] = typename Descriptor::Multiplier(Descriptor::kOneMask) -
                        log_walsh2[i];
  }

  template <typename Shard>
  Result<bool> reconstructSub(
      std::vector<uint8_t> &recovered_bytes,
      std::vector<Additive<Descriptor>> &codeword,
      const std::vector<Shard> &erasures, size_t gap, size_t n, size_t k,
      const std::array<typename Descriptor::Multiplier, Descriptor::kFieldSize>
          &error_poly) const {
    assert(math::isPowerOf2(n));
    assert(math::isPowerOf2(k));
    assert(codeword.size() + gap == n);
    assert(k <= n / 2);

    const auto recover_up_to = k;
    local().assign(recover_up_to, Additive<Descriptor>{0});

    for (size_t idx = 0ull; idx < codeword.size(); ++idx)
      if (idx < local().size())
        local()[idx] = codeword[idx];

    decode_main(codeword, recover_up_to, erasures, gap, error_poly, n);

    for (size_t idx = 0ull; idx < recover_up_to; ++idx)
      if (idx >= erasures.size() || erasures[idx].empty())
        local()[idx] = codeword[idx];

    const auto was = recovered_bytes.size();
    recovered_bytes.resize(was +
                           recover_up_to * sizeof(typename Descriptor::Elt));

    for (size_t i = 0; i < k; ++i)
      Descriptor::toBEBytes(
          &recovered_bytes[was + i * sizeof(typename Descriptor::Elt)],
          local()[i]._0);

    return true;
  }

private:
  const Descriptor &descriptor_;
  const AdditiveFFT<Descriptor> AFFT{
      AdditiveFFT<Descriptor>::initalize(descriptor_.kTables)};

  std::vector<Additive<Descriptor>> &local() const {
    thread_local std::vector<Additive<Descriptor>> data;
    return data;
  }

  template <typename Shard>
  void decode_main(std::vector<Additive<Descriptor>> &codeword,
                   size_t recover_up_to, const std::vector<Shard> &erasure,
                   size_t gap,
                   const std::array<typename Descriptor::Multiplier,
                                    Descriptor::kFieldSize> &log_walsh2,
                   size_t n) const {
    assert(codeword.size() + gap == n);
    assert(n >= recover_up_to);
    assert(erasure.size() + gap == n);

    for (size_t i = 0ull; i < codeword.size(); ++i)
      codeword[i] = erasure[i].empty()
                        ? Additive<Descriptor>{0}
                        : codeword[i].mul(log_walsh2[i], descriptor_.kTables);

    codeword.resize(codeword.size() + gap);
    AFFT.inverse_afft(codeword.data(), n, 0, descriptor_.kTables);
    tweaked_formal_derivative(codeword, n);

    AFFT.afft(codeword.data(), n, 0, descriptor_.kTables);

    for (size_t i = 0ull; i < recover_up_to; ++i)
      codeword[i] = (i >= erasure.size() || erasure[i].empty())
                        ? codeword[i].mul(log_walsh2[i], descriptor_.kTables)
                        : Additive<Descriptor>{0};
  }

  void tweaked_formal_derivative(std::vector<Additive<Descriptor>> &codeword,
                                 size_t n) const {
    formal_derivative(codeword, n);
  }

  void formal_derivative(std::vector<Additive<Descriptor>> &cos,
                         size_t size) const {
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

  void encodeLow(const std::vector<Additive<Descriptor>> &data, size_t k,
                 std::vector<Additive<Descriptor>> &codeword, size_t n) const {
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
