# Copyright André Costa <andre@lvgl.io>
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BOARD_KIT_PSE84_EVAL_PSE846GPS2DBZC4A_M33)
  zephyr_library()
  zephyr_library_sources(board.c)
endif()
