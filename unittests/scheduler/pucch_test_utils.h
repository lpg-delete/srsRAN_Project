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

#include "config_generators.h"
#include "lib/du_manager/converters/mac_cell_configuration_helpers.h"
#include "lib/mac/mac_ctrl/sched_config_helpers.h"
#include "lib/scheduler/pucch_scheduling/pucch_allocator_impl.h"
#include "lib/scheduler/pucch_scheduling/pucch_scheduler_impl.h"
#include "scheduler_test_suite.h"
#include "srsgnb/du/du_cell_config_helpers.h"
#include <gtest/gtest.h>

namespace srsgnb {

////////////    Builder of PUCCH scheduler output     ////////////

// Builds the PUCCH scheduler output for test.
pucch_info build_pucch_info(const bwp_configuration* bwp_cfg,
                            unsigned                 pci,
                            pucch_format             format,
                            prb_interval             prbs,
                            prb_interval             second_hop_prbs,
                            ofdm_symbol_range        symbols,
                            uint8_t                  initial_cyclic_shift,
                            sr_nof_bits              sr_bits,
                            unsigned                 harq_ack_nof_bits,
                            uint8_t                  time_domain_occ);

// Verify if the PUCCH scheduler output (or PUCCH PDU) is correct.
bool assess_ul_pucch_info(const pucch_info& expected, const pucch_info& test);

// Makes a default DCI for PUCCH test purposes but some given parameters.
inline pdcch_dl_information make_default_dci(unsigned n_cces, const coreset_configuration* coreset_cfg_)
{
  pdcch_dl_information dci{};
  dci.ctx.cces.ncce   = n_cces;
  dci.ctx.coreset_cfg = coreset_cfg_;
  return dci;
}

inline sched_cell_configuration_request_message make_custom_sched_cell_configuration_request(unsigned pucch_res_common)
{
  sched_cell_configuration_request_message req = make_default_sched_cell_configuration_request();
  req.ul_cfg_common.init_ul_bwp.pucch_cfg_common->pucch_resource_common = pucch_res_common;
  return req;
}

/////////        TEST BENCH for PUCCH scheduler        /////////

// Test bench with all that is needed for the PUCCH.
class test_bench
{
public:
  explicit test_bench(unsigned       pucch_res_common = 11,
                      unsigned       n_cces           = 0,
                      sr_periodicity period           = sr_periodicity::sl_40,
                      unsigned       offset           = 0);

  const ue& get_ue() const;

  void slot_indication(slot_point slot_tx);

  cell_configuration      cell_cfg;
  cell_resource_allocator res_grid{cell_cfg};
  pdcch_dl_information    dci_info;
  const unsigned          k0;
  const unsigned          k1{4};
  du_ue_index_t           ue_idx{du_ue_index_t::MIN_DU_UE_INDEX};
  ue_list                 ues;
  pucch_allocator_impl    pucch_alloc;
  pucch_scheduler_impl    pucch_sched;
  slot_point              sl_tx;
  srslog::basic_logger&   mac_logger  = srslog::fetch_basic_logger("MAC");
  srslog::basic_logger&   test_logger = srslog::fetch_basic_logger("TEST");
};

} // namespace srsgnb