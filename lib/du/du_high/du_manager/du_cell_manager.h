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

#pragma once

#include "srsran/du/du_high/du_manager/du_manager_params.h"
#include "srsran/ran/du_types.h"

namespace srsran {
namespace srs_du {

class du_cell_manager
{
public:
  explicit du_cell_manager(const du_manager_params& params_);

  size_t nof_cells() const { return cells.size(); }

  bool has_cell(du_cell_index_t cell_index) const { return cell_index < cells.size(); }

  bool is_cell_active(du_cell_index_t cell_index) const
  {
    assert_cell_exists(cell_index);
    return cells[cell_index]->active;
  }

  du_cell_index_t get_cell_index(nr_cell_global_id_t nr_cgi) const;

  du_cell_index_t get_cell_index(pci_t pci) const;

  void add_cell(const du_cell_config& cfg);

  const du_cell_config& get_cell_cfg(du_cell_index_t cell_index) const
  {
    assert_cell_exists(cell_index);
    return cells[cell_index]->cfg;
  }
  du_cell_config& get_cell_cfg(du_cell_index_t cell_index)
  {
    assert_cell_exists(cell_index);
    return cells[cell_index]->cfg;
  }

  async_task<void> start(du_cell_index_t cell_index);

  async_task<void> stop(du_cell_index_t cell_index);

  async_task<void> stop();

private:
  struct cell_t {
    bool           active = false;
    du_cell_config cfg;
  };

  void assert_cell_exists(du_cell_index_t cell_index) const
  {
    srsran_assert(has_cell(cell_index), "cell_index={} does not exist", fmt::underlying(cell_index));
  }

  const du_manager_params& cfg;
  srslog::basic_logger&    logger;

  std::vector<std::unique_ptr<cell_t>> cells;
};

} // namespace srs_du
} // namespace srsran
