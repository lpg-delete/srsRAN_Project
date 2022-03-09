/// \file
/// \brief LDPC rate dematcher declaration.

#ifndef SRSGNB_CHANNEL_CODING_LDPC_RATE_DEMATCHER_IMPL
#define SRSGNB_CHANNEL_CODING_LDPC_RATE_DEMATCHER_IMPL

#include "iostream"
#include "ldpc_graph_impl.h"
#include "srsgnb/phy/upper/channel_coding/ldpc_rate_dematcher.h"

namespace srsgnb {

/// \brief LDPC rate dematching implementation.
///
/// It reverts the rate matching procedure described in TS38.212 Section 5.4.2.
class ldpc_rate_dematcher_impl : public ldpc_rate_dematcher
{
public:
  void
  rate_dematch(span<int8_t> output, span<const int8_t> input, unsigned _nof_filler_bits, const config_t& cfg) override;

private:
  /// Initializes the rate dematcher internal state.
  void init(const config_t& cfg);

  /// Allots LLRs from the rate-matched input sequence to the full-sized output codeblock (i.e., reverts bit selection).
  /// \todo Change bits to llrs!
  void allot_bits(span<int8_t> out, span<const int8_t> in) const;

  /// Reverts the bit interleaving procedure.
  void deinterleave_bits(span<int8_t> out, span<const int8_t> in) const;

  // Data members.
  /// Auxiliary buffer.
  std::array<int8_t, ldpc::max_codeblock_length> auxiliary_buffer{};
  /// Redundancy version, values in {0, 1, 2, 3}.
  unsigned rv{0};
  /// Modulation scheme.
  unsigned modulation_order{1};
  /// Buffer length.
  unsigned buffer_length{0};
  /// Shift \f$ k_0 \f$ as defined in TS38.212 Table 5.4.2.1-2.
  unsigned shift_k0{0};
  /// Number of systematic bits (including filler bits) in the codeblock.
  unsigned nof_systematic_bits{0};
  /// Number of filler bits.
  unsigned nof_filler_bits{0};
};

} // namespace srsgnb
#endif // SRSGNB_CHANNEL_CODING_LDPC_RATE_DEMATCHER_IMPL
