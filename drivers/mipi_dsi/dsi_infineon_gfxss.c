/*
 * Copyright André Costa <andre@lvgl.io>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT infineon_gfxss_mipi_dsi

#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/mipi_dsi.h>
#include <zephyr/logging/log.h>

#include <cy_mipidsi.h>

LOG_MODULE_REGISTER(dsi_infineon_gfxss, CONFIG_MIPI_DSI_LOG_LEVEL);

/* Largest packet accepted by Cy_MIPIDSI_WritePacket() and Cy_MIPIDSI_GenericWritePacket() */
#define IFX_GFXSS_DSI_MAX_PACKET 128U

struct ifx_gfxss_dsi_config {
	GFXSS_MIPIDSI_Type *base;
	const struct device *dc;
	uint8_t data_lanes;
	uint8_t pixfmt;
};

static int ifx_gfxss_dsi_attach(const struct device *dev, uint8_t channel,
				const struct mipi_dsi_device *mdev)
{
	const struct ifx_gfxss_dsi_config *config = dev->config;

	/*
	 * Reject a panel that asks for anything but this node's configuration,
	 * because the display controller brings up the link from these properties
	 * before any panel attaches.
	 */
	if (channel != 0U) {
		LOG_ERR("Only virtual channel 0 is supported");
		return -ENOTSUP;
	}

	if ((mdev->mode_flags & MIPI_DSI_MODE_VIDEO) == 0U) {
		LOG_ERR("Only video mode is supported");
		return -ENOTSUP;
	}

	if (mdev->data_lanes != config->data_lanes) {
		LOG_ERR("Panel requests %u lanes, the host is configured for %u", mdev->data_lanes,
			config->data_lanes);
		return -EINVAL;
	}

	if (mdev->pixfmt != config->pixfmt) {
		LOG_ERR("Panel requests pixel format %u, the host is configured for %u",
			mdev->pixfmt, config->pixfmt);
		return -EINVAL;
	}

	return 0;
}

static ssize_t ifx_gfxss_dsi_transfer(const struct device *dev, uint8_t channel,
				      struct mipi_dsi_msg *msg)
{
	const struct ifx_gfxss_dsi_config *config = dev->config;
	uint8_t buf[IFX_GFXSS_DSI_MAX_PACKET];
	cy_en_mipidsi_status_t status;
	size_t len;

	ARG_UNUSED(channel);

	switch (msg->type) {
	case MIPI_DSI_DCS_SHORT_WRITE:
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
	case MIPI_DSI_DCS_LONG_WRITE:
		/* Prepend the DCS command, because the HAL expects it as the first payload byte */
		if (msg->tx_len >= sizeof(buf)) {
			return -EINVAL;
		}
		buf[0] = msg->cmd;
		if (msg->tx_len > 0U) {
			memcpy(&buf[1], msg->tx_buf, msg->tx_len);
		}
		len = msg->tx_len + 1U;
		status = Cy_MIPIDSI_WritePacket(config->base, buf, len);
		break;
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
	case MIPI_DSI_GENERIC_LONG_WRITE:
		len = msg->tx_len;
		status = Cy_MIPIDSI_GenericWritePacket(config->base, msg->tx_buf, len);
		break;
	default:
		LOG_ERR("Unsupported message type (%d)", msg->type);
		return -ENOTSUP;
	}

	if (status == CY_MIPIDSI_TIMEOUT) {
		return -ETIMEDOUT;
	} else if (status != CY_MIPIDSI_SUCCESS) {
		return -EIO;
	}

	return (ssize_t)msg->tx_len;
}

static DEVICE_API(mipi_dsi, ifx_gfxss_dsi_api) = {
	.attach = ifx_gfxss_dsi_attach,
	.transfer = ifx_gfxss_dsi_transfer,
};

static int ifx_gfxss_dsi_init(const struct device *dev)
{
	const struct ifx_gfxss_dsi_config *config = dev->config;

	/* Only check the parent display controller, because it brings up the D-PHY and the host */
	if (!device_is_ready(config->dc)) {
		LOG_ERR("GFXSS display controller not ready");
		return -ENODEV;
	}

	return 0;
}

#define IFX_GFXSS_DSI_INIT(n)                                                                      \
	static const struct ifx_gfxss_dsi_config ifx_gfxss_dsi_config_##n = {                      \
		.base = &((GFXSS_Type *)DT_REG_ADDR(DT_INST_PARENT(n)))->GFXSS_MIPIDSI,            \
		.dc = DEVICE_DT_GET(DT_INST_PARENT(n)),                                            \
		.data_lanes = DT_INST_PROP(n, data_lanes),                                         \
		.pixfmt = DT_INST_PROP(n, pixel_format),                                           \
	};                                                                                         \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(n, ifx_gfxss_dsi_init, NULL, NULL, &ifx_gfxss_dsi_config_##n,        \
			      POST_KERNEL, CONFIG_MIPI_DSI_INIT_PRIORITY, &ifx_gfxss_dsi_api);

DT_INST_FOREACH_STATUS_OKAY(IFX_GFXSS_DSI_INIT)
