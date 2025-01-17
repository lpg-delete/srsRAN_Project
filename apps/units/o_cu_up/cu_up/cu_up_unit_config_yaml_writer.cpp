/*
 *
 * Copyright 2021-2025 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */

#include "cu_up_unit_config_yaml_writer.h"
#include "apps/services/network/udp_config_yaml_writer.h"
#include "cu_up_unit_config.h"
#include "srsran/adt/span.h"

using namespace srsran;

static void fill_cu_up_ngu_socket_entry(YAML::Node& node, const cu_up_unit_ngu_socket_config& config)
{
  node["bind_addr"]      = config.bind_addr;
  node["bind_interface"] = config.bind_interface;
  node["ext_addr"]       = config.ext_addr;
  fill_udp_config_in_yaml_schema(node["udp"], config.udp_config);
}

static void fill_cu_up_ngu_socket_section(YAML::Node node, const std::vector<cu_up_unit_ngu_socket_config>& sock_cfg)
{
  auto sock_node = node["socket"];
  for (const auto& cfg : sock_cfg) {
    YAML::Node node_sock;
    fill_cu_up_ngu_socket_entry(node_sock, cfg);
    sock_node.push_back(node_sock);
  }
}

static void fill_cu_up_ngu_section(YAML::Node node, const cu_up_unit_ngu_config& config)
{
  node["no_core"] = config.no_core;

  fill_cu_up_ngu_socket_section(node, config.ngu_socket_cfg);
}

static void fill_cu_up_metrics_section(YAML::Node node, const cu_up_unit_metrics_config& config)
{
  node["cu_up_statistics_report_period"] = config.cu_up_statistics_report_period;
  node["pdcp_report_period"]             = config.pdcp.report_period;
}

static void fill_cu_up_pcap_section(YAML::Node node, const cu_up_unit_pcap_config& config)
{
  node["n3_filename"]   = config.n3.filename;
  node["n3_enable"]     = config.n3.enabled;
  node["f1u_filename"]  = config.f1u.filename;
  node["f1u_enable"]    = config.f1u.enabled;
  node["e1ap_filename"] = config.e1ap.filename;
  node["e1ap_enable"]   = config.e1ap.enabled;
}

static void fill_cu_up_log_section(YAML::Node node, const cu_up_unit_logger_config& config)
{
  node["pdcp_level"]   = srslog::basic_level_to_string(config.pdcp_level);
  node["sdap_level"]   = srslog::basic_level_to_string(config.sdap_level);
  node["gtpu_level"]   = srslog::basic_level_to_string(config.gtpu_level);
  node["f1u_level"]    = srslog::basic_level_to_string(config.f1u_level);
  node["cu_level"]     = srslog::basic_level_to_string(config.cu_level);
  node["hex_max_size"] = config.hex_max_size;
}

static void fill_cu_up_section(YAML::Node node, const cu_up_unit_config& config)
{
  node["gtpu_queue_size"] = config.gtpu_queue_size;
  node["warn_on_drop"]    = config.warn_on_drop;
}

static void fill_cu_up_f1_qos_section(YAML::Node node, const cu_cp_unit_f1u_config& config)
{
  node["backoff_timer"] = config.t_notify;
}

static void fill_cu_up_qos_entry(YAML::Node node, const cu_up_unit_qos_config& config)
{
  node["five_qi"] = five_qi_to_uint(config.five_qi);
  fill_cu_up_f1_qos_section(node["f1u_cu_up"], config.f1u_cu_up);
}

static YAML::Node get_last_entry(YAML::Node node)
{
  auto it = node.begin();
  for (unsigned i = 1; i != node.size(); ++i) {
    ++it;
  }
  return *it;
}

static void fill_cu_up_qos_section(YAML::Node node, span<const cu_up_unit_qos_config> qos_cfg)
{
  auto qos_node = node["qos"];
  for (const auto& qos : qos_cfg) {
    auto node_entry = std::find_if(qos_node.begin(), qos_node.end(), [five = qos.five_qi](const YAML::Node& tmp) {
      return static_cast<uint16_t>(five) == tmp["five_qi"].as<uint16_t>();
    });
    if (node_entry != qos_node.end()) {
      YAML::Node node_five = *node_entry;
      fill_cu_up_qos_entry(node_five, qos);
    } else {
      qos_node.push_back(YAML::Node());
      fill_cu_up_qos_entry(get_last_entry(qos_node), qos);
    }
  }
}

void srsran::fill_cu_up_config_in_yaml_schema(YAML::Node& node, const cu_up_unit_config& config)
{
  node["gnb_id"]            = config.gnb_id.id;
  node["gnb_id_bit_length"] = static_cast<unsigned>(config.gnb_id.bit_length);
  node["gnb_cu_up_id"]      = static_cast<uint64_t>(config.gnb_cu_up_id);

  YAML::Node cu_up_node = node["cu_up"];
  fill_cu_up_section(cu_up_node, config);
  fill_cu_up_log_section(cu_up_node["log"], config.loggers);
  fill_cu_up_pcap_section(cu_up_node["pcap"], config.pcap_cfg);
  fill_cu_up_metrics_section(cu_up_node["metrics"], config.metrics);
  fill_cu_up_ngu_section(cu_up_node["ngu"], config.ngu_cfg);

  fill_cu_up_qos_section(cu_up_node, config.qos_cfg);
}
