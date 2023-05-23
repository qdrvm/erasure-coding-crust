/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NOVELPOLY_REED_SOLOMON_CRUST_ADDITIVE_FFT_HPP
#define NOVELPOLY_REED_SOLOMON_CRUST_ADDITIVE_FFT_HPP

namespace ec_cpp {

template <typename TDescriptor> struct Additive {
  using Descriptor = TDescriptor;
  typename Descriptor::Elt point_0;

  typename Descriptor::Multiplier
  toMultiplier(const typename Descriptor::Tables &tables) const {
    const auto &log_table = std::get<0>(tables);
    return typename Descriptor::Multiplier(log_table[size_t(point_0)]);
  }

  Additive mul(typename Descriptor::Multiplier other,
               const typename Descriptor::Tables &tables) {
    if (point_0 == typename Descriptor::Elt(0))
      return Additive{0};

    const auto &[log_table, exp_table, _] = tables;
    const auto log = (typename Descriptor::Wide(log_table[size_t(point_0)])) +
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

template <typename TDescriptor> struct AdditiveFFT {
  using Descriptor = TDescriptor;
  typename Descriptor::Multiplier skews[size_t(Descriptor::kOneMask)];

  static AdditiveFFT initalize(const typename Descriptor::Tables &tables) {
    typename Descriptor::Elt base[Descriptor::kFieldBits - 1ull] = {0};
    Additive<Descriptor> skews_additive[Descriptor::kOneMask] = {0};

    for (size_t i = 1; i < Descriptor::kFieldBits; ++i)
      base[i - 1] = 1 << i;

    for (size_t m = 0ull; m < (Descriptor::kFieldBits - 1); ++m) {
      const auto step = 1ull << (m + 1ull);
      skews_additive[(1ull << m) - 1ull] = Additive<Descriptor>{0};
      for (size_t i = m; i < (Descriptor::kFieldBits - 1); ++i) {
        const auto s = 1ull << (i + 1ull);
        auto j = (1ull << m) - 1ull;
        while (j < s) {
          skews_additive[j + s] = Additive<Descriptor>{
              typename Descriptor::Elt(skews_additive[j].point_0 ^ base[i])};
          j += step;
        }
      }

      const auto idx = Additive<Descriptor>{base[m]}.mul(
          Additive<Descriptor>{
              typename Descriptor::Elt(base[m] ^ typename Descriptor::Elt(1))}
              .toMultiplier(tables),
          tables);
      base[m] = Descriptor::kOneMask - idx.toMultiplier(tables);
      for (size_t i = (m + 1ull); i < (Descriptor::kFieldBits - 1ull); ++i) {
        const auto b =
            (Additive<Descriptor>{typename Descriptor::Elt(base[i] ^ 1)}
                 .toMultiplier(tables) +
             typename Descriptor::Wide(base[m])) %
            typename Descriptor::Wide(Descriptor::kOneMask);
        base[i] = Additive<Descriptor>{base[i]}
                      .mul(typename Descriptor::Multiplier(b), tables)
                      .point_0;
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
    return result;
  }

  void inverse_afft(Additive<Descriptor> *data, size_t size, size_t index,
                    const typename Descriptor::Tables &tables) const {
    size_t depart_no(1ull);
    while (depart_no < size) {
      size_t j(depart_no);
      while (j < size) {
        for (size_t i = (j - depart_no); i < j; ++i)
          data[i + depart_no].point_0 =
              (data[i + depart_no].point_0 ^ data[i].point_0);

        const auto skew = skews[j + index - 1ull];
        if (skew != Descriptor::kOneMask)
          for (size_t i = (j - depart_no); i < j; ++i)
            data[i].point_0 = (data[i].point_0 ^
                               data[i + depart_no].mul(skew, tables).point_0);

        j += (depart_no << 1ull);
      }
      depart_no = (depart_no << 1ull);
    }
  }

  void afft(Additive<Descriptor> *data, size_t size, size_t index,
            const typename Descriptor::Tables &tables) const {
    size_t depart_no(size >> 1ull);
    while (depart_no > 0) {
      size_t j(depart_no);
      while (j < size) {
        const auto skew = skews[j + index - 1ull];
        if (skew != Descriptor::kOneMask)
          for (size_t i = (j - depart_no); i < j; ++i)
            data[i].point_0 =
                data[i].point_0 ^ data[i + depart_no].mul(skew, tables).point_0;

        for (size_t i = (j - depart_no); i < j; ++i)
          data[i + depart_no].point_0 =
              data[i + depart_no].point_0 ^ data[i].point_0;

        j += (depart_no << 1ull);
      }
      depart_no = (depart_no >> 1ull);
    }
  }
};

} // namespace ec_cpp

#endif // NOVELPOLY_REED_SOLOMON_CRUST_ADDITIVE_FFT_HPP
