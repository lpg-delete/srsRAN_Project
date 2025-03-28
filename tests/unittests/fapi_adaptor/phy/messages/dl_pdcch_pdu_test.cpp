/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * This file is part of srsRAN.
 *
 * srsRAN is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * srsRAN is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 *
 */

#include "srsran/fapi/message_builders.h"
#include "srsran/fapi_adaptor/phy/messages/pdcch.h"
#include "srsran/fapi_adaptor/precoding_matrix_table_generator.h"
#include "srsran/srsvec/bit.h"
#include "srsran/support/math/math_utils.h"
#include <gtest/gtest.h>
#include <random>

using namespace srsran;
using namespace fapi_adaptor;

static std::mt19937 gen(0);

TEST(fapi_to_phy_pdcch_conversion_test, valid_pdu_conversion_success)
{
  // Random generators.
  std::uniform_int_distribution<unsigned> sfn_dist(0, 1023);
  std::uniform_int_distribution<unsigned> slot_dist(0, 159);
  std::uniform_int_distribution<unsigned> bwp_size_dist(1, 275);
  std::uniform_int_distribution<unsigned> bwp_start_dist(0, 274);
  std::uniform_int_distribution<unsigned> start_symbol_index_dist(0, 13);
  std::uniform_int_distribution<unsigned> duration_symbol_dist(0, 3);
  std::uniform_int_distribution<unsigned> shift_index_dist(0, 275);
  std::uniform_int_distribution<unsigned> n_rnti_dist(0, 65535);
  std::uniform_int_distribution<unsigned> cce_dist(0, 135);
  std::uniform_int_distribution<unsigned> aggregation_dist(0, 4);
  std::uniform_int_distribution<unsigned> nid_dmrs_dist(0, 65535);
  std::uniform_int_distribution<unsigned> nid_data_dist(0, 65535);

  auto                               pm_tools = generate_precoding_matrix_tables(1, 0);
  const precoding_matrix_repository& pm_repo  = *std::get<std::unique_ptr<precoding_matrix_repository>>(pm_tools);

  for (auto cp : {cyclic_prefix::NORMAL, cyclic_prefix::EXTENDED}) {
    for (auto cce_reg_mapping :
         {fapi::cce_to_reg_mapping_type::non_interleaved, fapi::cce_to_reg_mapping_type::interleaved}) {
      for (auto reg_bundle : {2U, 3U, 6U}) {
        for (auto interleaver_size : {2U, 3U, 6U}) {
          for (auto type : {fapi::pdcch_coreset_type::pbch_or_coreset0, fapi::pdcch_coreset_type::other}) {
            for (auto precoder : {coreset_configuration::precoder_granularity_type::same_as_reg_bundle,
                                  coreset_configuration::precoder_granularity_type::all_contiguous_rbs}) {
              for (int power_nr = -8; power_nr != -7; ++power_nr) {
                for (int power = -33; power != 3; power += 3) {
                  unsigned sfn                = sfn_dist(gen);
                  unsigned slot               = slot_dist(gen);
                  unsigned bwp_size           = bwp_size_dist(gen);
                  unsigned bwp_start          = bwp_start_dist(gen);
                  unsigned start_symbol_index = start_symbol_index_dist(gen);
                  unsigned duration_symbol    = duration_symbol_dist(gen);
                  unsigned shift_index        = shift_index_dist(gen);
                  unsigned n_rnti             = n_rnti_dist(gen);
                  unsigned cce                = cce_dist(gen);
                  unsigned aggregation        = pow2(aggregation_dist(gen));
                  unsigned nid_dmrs           = nid_dmrs_dist(gen);
                  unsigned nid_data           = nid_data_dist(gen);

                  fapi::dl_pdcch_pdu         pdu;
                  fapi::dl_pdcch_pdu_builder builder(pdu);

                  freq_resource_bitmap freq_domain = {1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0,
                                                      0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0,
                                                      0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1};

                  // Always work with the biggest numerology.
                  builder.set_bwp_parameters(bwp_size, bwp_start, subcarrier_spacing::kHz240, cp);
                  builder.set_coreset_parameters(start_symbol_index,
                                                 duration_symbol,
                                                 freq_domain,
                                                 cce_reg_mapping,
                                                 reg_bundle,
                                                 interleaver_size,
                                                 type,
                                                 shift_index,
                                                 precoder);

                  // Add DCI.
                  auto builder_dci = builder.add_dl_dci();

                  builder_dci.set_basic_parameters(to_rnti(0), nid_data, n_rnti, cce, aggregation);

                  builder_dci.set_tx_power_info_parameter(power_nr);
                  builder_dci.get_tx_precoding_and_beamforming_pdu_builder().set_basic_parameters(275, 0).add_prg(0,
                                                                                                                  {});

                  // Payload.
                  dci_payload payload = {1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1};
                  builder_dci.set_payload(payload);
                  builder_dci.set_parameters_v4_dci(nid_dmrs);

                  pdcch_processor::pdu_t proc_pdu;

                  convert_pdcch_fapi_to_phy(proc_pdu, pdu, sfn, slot, 0, pm_repo);

                  // Test basic parameters.
                  ASSERT_EQ(sfn, proc_pdu.slot.sfn());
                  ASSERT_EQ(slot, proc_pdu.slot.slot_index());
                  ASSERT_EQ(cp, proc_pdu.cp.value);

                  // Test coreset parameters.
                  ASSERT_EQ(bwp_size, proc_pdu.coreset.bwp_size_rb);
                  ASSERT_EQ(bwp_start, proc_pdu.coreset.bwp_start_rb);
                  ASSERT_EQ(start_symbol_index, proc_pdu.coreset.start_symbol_index);
                  ASSERT_EQ(duration_symbol, proc_pdu.coreset.duration);

                  if (type == fapi::pdcch_coreset_type::pbch_or_coreset0) {
                    ASSERT_TRUE(proc_pdu.coreset.cce_to_reg_mapping ==
                                pdcch_processor::cce_to_reg_mapping_type::CORESET0);
                    ASSERT_EQ(0, proc_pdu.coreset.reg_bundle_size);
                    ASSERT_EQ(0, proc_pdu.coreset.interleaver_size);
                    ASSERT_EQ(shift_index, proc_pdu.coreset.shift_index);
                  } else {
                    if (cce_reg_mapping == fapi::cce_to_reg_mapping_type::non_interleaved) {
                      ASSERT_TRUE(proc_pdu.coreset.cce_to_reg_mapping ==
                                  pdcch_processor::cce_to_reg_mapping_type::NON_INTERLEAVED);
                      ASSERT_EQ(0, proc_pdu.coreset.reg_bundle_size);
                      ASSERT_EQ(0, proc_pdu.coreset.interleaver_size);
                      ASSERT_EQ(0, proc_pdu.coreset.shift_index);
                    } else {
                      ASSERT_TRUE(proc_pdu.coreset.cce_to_reg_mapping ==
                                  pdcch_processor::cce_to_reg_mapping_type::INTERLEAVED);
                      ASSERT_EQ(reg_bundle, proc_pdu.coreset.reg_bundle_size);
                      ASSERT_EQ(interleaver_size, proc_pdu.coreset.interleaver_size);
                      ASSERT_EQ(shift_index, proc_pdu.coreset.shift_index);
                    }
                  }

                  // Test DCI.
                  ASSERT_EQ(n_rnti, proc_pdu.dci.n_rnti);
                  ASSERT_EQ(cce, proc_pdu.dci.cce_index);
                  ASSERT_EQ(aggregation, proc_pdu.dci.aggregation_level);
                  ASSERT_EQ(nid_data, proc_pdu.dci.n_id_pdcch_data);
                  ASSERT_EQ(nid_dmrs, proc_pdu.dci.n_id_pdcch_dmrs);

                  // Test powers.
                  ASSERT_TRUE(std::fabs((power_nr - proc_pdu.dci.dmrs_power_offset_dB)) < 0.001F);
                  ASSERT_TRUE(std::fabs((power_nr - proc_pdu.dci.data_power_offset_dB)) < 0.001F);

                  // Test vectors.
                  for (unsigned i = 0, e = payload.size(); i != e; ++i) {
                    ASSERT_EQ(payload.test(i), bool(proc_pdu.dci.payload[i]));
                  }

                  // Test frequency domain resources.
                  for (unsigned i = 0, e = 45; i != e; ++i) {
                    ASSERT_EQ(freq_domain.test(i), bool(proc_pdu.coreset.frequency_resources.test(i)));
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}
