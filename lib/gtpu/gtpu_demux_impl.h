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

#include "srsran/gtpu/gtpu_demux.h"
#include "srsran/pcap/dlt_pcap.h"
#include "srsran/support/executors/task_executor.h"
#include "fmt/format.h"
#include <memory>
#include <unordered_map>

namespace srsran {

struct gtpu_demux_tunnel_ctx_t {
  task_executor*                        tunnel_exec;
  gtpu_tunnel_rx_upper_layer_interface* tunnel;
};

class gtpu_demux_impl final : public gtpu_demux
{
public:
  explicit gtpu_demux_impl(gtpu_demux_cfg_t cfg_, dlt_pcap& gtpu_pcap_);
  ~gtpu_demux_impl() = default;

  // gtpu_demux_rx_upper_layer_interface
  void handle_pdu(byte_buffer pdu, const sockaddr_storage& src_addr) override; // Will be run from IO executor.

  // gtpu_demux_ctrl
  bool add_tunnel(gtpu_teid_t teid, task_executor& tunnel_exec, gtpu_tunnel_rx_upper_layer_interface* tunnel) override;
  bool remove_tunnel(gtpu_teid_t teid) override;

private:
  // Actual demuxing, to be run in CU-UP executor.
  void handle_pdu_impl(gtpu_teid_t                           teid,
                       gtpu_tunnel_rx_upper_layer_interface* tunnel,
                       byte_buffer                           pdu,
                       const sockaddr_storage&               src_addr);

  const gtpu_demux_cfg_t cfg;
  dlt_pcap&              gtpu_pcap;

  std::unordered_map<gtpu_teid_t, gtpu_demux_tunnel_ctx_t, gtpu_teid_hasher_t> teid_to_tunnel;

  srslog::basic_logger& logger;
};

} // namespace srsran

namespace fmt {
// GTP-U demux config formatter
template <>
struct formatter<srsran::gtpu_demux_cfg_t> {
  template <typename ParseContext>
  auto parse(ParseContext& ctx) -> decltype(ctx.begin())
  {
    return ctx.begin();
  }

  template <typename FormatContext>
  auto format(srsran::gtpu_demux_cfg_t cfg, FormatContext& ctx) -> decltype(std::declval<FormatContext>().out())
  {
    return format_to(ctx.out(), "warn_on_drop={}", cfg.warn_on_drop);
  }
};
} // namespace fmt
