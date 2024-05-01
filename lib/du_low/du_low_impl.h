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

#include "srsran/du_low/du_low.h"
#include "srsran/du_low/du_low_config.h"
#include "srsran/fapi_adaptor/phy/phy_fapi_adaptor.h"
#include "srsran/phy/upper/upper_phy.h"

namespace srsran {

/// DU low implementation.
class du_low_impl final : public du_low
{
public:
  explicit du_low_impl(std::vector<std::unique_ptr<upper_phy>> upper_);

  // See interface for documentation.
  upper_phy& get_upper_phy(unsigned cell_id) override;

  // See interface for documentation.
  void stop() override;

private:
  std::vector<std::unique_ptr<upper_phy>> upper;
};

} // namespace srsran
