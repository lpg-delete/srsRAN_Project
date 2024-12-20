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

#include "../cu_cp_impl_interface.h"
#include "../ue_manager/cu_cp_ue_impl_interface.h"
#include "srsran/cu_cp/ue_task_scheduler.h"
#include "srsran/nrppa/nrppa.h"

namespace srsran {
namespace srs_cu_cp {

/// Adapter between NRPPa and CU-CP
class nrppa_cu_cp_adapter : public nrppa_cu_cp_notifier
{
public:
  nrppa_cu_cp_adapter() = default;

  void connect_cu_cp(cu_cp_nrppa_handler& cu_cp_handler_) { cu_cp_handler = &cu_cp_handler_; }

  nrppa_cu_cp_ue_notifier* on_new_nrppa_ue(ue_index_t ue_index) override
  {
    srsran_assert(cu_cp_handler != nullptr, "CU-CP NRPPA handler must not be nullptr");
    return cu_cp_handler->handle_new_nrppa_ue(ue_index);
  }

  void on_ul_nrppa_pdu(const byte_buffer& nrppa_pdu, std::optional<ue_index_t> ue_index) override
  {
    srsran_assert(cu_cp_handler != nullptr, "CU-CP NRPPA handler must not be nullptr");
    cu_cp_handler->handle_ul_nrppa_pdu(nrppa_pdu, ue_index);
  }

private:
  cu_cp_nrppa_handler* cu_cp_handler = nullptr;
};

/// Adapter between NRPPA and CU-CP UE
class nrppa_cu_cp_ue_adapter : public nrppa_cu_cp_ue_notifier
{
public:
  nrppa_cu_cp_ue_adapter() = default;

  void connect_ue(cu_cp_ue_impl_interface& ue_) { ue = &ue_; }

  /// \brief Get the UE index of the UE.
  ue_index_t get_ue_index() override
  {
    srsran_assert(ue != nullptr, "CU-CP UE must not be nullptr");
    return ue->get_ue_index();
  }

  std::optional<cell_measurement_positioning_info>& on_measurement_results_required() override
  {
    srsran_assert(ue != nullptr, "CU-CP UE must not be nullptr");
    return ue->get_measurement_results();
  }

  /// \brief Schedule an async task for the UE.
  bool schedule_async_task(async_task<void> task) override
  {
    srsran_assert(ue != nullptr, "CU-CP UE must not be nullptr");
    return ue->get_task_sched().schedule_async_task(std::move(task));
  }

private:
  cu_cp_ue_impl_interface* ue = nullptr;
};

} // namespace srs_cu_cp
} // namespace srsran