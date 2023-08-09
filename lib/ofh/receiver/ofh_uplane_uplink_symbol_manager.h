/*
 *
 * Copyright 2021-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#pragma once

#include "../support/uplink_context_repository.h"
#include "ofh_uplane_uplink_packet_handler.h"
#include "srsran/adt/static_vector.h"
#include "srsran/ofh/ethernet/ethernet_frame_notifier.h"
#include "srsran/ofh/ofh_uplane_rx_symbol_notifier.h"
#include "srsran/ofh/serdes/ofh_uplane_message_decoder.h"

namespace srsran {
namespace ofh {

/// User-Plane uplink symbol manager configuration.
struct uplane_uplink_symbol_manager_config {
  uplane_uplink_symbol_manager_config(srslog::basic_logger&                                        logger_,
                                      uplane_rx_symbol_notifier&                                   notifier_,
                                      uplane_uplink_packet_handler&                                packet_handler_,
                                      std::shared_ptr<uplink_context_repository<ul_prach_context>> prach_repo_,
                                      std::shared_ptr<uplink_context_repository<ul_slot_context>>  ul_slot_repo_,
                                      const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC>&       ul_eaxc_,
                                      const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC>&       prach_eaxc_) :
    logger(logger_),
    notifier(notifier_),
    packet_handler(packet_handler_),
    prach_repo(prach_repo_),
    ul_slot_repo(ul_slot_repo_),
    ul_eaxc(ul_eaxc_),
    prach_eaxc(prach_eaxc_)
  {
  }

  /// Logger.
  srslog::basic_logger& logger;
  /// User-Plane receive symbol notifier.
  uplane_rx_symbol_notifier& notifier;
  /// User-Plane uplink packet handler.
  uplane_uplink_packet_handler& packet_handler;
  /// PRACH context repository.
  std::shared_ptr<uplink_context_repository<ul_prach_context>> prach_repo;
  /// Uplink slot context repository.
  std::shared_ptr<uplink_context_repository<ul_slot_context>> ul_slot_repo;
  /// Uplink eAxC.
  static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> ul_eaxc;
  /// PRACH eAxC.
  static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC> prach_eaxc;
};

/// User-Plane uplink symbol manager.
class uplane_uplink_symbol_manager : public ether::frame_notifier
{
public:
  explicit uplane_uplink_symbol_manager(const uplane_uplink_symbol_manager_config& config);

  /// Handles the given User-Plane decoder results.
  void on_new_frame(span<const uint8_t> payload) override;

private:
  /// Handles the PRACH PRBs given in the results.
  void handle_prach_prbs(const message_decoder_results& results);

  /// Handles the uplink grid PRBs given in the results.
  void handle_grid_prbs(const message_decoder_results& results);

private:
  srslog::basic_logger&                                        logger;
  const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC>        ul_eaxc;
  const static_vector<unsigned, MAX_NOF_SUPPORTED_EAXC>        prach_eaxc;
  std::shared_ptr<uplink_context_repository<ul_prach_context>> prach_repo_ptr;
  std::shared_ptr<uplink_context_repository<ul_slot_context>>  ul_slot_repo_ptr;
  uplane_rx_symbol_notifier&                                   notifier;
  uplane_uplink_packet_handler&                                packet_handler;
  uplink_context_repository<ul_prach_context>&                 prach_repo;
  uplink_context_repository<ul_slot_context>&                  ul_slot_repo;
};

} // namespace ofh
} // namespace srsran
