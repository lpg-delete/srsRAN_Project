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

#include "srsran/cu_cp/mobility_manager_config.h"
#include "srsran/ran/cu_types.h"
#include "srsran/ran/logical_channel/lcid.h"
#include "srsran/ran/nr_cgi.h"
#include "srsran/ran/qos/qos_flow_id.h"
#include "srsran/support/async/async_task.h"
#include <map>

namespace srsran {

struct cu_handover_control_config {
  nr_cell_global_id_t                                    target_cell_id;
  std::map<pdu_session_id_t, std::vector<qos_flow_id_t>> pdu_sessions_for_ho;
  std::map<drb_id_t, std::vector<qos_flow_id_t>>         drbs_for_ho;
  std::vector<nr_cell_global_id_t>                       secondary_cells_to_be_added;
};

class cu_configurator
{
public:
  cu_configurator(srs_cu_cp::mobility_manager_cu_cp_notifier& mobility_notif_) : mobility_notif(mobility_notif_) {}
  virtual ~cu_configurator() = default;
  srs_cu_cp::mobility_manager_cu_cp_notifier& get_mobility_notifier() { return mobility_notif; }

private:
  srs_cu_cp::mobility_manager_cu_cp_notifier& mobility_notif;
};

} // namespace srsran
