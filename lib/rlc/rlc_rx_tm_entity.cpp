/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "rlc_rx_tm_entity.h"

using namespace srsran;

rlc_rx_tm_entity::rlc_rx_tm_entity(uint32_t                          du_index,
                                   du_ue_index_t                     ue_index,
                                   rb_id_t                           rb_id,
                                   rlc_rx_upper_layer_data_notifier& upper_dn_,
                                   rlc_pcap&                         pcap_) :
  rlc_rx_entity(du_index, ue_index, rb_id, upper_dn_, pcap_), pcap_context(ue_index, rb_id, /* is_uplink */ true)
{
  metrics.metrics_set_mode(rlc_mode::tm);
  logger.log_info("RLC TM created.");
}

void rlc_rx_tm_entity::handle_pdu(byte_buffer_slice buf)
{
  metrics.metrics_add_pdus(1, buf.length());

  pcap.push_pdu(pcap_context, buf);

  logger.log_info(buf.begin(), buf.end(), "RX SDU. sdu_len={}", buf.length());
  metrics.metrics_add_sdus(1, buf.length());
  upper_dn.on_new_sdu(std::move(buf));
}
