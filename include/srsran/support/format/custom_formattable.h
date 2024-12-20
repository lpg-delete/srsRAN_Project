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

#include "srsran/support/format/fmt_basic_parser.h"
#include <optional>

namespace srsran {

namespace detail {

template <typename FormatFunc>
class custom_formattable
{
public:
  custom_formattable(FormatFunc func_) : format(std::move(func_)) {}

  /// Functor that takes a "FormatContext& ctx" as argument.
  FormatFunc format;
};

} // namespace detail

/// \brief Creates a formattable object, whose format function is the passed functor/lambda \c func.
///
/// This function is useful to defer the formatting logic to the point where the format is actually needed (e.g.
/// logging backend).
/// \tparam FormatFunc Type of the format function.
/// \param func Format function to be called when formatting is needed.
/// \return Formattable object.
template <typename FormatFunc>
detail::custom_formattable<FormatFunc> make_formattable(FormatFunc func)
{
  return detail::custom_formattable<FormatFunc>(std::move(func));
}

namespace detail {

/// Formattable object that prepends a prefix (e.g. "snr=")  to a given value. If the value is not set, the prefix is
/// not logged.
template <typename T>
struct optional_prefix_formatter {
  optional_prefix_formatter(const char* label_, const std::optional<T>& value_) : prefix(label_), value(value_) {}
  optional_prefix_formatter(const char* label_, std::optional<T>&& value_) : prefix(label_), value(std::move(value_)) {}
  const char*      prefix;
  std::optional<T> value;
};

} // namespace detail

/// \brief Creates a formattable object that prefixes an optional value (e.g. " snr=<value>"), if the optional set.
///
/// This is useful to defer the formatting logic to the point where the format is actually needed (e.g. logging
/// backend).
template <typename T>
detail::optional_prefix_formatter<T> add_prefix_if_set(const char* prefix, const std::optional<T>& value)
{
  return detail::optional_prefix_formatter<T>(prefix, value);
}
template <typename T>
detail::optional_prefix_formatter<T> add_prefix_if_set(const char* prefix, std::optional<T>&& value)
{
  return detail::optional_prefix_formatter<T>(prefix, std::move(value));
}

} // namespace srsran

namespace fmt {

template <typename FormatFunc>
struct formatter<srsran::detail::custom_formattable<FormatFunc>> : public basic_parser {
public:
  template <typename FormatContext>
  auto format(const srsran::detail::custom_formattable<FormatFunc>& f, FormatContext& ctx) const
  {
    return f.format(ctx);
  }
};

template <typename T>
struct formatter<srsran::detail::optional_prefix_formatter<T>> : public basic_parser {
  template <typename FormatContext>
  auto format(const srsran::detail::optional_prefix_formatter<T>& f, FormatContext& ctx) const
  {
    if constexpr (std::is_enum_v<T>) {
      if (f.value.has_value()) {
        return fmt::format_to(ctx.out(), "{}{}", f.prefix, fmt::underlying(*f.value));
      }
    } else {
      if (f.value.has_value()) {
        return fmt::format_to(ctx.out(), "{}{}", f.prefix, *f.value);
      }
    }
    return ctx.out();
  }
};

} // namespace fmt