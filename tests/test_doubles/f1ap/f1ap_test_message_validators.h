/*
 *
 * Copyright 2021-2024 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/rb_id.h"
#include "srsran/ran/rnti.h"
#include <optional>

namespace srsran {

struct f1ap_message;
class byte_buffer;

namespace srs_du {
struct gnbdu_config_update_request;
}

namespace test_helpers {

bool is_gnb_du_config_update_valid(const f1ap_message& msg);
bool is_gnb_du_config_update_valid(const f1ap_message& msg, const srs_du::gnbdu_config_update_request& req);

/// \brief Check if an F1AP message is a valid Initial UL RRC Message Transfer message.
bool is_init_ul_rrc_msg_transfer_valid(const f1ap_message&                       msg,
                                       rnti_t                                    rnti,
                                       const std::optional<nr_cell_global_id_t>& nci = {});

bool is_valid_dl_rrc_message_transfer(const f1ap_message& msg);

const byte_buffer& get_rrc_container(const f1ap_message& msg);

byte_buffer get_du_to_cu_container(const f1ap_message& msg);

bool is_valid_dl_rrc_message_transfer_with_msg4(const f1ap_message& msg);

bool is_ul_rrc_msg_transfer_valid(const f1ap_message& msg, srb_id_t srb_id);

bool is_valid_ue_context_setup_request(const f1ap_message& msg);

bool is_valid_ue_context_setup_request_with_ue_capabilities(const f1ap_message& msg);

bool is_ue_context_setup_response_valid(const f1ap_message& msg);

bool is_valid_ue_context_modification_request(const f1ap_message& msg);

enum class ue_context_mod_context { default_case, reestablistment };
bool is_valid_ue_context_modification_response(const f1ap_message&    msg,
                                               ue_context_mod_context context = ue_context_mod_context::default_case);
bool is_valid_ue_context_modification_response(const f1ap_message&    resp_msg,
                                               const f1ap_message&    req_msg,
                                               ue_context_mod_context context = ue_context_mod_context::default_case);

bool is_valid_ue_context_release_command(const f1ap_message& msg);

bool is_valid_paging(const f1ap_message& msg);

bool is_valid_f1_reset_ack(const f1ap_message& msg);
bool is_valid_f1_reset_ack(const f1ap_message& req, const f1ap_message& resp);

} // namespace test_helpers
} // namespace srsran
