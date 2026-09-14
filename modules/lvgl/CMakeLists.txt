# Copyright (c) 2019 jan.van_winkel@dxplore.eu
# Copyright (c) 2020 Teslabs Engineering S.L.
# Copyright (c) 2023 Fabian Blatz <fabianblatz@gmail.com>
#
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_LVGL)

  set(ZEPHYR_CURRENT_LIBRARY lvgl)
  set(LVGL_DIR ${ZEPHYR_LVGL_MODULE_DIR})

  zephyr_interface_library_named(LVGL)
  zephyr_library()

  zephyr_include_directories(${LVGL_DIR}/src/)

  if(CONFIG_LV_USE_DRAW_PXP)
    zephyr_include_directories(${ZEPHYR_HAL_NXP_MODULE_DIR}/mcux/mcux-sdk-ng/drivers/pxp)
    zephyr_library_sources(${ZEPHYR_HAL_NXP_MODULE_DIR}/mcux/mcux-sdk-ng/drivers/pxp/fsl_pxp.c)
  endif()
  zephyr_include_directories(include)

  add_subdirectory_ifdef(CONFIG_LV_USE_NEMA_GFX nemagfx)

  zephyr_compile_definitions(LV_CONF_INCLUDE_SIMPLE=1)
  zephyr_library_compile_definitions(_POSIX_C_SOURCE=200809L)
  zephyr_compile_definitions(LV_CONF_PATH="${CMAKE_CURRENT_SOURCE_DIR}/include/lv_conf.h")

  file(GLOB_RECURSE LVGL_SRC_FILES CONFIGURE_DEPENDS
    ${LVGL_DIR}/src/*.c
  )

  zephyr_library_sources(${LVGL_SRC_FILES})

  if(CONFIG_LV_USE_THORVG_INTERNAL)
    file(GLOB_RECURSE LVGL_THORVG_SRC_FILES CONFIGURE_DEPENDS
      ${LVGL_DIR}/src/libs/thorvg/*.cpp
    )
    zephyr_library_sources(${LVGL_THORVG_SRC_FILES})
  endif()

  zephyr_library_sources(
      lvgl.c
      lvgl_display.c
      lvgl_display_8bit.c
      lvgl_display_16bit.c
      lvgl_display_24bit.c
      lvgl_display_32bit.c
      lvgl_zephyr_osal.c
  )
  zephyr_library_sources_ifdef(CONFIG_LV_Z_MONOCHROME_CONVERSION_BUFFER lvgl_display_mono.c)

  zephyr_library_sources_ifdef(CONFIG_LV_Z_USE_FILESYSTEM lvgl_fs.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_MEM_POOL_SYS_HEAP lvgl_mem.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_SHELL lvgl_shell.c)

  zephyr_library_sources(input/lvgl_common_input.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_POINTER_INPUT input/lvgl_pointer_input.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_BUTTON_INPUT input/lvgl_button_input.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_ENCODER_INPUT input/lvgl_encoder_input.c)
  zephyr_library_sources_ifdef(CONFIG_LV_Z_KEYPAD_INPUT input/lvgl_keypad_input.c)

  zephyr_library_link_libraries(LVGL)
  target_link_libraries(LVGL INTERFACE zephyr_interface)

endif()
