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

#include "ue/rrc_ue_impl.h"
#include "srsgnb/rrc/rrc_config.h"
#include "srsgnb/rrc/rrc_du.h"

namespace srsgnb {

namespace srs_cu_cp {

/// Main RRC representation with the DU
class rrc_du_impl : public rrc_du_interface
{
public:
  rrc_du_impl(const rrc_cfg_t&              cfg_,
              rrc_ue_du_processor_notifier& rrc_ue_du_proc_notif_,
              rrc_ue_nas_notifier&          nas_notif_,
              rrc_ue_control_notifier&      ngc_ctrl_notif_);
  ~rrc_du_impl() = default;

  // rrc_du_ue_manager
  rrc_ue_interface* add_ue(rrc_ue_creation_message msg) override;
  void              remove_ue(ue_index_t ue_index) override;
  void              release_ues() override;
  void              handle_amf_connection() override;
  void              handle_amf_connection_drop() override;

  // rrc_du_ue_manager
  bool is_rrc_connect_allowed() override;

  rrc_du_ue_manager&    get_rrc_du_ue_manager() override { return *this; }
  rrc_du_ue_repository& get_rrc_du_ue_repository() override { return *this; }

private:
  // helpers
  const rrc_cfg_t& cfg;

  bool reject_users = true; ///< Reject all connection attempts, i.e. when AMF is not connected.

  rrc_ue_du_processor_notifier& rrc_ue_du_proc_notifier; // notifier to the DU processor
  rrc_ue_nas_notifier&          nas_notifier;            // PDU notifier to the NGC
  rrc_ue_control_notifier&      ngc_ctrl_notifier;       // Control notifier to the NGC

  // RRC-internal user database indexed by ue_index
  slotted_array<std::unique_ptr<rrc_ue_impl>, MAX_NOF_UES> ue_db;
};

} // namespace srs_cu_cp

} // namespace srsgnb
