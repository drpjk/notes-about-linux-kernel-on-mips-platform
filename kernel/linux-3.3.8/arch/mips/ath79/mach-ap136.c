/*
 * Atheros AP136 reference board support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define AP136_GPIO_LED_USB		4
#define AP136_GPIO_LED_WLAN_5G		12
#define AP136_GPIO_LED_WLAN_2G		13
#define AP136_GPIO_LED_STATUS_RED	14
#define AP136_GPIO_LED_WPS_RED		15
#define AP136_GPIO_LED_STATUS_GREEN	19
#define AP136_GPIO_LED_WPS_GREEN	20

#define AP136_GPIO_BTN_WPS		16
#define AP136_GPIO_BTN_RFKILL		21

#define AP136_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP136_KEYS_DEBOUNCE_INTERVAL	(3 * AP136_KEYS_POLL_INTERVAL)

#define AP136_MAC0_OFFSET		0
#define AP136_MAC1_OFFSET		6
#define AP136_WMAC_CALDATA_OFFSET	0x1000
#define AP136_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led ap136_leds_gpio[] __initdata = {
	{
		.name		= "ap136:green:status",
		.gpio		= AP136_GPIO_LED_STATUS_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "ap136:red:status",
		.gpio		= AP136_GPIO_LED_STATUS_RED,
		.active_low	= 1,
	},
	{
		.name		= "ap136:green:wps",
		.gpio		= AP136_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "ap136:red:wps",
		.gpio		= AP136_GPIO_LED_WPS_RED,
		.active_low	= 1,
	},
	{
		.name		= "ap136:red:wlan-2g",
		.gpio		= AP136_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "ap136:red:usb",
		.gpio		= AP136_GPIO_LED_USB,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap136_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = AP136_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP136_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = AP136_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP136_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg ap136_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data ap136_ar8327_data = {
	.pad0_cfg = &ap136_ar8327_pad0_cfg,
	.cpuport_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	}
};

static struct mdio_board_info ap136_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ap136_ar8327_data,
	},
};

static void __init ap136_gmac_setup(void)
{
	void __iomem *base;
	u32 t;

	base = ioremap(QCA955X_GMAC_BASE, QCA955X_GMAC_SIZE);

	t = __raw_readl(base + QCA955X_GMAC_REG_ETH_CFG);

	t &= ~(QCA955X_ETH_CFG_RGMII_GMAC0 | QCA955X_ETH_CFG_SGMII_GMAC0);
	t |= QCA955X_ETH_CFG_RGMII_GMAC0;

	__raw_writel(t, base + QCA955X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

static void __init ap136_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap136_leds_gpio),
				 ap136_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AP136_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap136_gpio_keys),
					ap136_gpio_keys);

	ath79_register_usb();
	ath79_register_wmac(art + AP136_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(art + AP136_PCIE_CALDATA_OFFSET, NULL);

	ap136_gmac_setup();

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + AP136_MAC0_OFFSET, 0);

	mdiobus_register_board_info(ap136_mdio0_info,
				    ARRAY_SIZE(ap136_mdio0_info));

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_AP136, "AP136", "Atheros AP136 reference board",
	     ap136_setup);
