/*
 *
 * Copyright 2013-2023 Software Radio Systems Limited
 *
 * By using this file, you agree to the terms and conditions set
 * forth in the LICENSE file which can be found at the top level of
 * the distribution.
 *
 */
#include "pdxch_processor_impl.h"

using namespace srsran;

void srsran::pdxch_processor_impl::connect(pdxch_processor_notifier& notifier_)
{
  notifier = &notifier_;
}

pdxch_processor_request_handler& srsran::pdxch_processor_impl::get_request_handler()
{
  srsran_assert(notifier != nullptr, "Notifier has not been connected.");
  return *this;
}

pdxch_processor_baseband& srsran::pdxch_processor_impl::get_baseband()
{
  srsran_assert(notifier != nullptr, "Notifier has not been connected.");
  return *this;
}

void pdxch_processor_impl::process_symbol(baseband_gateway_buffer&                        samples,
                                          const pdxch_processor_baseband::symbol_context& context)
{
  srsran_assert(notifier != nullptr, "Notifier has not been connected.");

  // Renew the current slot and grid if the current slot is invalid or in the past.
  while ((current_slot.numerology() == to_numerology_value(subcarrier_spacing::invalid)) ||
         (context.slot > current_slot)) {
    // Try to get the next request.
    rg_grid_request request;
    if (!request_queue.try_pop(request)) {
      // No request available, then set the context slot and no grid.
      current_slot = context.slot;
      current_grid = nullptr;
    } else {
      // Request available, set the next slot and grid to process.
      current_slot = request.slot;
      current_grid = request.grid;
    }

    // Detect if the slot is in the past.
    if (context.slot > current_slot) {
      // Notify a late request.
      resource_grid_context late_context;
      late_context.slot   = current_slot;
      late_context.sector = context.sector;
      notifier->on_late_resource_grid(late_context);
    }
  }

  // Skip symbol processing if the context slot does not match with the current slot or no resource grid is available.
  if ((context.slot != current_slot) || (current_grid == nullptr)) {
    return;
  }

  // Symbol index within the subframe.
  unsigned symbol_index_subframe = context.symbol + context.slot.subframe_slot_index() * nof_symbols_per_slot;

  // Demodulate each of the ports.
  for (unsigned i_port = 0; i_port != nof_tx_ports; ++i_port) {
    modulator->modulate(samples.get_channel_buffer(i_port), *current_grid, i_port, symbol_index_subframe);
  }
}

void pdxch_processor_impl::handle_request(const resource_grid_reader& grid, const resource_grid_context& context)
{
  rg_grid_request request = {context.slot, &grid};
  if (!request_queue.try_push(request)) {
    notifier->on_overflow_resource_grid(context);
  }
}
