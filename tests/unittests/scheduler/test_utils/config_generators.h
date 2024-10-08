/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
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

#pragma once

#include "lib/du/du_high/du_manager/converters/scheduler_configuration_helpers.h"
#include "lib/scheduler/config/sched_config_manager.h"
#include "lib/scheduler/logging/scheduler_metrics_handler.h"
#include "srsran/du/du_cell_config_helpers.h"
#include "srsran/ran/duplex_mode.h"
#include "srsran/ran/pucch/pucch_info.h"
#include "srsran/scheduler/config/csi_helper.h"
#include "srsran/scheduler/config/logical_channel_config_factory.h"
#include "srsran/scheduler/config/sched_cell_config_helpers.h"
#include "srsran/scheduler/config/serving_cell_config.h"
#include "srsran/scheduler/config/serving_cell_config_factory.h"
#include "srsran/scheduler/mac_scheduler.h"
#include "srsran/support/test_utils.h"

namespace srsran {

class sched_metrics_ue_configurator;

namespace test_helpers {

inline sched_cell_configuration_request_message
make_default_sched_cell_configuration_request(const config_helpers::cell_config_builder_params_extended& params = {})
{
  sched_cell_configuration_request_message sched_req{};
  sched_req.cell_index           = to_du_cell_index(0);
  sched_req.pci                  = params.pci;
  sched_req.scs_common           = params.scs_common;
  sched_req.dl_carrier           = config_helpers::make_default_dl_carrier_configuration(params);
  sched_req.ul_carrier           = config_helpers::make_default_ul_carrier_configuration(params);
  sched_req.dl_cfg_common        = config_helpers::make_default_dl_config_common(params);
  sched_req.ul_cfg_common        = config_helpers::make_default_ul_config_common(params);
  sched_req.ssb_config           = config_helpers::make_default_ssb_config(params);
  sched_req.tdd_ul_dl_cfg_common = params.tdd_ul_dl_cfg_common;

  // The CORESET duration of 3 symbols is only permitted if dmrs-typeA-Position is set to 3. Refer TS 38.211, 7.3.2.2.
  const pdcch_type0_css_coreset_description coreset0_desc = pdcch_type0_css_coreset_get(
      *params.band, params.ssb_scs, params.scs_common, *params.coreset0_index, params.k_ssb->value());
  sched_req.dmrs_typeA_pos =
      coreset0_desc.nof_symb_coreset == 3U ? dmrs_typeA_position::pos3 : dmrs_typeA_position::pos2;

  sched_req.nof_beams = 1;

  // SIB1 parameters.
  sched_req.coreset0          = *params.coreset0_index;
  sched_req.searchspace0      = params.search_space0_index;
  sched_req.sib1_payload_size = 101; // Random size.

  srs_du::pucch_builder_params default_pucch_builder_params   = srs_du::du_cell_config{}.pucch_cfg;
  default_pucch_builder_params.nof_ue_pucch_f0_or_f1_res_harq = 3;
  default_pucch_builder_params.nof_ue_pucch_f2_res_harq       = 6;
  default_pucch_builder_params.nof_sr_resources               = 2;

  sched_req.pucch_guardbands = config_helpers::build_pucch_guardbands_list(
      default_pucch_builder_params, sched_req.ul_cfg_common.init_ul_bwp.generic_params.crbs.length());

  if (params.csi_rs_enabled) {
    csi_meas_config csi_meas      = config_helpers::make_csi_meas_config(params);
    pdsch_config    pdsch         = config_helpers::make_default_pdsch_config(params);
    sched_req.zp_csi_rs_list      = pdsch.zp_csi_rs_res_list;
    sched_req.nzp_csi_rs_res_list = csi_meas.nzp_csi_rs_res_list;
  }

  if (sched_req.tdd_ul_dl_cfg_common.has_value()) {
    sched_req.dl_data_to_ul_ack =
        config_helpers::generate_k1_candidates(*sched_req.tdd_ul_dl_cfg_common, params.min_k1);
  } else {
    sched_req.dl_data_to_ul_ack = {params.min_k1};
  }

  return sched_req;
}

inline cell_config_dedicated create_test_initial_ue_spcell_cell_config(const cell_config_builder_params& params = {})
{
  cell_config_dedicated cfg;
  cfg.serv_cell_idx = to_serv_cell_index(0);
  cfg.serv_cell_cfg = config_helpers::create_default_initial_ue_serving_cell_config(params);
  return cfg;
}

inline sched_ue_creation_request_message
create_default_sched_ue_creation_request(const cell_config_builder_params&    params      = {},
                                         const std::initializer_list<lcid_t>& lcid_to_cfg = {})
{
  sched_ue_creation_request_message msg{};

  msg.ue_index = to_du_ue_index(0);
  msg.crnti    = to_rnti(0x4601);

  scheduling_request_to_addmod sr_0{.sr_id = scheduling_request_id::SR_ID_MIN, .max_tx = sr_max_tx::n64};
  msg.cfg.sched_request_config_list.emplace();
  msg.cfg.sched_request_config_list->push_back(sr_0);

  msg.cfg.cells.emplace();
  msg.cfg.cells->push_back(create_test_initial_ue_spcell_cell_config(params));

  msg.cfg.lc_config_list.emplace();
  msg.cfg.lc_config_list->resize(2);
  (*msg.cfg.lc_config_list)[0] = config_helpers::create_default_logical_channel_config(lcid_t::LCID_SRB0);
  (*msg.cfg.lc_config_list)[1] = config_helpers::create_default_logical_channel_config(lcid_t::LCID_SRB1);
  for (lcid_t lcid : lcid_to_cfg) {
    if (lcid >= lcid_t::LCID_SRB2) {
      msg.cfg.lc_config_list->push_back(config_helpers::create_default_logical_channel_config(lcid));
    }
  }

  return msg;
}

inline sched_ue_creation_request_message
create_empty_spcell_cfg_sched_ue_creation_request(const cell_config_builder_params& params = {})
{
  sched_ue_creation_request_message msg{};

  msg.ue_index = to_du_ue_index(0);
  msg.crnti    = to_rnti(0x4601);

  cell_config_dedicated cfg;
  cfg.serv_cell_idx              = to_serv_cell_index(0);
  serving_cell_config& serv_cell = cfg.serv_cell_cfg;

  serv_cell.cell_index = to_du_cell_index(0);
  // > TAG-ID.
  serv_cell.tag_id = static_cast<tag_id_t>(0);

  msg.cfg.cells.emplace();
  msg.cfg.cells->push_back(cfg);

  msg.cfg.lc_config_list.emplace();
  msg.cfg.lc_config_list->resize(1);
  (*msg.cfg.lc_config_list)[0] = config_helpers::create_default_logical_channel_config(lcid_t::LCID_SRB0);

  return msg;
}

inline rach_indication_message generate_rach_ind_msg(slot_point prach_slot_rx, rnti_t temp_crnti, unsigned rapid = 0)
{
  rach_indication_message msg{};
  msg.cell_index = to_du_cell_index(0);
  msg.slot_rx    = prach_slot_rx;
  msg.occasions.emplace_back();
  msg.occasions.back().frequency_index = 0;
  msg.occasions.back().start_symbol    = 0;
  msg.occasions.back().preambles.emplace_back();
  msg.occasions.back().preambles.back().preamble_id  = rapid;
  msg.occasions.back().preambles.back().tc_rnti      = temp_crnti;
  msg.occasions.back().preambles.back().time_advance = phy_time_unit::from_seconds(0);
  return msg;
}

inline uplink_config make_test_ue_uplink_config(const config_helpers::cell_config_builder_params_extended& params)

{
  // > UL Config.
  uplink_config ul_config{};
  ul_config.init_ul_bwp.pucch_cfg.emplace();

  // >> PUCCH.
  auto& pucch_cfg = ul_config.init_ul_bwp.pucch_cfg.value();
  // PUCCH Resource Set ID 0.
  auto& pucch_res_set_0            = pucch_cfg.pucch_res_set.emplace_back();
  pucch_res_set_0.pucch_res_set_id = pucch_res_set_idx::set_0;
  pucch_res_set_0.pucch_res_id_list.emplace_back(pucch_res_id_t{0, 0});
  pucch_res_set_0.pucch_res_id_list.emplace_back(pucch_res_id_t{1, 1});
  pucch_res_set_0.pucch_res_id_list.emplace_back(pucch_res_id_t{2, 2});

  auto& pucch_res_set_1            = pucch_cfg.pucch_res_set.emplace_back();
  pucch_res_set_1.pucch_res_set_id = pucch_res_set_idx::set_1;
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{3, 3});
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{4, 4});
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{5, 5});
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{6, 6});
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{7, 7});
  pucch_res_set_1.pucch_res_id_list.emplace_back(pucch_res_id_t{8, 8});

  unsigned nof_rbs = params.cell_nof_crbs;

  // PUCCH resource format 1, for HARQ-ACK.
  // >>> PUCCH resource 0.
  pucch_resource res_basic{
      .res_id = pucch_res_id_t{0, 0}, .starting_prb = nof_rbs - 1, .format = pucch_format::FORMAT_1};

  res_basic.format_params.emplace<pucch_format_1_cfg>(
      pucch_format_1_cfg{.initial_cyclic_shift = 0, .nof_symbols = 14, .starting_sym_idx = 0, .time_domain_occ = 0});
  pucch_cfg.pucch_res_list.push_back(res_basic);
  // >>> PUCCH resource 1.
  pucch_cfg.pucch_res_list.push_back(res_basic);
  pucch_resource& res1 = pucch_cfg.pucch_res_list.back();
  res1.res_id          = pucch_res_id_t{1, 1};
  res1.starting_prb    = 1;
  res1.second_hop_prb.emplace(nof_rbs - res1.starting_prb - 1);
  // >>> PUCCH resource 2.
  pucch_cfg.pucch_res_list.push_back(res_basic);
  pucch_resource& res2 = pucch_cfg.pucch_res_list.back();
  res2.res_id          = pucch_res_id_t{2, 2};
  res2.second_hop_prb.emplace(1);
  res2.starting_prb = nof_rbs - res2.second_hop_prb.value() - 1;

  // PUCCH resource format 2, for HARQ-ACK and CSI.
  // >>> PUCCH resource 3.
  pucch_resource res_basic_f2{.starting_prb = 2, .format = pucch_format::FORMAT_2};
  res_basic_f2.res_id = pucch_res_id_t{3, 3};
  res_basic_f2.format_params.emplace<pucch_format_2_3_cfg>(
      pucch_format_2_3_cfg{.nof_prbs = 1, .nof_symbols = 2, .starting_sym_idx = 0});
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  // >>> PUCCH resource 4.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res4                                                = pucch_cfg.pucch_res_list.back();
  res4.res_id                                                         = pucch_res_id_t{4, 4};
  std::get<pucch_format_2_3_cfg>(res4.format_params).starting_sym_idx = 2;
  // >>> PUCCH resource 5.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res5                                                = pucch_cfg.pucch_res_list.back();
  res5.res_id                                                         = pucch_res_id_t{5, 5};
  std::get<pucch_format_2_3_cfg>(res5.format_params).starting_sym_idx = 4;
  // >>> PUCCH resource 6.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res6                                                = pucch_cfg.pucch_res_list.back();
  res6.res_id                                                         = pucch_res_id_t{6, 6};
  std::get<pucch_format_2_3_cfg>(res6.format_params).starting_sym_idx = 6;
  // >>> PUCCH resource 7.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res7                                                = pucch_cfg.pucch_res_list.back();
  res7.res_id                                                         = pucch_res_id_t{7, 7};
  std::get<pucch_format_2_3_cfg>(res7.format_params).starting_sym_idx = 8;
  // >>> PUCCH resource 8.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res8                                                = pucch_cfg.pucch_res_list.back();
  res8.res_id                                                         = pucch_res_id_t{8, 8};
  std::get<pucch_format_2_3_cfg>(res8.format_params).starting_sym_idx = 10;
  // >>> PUCCH resource 9.
  pucch_cfg.pucch_res_list.push_back(res_basic_f2);
  pucch_resource& res9                                                = pucch_cfg.pucch_res_list.back();
  res9.res_id                                                         = pucch_res_id_t{9, 9};
  std::get<pucch_format_2_3_cfg>(res9.format_params).starting_sym_idx = 12;

  // >>> PUCCH resource 10.
  pucch_cfg.pucch_res_list.push_back(res_basic);
  pucch_resource& res10 = pucch_cfg.pucch_res_list.back();
  res10.res_id          = pucch_res_id_t{10, 10};
  res10.starting_prb    = 0;

  // >>> PUCCH resource 11.
  pucch_cfg.pucch_res_list.push_back(res_basic);
  pucch_resource& res11 = pucch_cfg.pucch_res_list.back();
  res11.res_id          = pucch_res_id_t{11, 11};
  res11.starting_prb    = nof_rbs - 3;

  // TODO: add more PUCCH resources.

  // >>> SR Resource.
  const unsigned pucch_sr_res_id = pucch_cfg.pucch_res_list.size() - 1;
  pucch_cfg.sr_res_list.push_back(
      scheduling_request_resource_config{.sr_res_id    = 1,
                                         .sr_id        = uint_to_sched_req_id(0),
                                         .period       = sr_periodicity::sl_40,
                                         .offset       = 0,
                                         .pucch_res_id = pucch_res_id_t{pucch_sr_res_id, pucch_sr_res_id}});

  pucch_cfg.format_1_common_param.emplace();
  pucch_cfg.format_2_common_param.emplace(
      pucch_common_all_formats{.max_c_rate = max_pucch_code_rate::dot_25, .simultaneous_harq_ack_csi = true});

  // >>> dl-DataToUl-Ack
  // TS38.213, 9.1.2.1 - "If a UE is provided dl-DataToUL-ACK, the UE does not expect to be indicated by DCI format 1_0
  // a slot timing value for transmission of HARQ-ACK information that does not belong to the intersection of the set
  // of slot timing values {1, 2, 3, 4, 5, 6, 7, 8} and the set of slot timing values provided by dl-DataToUL-ACK for
  // the active DL BWP of a corresponding serving cell.
  // Inactive for format1_0."
  // Note2: Only k1 >= 4 supported.
  nr_band band =
      params.band.has_value() ? params.band.value() : band_helper::get_band_from_dl_arfcn(params.dl_f_ref_arfcn);
  if (band_helper::get_duplex_mode(band) == duplex_mode::FDD) {
    pucch_cfg.dl_data_to_ul_ack = {params.min_k1};
  } else {
    // TDD
    pucch_cfg.dl_data_to_ul_ack = config_helpers::generate_k1_candidates(params.tdd_ul_dl_cfg_common.value());
  }

  // > PUSCH config.
  ul_config.init_ul_bwp.pusch_cfg.emplace(config_helpers::make_default_pusch_config(params));
  if (band_helper::get_duplex_mode(band) == duplex_mode::TDD) {
    ul_config.init_ul_bwp.pusch_cfg->pusch_td_alloc_list =
        config_helpers::generate_k2_candidates(cyclic_prefix::NORMAL, params.tdd_ul_dl_cfg_common.value());
  }

  // Compute the max UCI payload per format.
  pucch_cfg.format_max_payload[pucch_format_to_uint(pucch_format::FORMAT_1)] = 2U;
  const auto& res_f2 = std::get<pucch_format_2_3_cfg>(res_basic_f2.format_params);
  pucch_cfg.format_max_payload[pucch_format_to_uint(pucch_format::FORMAT_2)] = get_pucch_format2_max_payload(
      res_f2.nof_prbs, res_f2.nof_symbols, to_max_code_rate_float(pucch_cfg.format_2_common_param.value().max_c_rate));

  // > SRS config.
  ul_config.init_ul_bwp.srs_cfg.emplace(config_helpers::make_default_srs_config(params));

  return ul_config;
}

