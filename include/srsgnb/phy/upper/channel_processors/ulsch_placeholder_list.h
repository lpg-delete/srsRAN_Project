/*
 *
 * Copyright 2013-2022 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsgnb/adt/static_vector.h"
#include "srsgnb/ran/modulation_scheme.h"
#include "srsgnb/ran/pusch/pusch_constants.h"

namespace srsgnb {

/// \brief Abstracts a list of UL-SCH repetition placeholders \f$y\f$.
///
/// The process of descrambling reverses the effect of the repetition placeholders \f$y\f$ when one bit of information
/// is encoded in one of the Uplink Control Information (UCI) fields. The encoding is described in TS38.212
/// Section 5.3.3.1 and the scrambling in TS38.211 Section 6.3.1.1.
class ulsch_placeholder_list
{
  /// Indexing data type. Unsigned 16-bit integer is sufficient for the maximum resource grid size considering a maximum
  /// of \ref pusch_constants::MAX_NRE_PER_RB resource elements for \ref MAX_RB resource blocks.
  using index_type = uint16_t;

public:
  /// The maximum number of RE containing repetition placeholders is equal to the maximum number of RE used for a PUSCH
  /// transmission.
  static constexpr unsigned MAX_NOF_PLACEHOLDERS = pusch_constants::MAX_NRE_PER_RB * MAX_RB;

  /// Appends a resource element index that contains a repetition placeholder.
  void push_back(unsigned re_index)
  {
    // Append an RE index to the list.
    re_indexes.emplace_back(static_cast<index_type>(re_index));
  }

  /// \brief Applies the function \c func() to every repetition placeholder.
  ///
  /// The bit index is passed as an argument to the function.
  ///
  /// \tparam Func     Lambda function with <tt>void (unsigned)</tt> signature.
  /// \param func[in]  Lambda function to apply for each placeholder.
  template <typename Func>
  void for_each(modulation_scheme modulation, unsigned nof_layers, Func&& func) const
  {
    static_assert(std::is_convertible<Func, std::function<void(unsigned)>>::value,
                  "The function signature must be \"void () (unsigned)\"");
    unsigned bits_per_symbol = get_bits_per_symbol(modulation);

    // For each RE index in the list...
    for (uint16_t i_re : re_indexes) {
      // For each layer...
      for (unsigned i_layer = 0; i_layer != nof_layers; ++i_layer) {
        // Calculate the soft bit index that contains a placeholder within the RE as described in TS38.212
        // Table 5.3.3.1-1, where the repetition placeholder `y` is placed always on the second bit.
        unsigned i_bit = bits_per_symbol * (nof_layers * static_cast<unsigned>(i_re) + i_layer) + 1;

        // Call the lambda function.
        func(i_bit);
      }
    }
  }

private:
  /// List of RE indexes.
  static_vector<index_type, MAX_NOF_PLACEHOLDERS> re_indexes;
};

} // namespace srsgnb