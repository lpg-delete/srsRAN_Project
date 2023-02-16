#
# Copyright 2013-2022 Software Radio Systems Limited
#
# By using this file, you agree to the terms and conditions set
# forth in the LICENSE file which can be found at the top level of
# the distribution.
#

set_directory_properties(PROPERTIES LABELS "gtpu")

include_directories(../../..)

add_executable(gtpu_test gtpu_test.cpp)
target_link_libraries(gtpu_test srsgnb_gtpu srsgnb_support srslog gtest gtest_main)
gtest_discover_tests(gtpu_test)

add_executable(gtpu_demux_test gtpu_demux_test.cpp)
target_link_libraries(gtpu_demux_test srsgnb_gtpu srsgnb_support srslog gtest gtest_main)
gtest_discover_tests(gtpu_demux_test)