inline slot_point generate_random_slot_point(subcarrier_spacing scs)
{
  static std::array<std::uniform_int_distribution<uint32_t>, NOF_NUMEROLOGIES> scs_dists = {
      std::uniform_int_distribution<uint32_t>{0, (10240 * get_nof_slots_per_subframe(subcarrier_spacing::kHz15)) - 1},
      std::uniform_int_distribution<uint32_t>{0, (10240 * get_nof_slots_per_subframe(subcarrier_spacing::kHz30)) - 1},
      std::uniform_int_distribution<uint32_t>{0, (10240 * get_nof_slots_per_subframe(subcarrier_spacing::kHz60)) - 1},
      std::uniform_int_distribution<uint32_t>{0, (10240 * get_nof_slots_per_subframe(subcarrier_spacing::kHz120)) - 1},
      std::uniform_int_distribution<uint32_t>{0, (10240 * get_nof_slots_per_subframe(subcarrier_spacing::kHz240)) - 1}};

  uint32_t count = scs_dists[to_numerology_value(scs)](test_rgen::get());
  return slot_point{scs, count};
}

/// Helper class to manage cell and UE configurations of a scheduler test.
class test_sched_config_manager
{
public:
  test_sched_config_manager(const cell_config_builder_params& builder_params,
                            const scheduler_expert_config&    expert_cfg_ = {});
  ~test_sched_config_manager();

  const sched_cell_configuration_request_message& get_default_cell_config_request() const { return default_cell_req; }
  const sched_ue_creation_request_message&        get_default_ue_config_request() const { return default_ue_req; }

  const cell_configuration* add_cell(const sched_cell_configuration_request_message& msg);

  const cell_configuration& get_cell(du_cell_index_t cell_idx) const { return *cfg_mng.common_cell_list()[cell_idx]; }

  const ue_configuration* add_ue(const sched_ue_creation_request_message& cfg_req);
  bool                    rem_ue(du_ue_index_t ue_index);

private:
  const cell_config_builder_params               builder_params;
  scheduler_expert_config                        expert_cfg;
  std::unique_ptr<sched_configuration_notifier>  cfg_notifier;
  std::unique_ptr<scheduler_metrics_notifier>    metric_notifier;
  std::unique_ptr<sched_metrics_ue_configurator> ue_metrics_configurator;
  scheduler_metrics_handler                      metrics_handler;

  sched_cell_configuration_request_message default_cell_req;
  sched_ue_creation_request_message        default_ue_req;

  sched_config_manager cfg_mng;
};

} // namespace test_helpers
} // namespace srsran
