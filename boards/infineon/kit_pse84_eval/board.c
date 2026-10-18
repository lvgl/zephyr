/*
 * Copyright André Costa <andre@lvgl.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>

#include <infineon_kconfig.h>
#include <cy_gpio.h>
#include <cy_sysclk.h>

/*
 * Connect the 24 MHz clock on P7.4 to SRSS EXT_CLK and tell the PDL its frequency, so that
 * clock path 4 (switched to EXT_CLK in the board DTS) gives CLK_HF12, the MIPI D-PHY PLL
 * reference, the 24 MHz that the PDL's D-PHY PLL settings assume.
 *
 * Run at PRE_KERNEL_1 priority 0, so that the pin is connected before the clock drivers
 * switch the path mux to it.
 */
static int kit_pse84_eval_ext_clk_init(void)
{
	Cy_GPIO_Pin_SecFastInit(GPIO_PRT7, 4U, CY_GPIO_DM_HIGHZ, 0UL, P7_4_SRSS_EXT_CLK);
	Cy_SysClk_ExtClkSetFrequency(24000000U);

	return 0;
}

SYS_INIT(kit_pse84_eval_ext_clk_init, PRE_KERNEL_1, 0);
