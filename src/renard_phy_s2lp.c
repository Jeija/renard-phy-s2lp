#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "renard_phy_s2lp_hal.h"

#include "renard_phy_s2lp_rc_profiles.h"
#include "renard_phy_s2lp.h"
#include "s2lp_registers.h"
#include "conf_hardware.h"
#include "fifo_symbols.h"

/*
 * Modulation type, see register table in datasheet:
 * --> 0x6 is direct polar mode (used to generate DBPSK) for uplink
 * --> 0x2 is 2-GFSK BT = 2 for downlink
 */
#define UPLINK_MOD_TYPE                 0x6
#define DOWNLINK_MOD_TYPE               0x2

/**********************************************************************************************************************/

/*
 * Private, low-level SPI read / write functions
 * renard_phy_s2lp_cmd: Write command to S2-LP (datasheet: "6.1 Command List")
 * renard_phy_s2lp_write: Set value of S2-LP register
 * renard_phy_s2lp_read: Read value of S2-LP register
 */
static void renard_phy_s2lp_cmd(uint8_t cmd)
{
	uint8_t out_buffer[2];

	out_buffer[0] = 0x80;
	out_buffer[1] = cmd;

	renard_phy_s2lp_hal_spi(2, out_buffer, NULL);
}

static void renard_phy_s2lp_symbol(const uint8_t *symbol, uint8_t length, renard_phy_s2lp_rc_t rc_profile)
{
	uint8_t symbol_poweradjusted[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH];

	memcpy(symbol_poweradjusted, symbol, length);

#if (RENARD_PHY_S2LP_HAVE_FEM == 1)
	// If using special symbol waveforms for FEM, adjust power according to provided value (depends on RC profile and
	// on whether we want to bypass the FEM or let it amplify the TX signal).
	for (uint8_t i = FIFO_CMD_LENGTH + 1; i < length; i += 2)
		symbol_poweradjusted[i] = symbol_poweradjusted[i] - renard_phy_s2lp_fem_power_adjustment_by_rc[rc_profile];
#endif

	renard_phy_s2lp_hal_spi(length, symbol_poweradjusted, NULL);
}

static void renard_phy_s2lp_write(uint8_t address, uint8_t value)
{
	uint8_t out_buffer[3];

	out_buffer[0] = 0x00;
	out_buffer[1] = address;
	out_buffer[2] = value;

	renard_phy_s2lp_hal_spi(3, out_buffer, NULL);
}

static uint8_t renard_phy_s2lp_read(uint8_t address)
{
	uint8_t in_buffer[3], out_buffer[3];

	out_buffer[0] = 0x01;
	out_buffer[1] = address;
	out_buffer[2] = 0xff;

	renard_phy_s2lp_hal_spi(3, out_buffer, in_buffer);

	return in_buffer[2];
}

/**********************************************************************************************************************/

/*
 * Front-End Module (FEM) control functions, for S2-LP's GPIOs 0-2 connected to SKY66420-11:
 */

#if (RENARD_PHY_S2LP_HAVE_FEM == 1)

typedef enum
{
  S2LP_FEM_MODE_SHUTDOWN = 0x00,
  S2LP_FEM_MODE_TX_BYPASS,
  S2LP_FEM_MODE_TX,
  S2LP_FEM_MODE_RX
} renard_phy_s2lp_fem_mode_t;

#define S2_LP_GPIO_HIGH 0x9a
#define S2_LP_GPIO_LOW 0xa2

static void fem_mode(renard_phy_s2lp_fem_mode_t mode)
{
	switch (mode)
	{
		case S2LP_FEM_MODE_SHUTDOWN:
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CSD_GPIO, S2_LP_GPIO_LOW);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CTX_GPIO, S2_LP_GPIO_LOW);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CPS_GPIO, S2_LP_GPIO_LOW);
			break;

		case S2LP_FEM_MODE_RX:
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CSD_GPIO, S2_LP_GPIO_HIGH);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CTX_GPIO, S2_LP_GPIO_LOW);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CPS_GPIO, S2_LP_GPIO_LOW);
			break;

		case S2LP_FEM_MODE_TX_BYPASS:
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CSD_GPIO, S2_LP_GPIO_HIGH);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CTX_GPIO, S2_LP_GPIO_HIGH);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CPS_GPIO, S2_LP_GPIO_LOW);
			break;

		case S2LP_FEM_MODE_TX:
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CSD_GPIO, S2_LP_GPIO_HIGH);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CTX_GPIO, S2_LP_GPIO_HIGH);
			renard_phy_s2lp_write(GPIOx_CONF_ADDR_OFFSET + RENARD_PHY_S2LP_FEM_CPS_GPIO, S2_LP_GPIO_HIGH);
			break;
	}
}

#endif

/**********************************************************************************************************************/

/*
 * Private RX / TX mode initialization functions
 */
static void renard_phy_s2lp_tx_rf_init(void)
{
	/* Switch to "Direct through FIFO mode" and configure power levels */
	renard_phy_s2lp_write(PCKTCTRL1_ADDR, 0x04);
	renard_phy_s2lp_write(PA_POWER0_ADDR, 0x07);

	renard_phy_s2lp_write(PA_CONFIG1_ADDR, 0x01);

	/*
	 * Configure SMPS switching frequency and setup PA configuration:
	 * KRM = 0x1c28 = 7208, fdig = 25MHz: FSW = 7208 * fdig / 2**15 = 5.5MHz
	 * These values as well as those for PA_CONFIG1_ADDR and SYNTH_CONFIG2_ADDR are directly taken from STM32's original
	 * S2-LP Sigfox demo application, they have probably been optimized for minimal power consumption / reliability.
	 */
	renard_phy_s2lp_write(PM_CONF3_ADDR, 0x9c);
	renard_phy_s2lp_write(PM_CONF2_ADDR, 0x28);
	renard_phy_s2lp_write(PA_CONFIG1_ADDR, renard_phy_s2lp_read(PA_CONFIG1_ADDR) & 0xfd);
	renard_phy_s2lp_write(PA_CONFIG0_ADDR, 0xc8);
	renard_phy_s2lp_write(SYNTH_CONFIG2_ADDR, 0xd3);
}

static void renard_phy_s2lp_rx_rf_init(void)
{
	/* Configure data rate and frequency deviation */
	renard_phy_s2lp_write(MOD4_ADDR, (DOWNLINK_DATARATE_M >> 8) & 0xff);
	renard_phy_s2lp_write(MOD3_ADDR, (DOWNLINK_DATARATE_M >> 0) & 0xff);
	renard_phy_s2lp_write(MOD2_ADDR, (DOWNLINK_MOD_TYPE << 4) | DOWNLINK_DATARATE_E);
	renard_phy_s2lp_write(MOD1_ADDR, DOWNLINK_FDEV_E);
	renard_phy_s2lp_write(MOD0_ADDR, DOWNLINK_FDEV_M);

	/* Disable automatic packet decoding (CRC, FEC, Encoding, ...) + set 15-byte packet length */
	renard_phy_s2lp_write(PCKTCTRL3_ADDR, 0);
	renard_phy_s2lp_write(PCKTCTRL2_ADDR, 0);
	renard_phy_s2lp_write(PCKTCTRL1_ADDR, 0);
	renard_phy_s2lp_write(PCKTLEN1_ADDR, 0);
	renard_phy_s2lp_write(PCKTLEN0_ADDR, 15);

	/*
	 * Configure frame synchronization word:
	 * SYNC_LEN = 0x40 >> 2 = 0x10 = 16 --> 2 byte SYNC
	 * Sync word = SYNC1 .. SYNC0 = 0xb227 (Sigfox Downlink SYNC word)
	 * Preamble-based synchronization is *not* used.
	 */
	renard_phy_s2lp_write(PCKTCTRL6_ADDR, 0x40);
	renard_phy_s2lp_write(SYNC0_ADDR, 0xb2);
	renard_phy_s2lp_write(SYNC1_ADDR, 0x27);

	/*
	 * Configure reception RF settings and thresholds:
	 * AFC_ENABLED = 0 --> disable automatic frequency correction
	 * EQU_CTRL = 0b00 --> disable inter-symbol interference cancellation
	 * CS_BLANKING = 0 --> disable minimum RSSI for data reception
	 * RSSI_TH = 7 --> Minimum RSSI detection threshold -140dBm
	 * CLK_REC_P_GAIN_SLOW = 1 --> Clock recovery slow loop gain
	 * CLK_REC_P_GAIN_FAST = 3 --> Clock recovery fast loop gain
	 * PSTFLT_LEN = 1 --> 16 symbols post filter length
	 * CHFLT_M = 8, CHFLT_E = 8 --> RX filter bandwidth 2.1kHz (see "5.5.4 RX channel filter bandwidth" table 44)
	 *
	 * Values taken from ST's original S2-LP Sigfox demo.
	 */
	renard_phy_s2lp_write(AFC2_ADDR, 0x00);
	renard_phy_s2lp_write(ANT_SELECT_CONF_ADDR, 0x00);
	renard_phy_s2lp_write(RSSI_TH_ADDR, 0x07);
	renard_phy_s2lp_write(CLOCKREC2_ADDR, 0x20);
	renard_phy_s2lp_write(CLOCKREC1_ADDR, 0x70);
	renard_phy_s2lp_write(CHFLT_ADDR, 0x88);

	/*
	 * Configure SMPS switching frequency
	 * KRM = 0x0800 = 2048, fdig = 25MHz: FSW = 2048 * fdig / 2**15 = 1.563MHz
	 * Values taken from ST's original S2-LP Sigfox demo.
	 */
	renard_phy_s2lp_write(PM_CONF3_ADDR, 0x88);
	renard_phy_s2lp_write(PM_CONF2_ADDR, 0x00);
}

/**********************************************************************************************************************/

/*
 * Other private functions
 */
static void renard_phy_s2lp_reset(void)
{
	/* Power-On-Reset S2-LP (see datasheet "5.2 Power-On-Reset") */
	renard_phy_s2lp_hal_shutdown(true);
	renard_phy_s2lp_hal_interrupt_timeout(2);
	renard_phy_s2lp_hal_interrupt_wait();
	renard_phy_s2lp_hal_shutdown(false);
	renard_phy_s2lp_hal_interrupt_timeout(2);
	renard_phy_s2lp_hal_interrupt_wait();
	renard_phy_s2lp_hal_interrupt_clear();
}

/**********************************************************************************************************************/

/*
 * Public interface
 */
bool renard_phy_s2lp_init(void)
{
	renard_phy_s2lp_hal_init();
	renard_phy_s2lp_reset();

	uint8_t partnum = renard_phy_s2lp_read(DEVICE_INFO1_ADDR);
	uint8_t version = renard_phy_s2lp_read(DEVICE_INFO0_ADDR);

	/* Check if S2-LP responds to SPI commands by verifying PARTNUM and VERSION information */
	return (partnum == 0x03) && (version == 0xc1 || version == 0x91);
}


void renard_phy_s2lp_mode(renard_phy_s2lp_mode_t mode)
{
	renard_phy_s2lp_reset();

	/*
	 * Choose correct digital domain clock: f_XO or f_XO / 2
	 * Also, I have verified that writing writing 0x2e to XO_RCO_CONF1_ADDR massively reduces frequency drift.
	 * This is something that is also done by STMicro's original S2-LP Sigfox Middleware (X-CUBE-SFXS2LP1), but it is
	 * currently not documented in the datasheet.
	 */
	renard_phy_s2lp_write(XO_RCO_CONF1_ADDR, 0x2e | ((DISABLE_CLKDIV << 4) & 0x10));

	if (mode == S2LP_MODE_TX)
		renard_phy_s2lp_tx_rf_init();
	else
		renard_phy_s2lp_rx_rf_init();
}

void renard_phy_s2lp_stop(void)
{
	renard_phy_s2lp_cmd(CMD_SABORT);
	renard_phy_s2lp_hal_shutdown(true);
}

void renard_phy_s2lp_tx(uint8_t *stream, uint8_t size, renard_phy_s2lp_ul_datarate_t datarate,
		renard_phy_s2lp_rc_t rc_profile)
{
#if RENARD_PHY_S2LP_HAVE_FEM == 1
	/*
	 * Optional, if present: Configure front-end module
	 */
	fem_mode(renard_phy_s2lp_bypass_fem_by_rc[rc_profile] ? S2LP_FEM_MODE_TX_BYPASS : S2LP_FEM_MODE_TX);
#endif

	/* Configure S2-LP data rate (100bps, 600bps) */
	uint8_t rate_e = (datarate == UL_DATARATE_600BPS) ? UPLINK_600BPS_DATARATE_E : UPLINK_100BPS_DATARATE_E;
	uint16_t rate_m = (datarate == UL_DATARATE_600BPS) ? UPLINK_600BPS_DATARATE_M : UPLINK_100BPS_DATARATE_M;
	uint8_t fdev_e = (datarate == UL_DATARATE_600BPS) ? UPLINK_600BPS_FDEV_E : UPLINK_100BPS_FDEV_E;
	uint8_t fdev_m = (datarate == UL_DATARATE_600BPS) ? UPLINK_600BPS_FDEV_M : UPLINK_100BPS_FDEV_M;

	/* Configure datarate, modulation type, frequency deviation, enable PA power interpolator */
	renard_phy_s2lp_write(MOD4_ADDR, (rate_m >> 8) & 0xff);
	renard_phy_s2lp_write(MOD3_ADDR, (rate_m >> 0) & 0xff);
	renard_phy_s2lp_write(MOD2_ADDR, rate_e | (UPLINK_MOD_TYPE << 4));
	renard_phy_s2lp_write(MOD1_ADDR, fdev_e | 0x80);
	renard_phy_s2lp_write(MOD0_ADDR, fdev_m);

	/*
	 * Configure "FIFO almost empty" GPIO interrupt:
	 * --> S2-LP Conf: Set "almost empty" treshold down to 48 bytes:
	 *	 FIFO size is 128 bytes, max. symbol size is 80 bytes: 128 - 80 = 48 bytes.
	 * --> S2-LP GPIO: Output FIFO almost empty flag on GPIO3
	 * --> MCU GPIO: Enable interrupt with renard_phy_s2lp_hal_interrupt_gpio
	 */
	renard_phy_s2lp_write(FIFO_CONFIG0_ADDR, 48);
	renard_phy_s2lp_write(GPIO3_CONF_ADDR, 0x32);
	renard_phy_s2lp_hal_interrupt_gpio(true);

	/* Transmit "Extra Symbol Before Frame": First fill FIFO, then tell S2-LP to transmit FIFO contents */
	renard_phy_s2lp_cmd(CMD_FLUSHTXFIFO);
	renard_phy_s2lp_symbol(FIFO_POLAR_BEFOREFRAME_1, sizeof(FIFO_POLAR_BEFOREFRAME_1), rc_profile);
	renard_phy_s2lp_cmd(CMD_TX);
	renard_phy_s2lp_hal_interrupt_wait();
	renard_phy_s2lp_symbol(FIFO_POLAR_BEFOREFRAME_2, sizeof(FIFO_POLAR_BEFOREFRAME_2), rc_profile);

	/* Transmit actual DBPSK bits */
	uint8_t byte_index = 0;
	uint8_t bit_index = 7;

	while (byte_index < size)
	{
		/* Fetch current bit */
		uint8_t bit = (stream[byte_index] >> bit_index) & 0x01;

		/* Transmit bit: Always switch between +180° and -180° phase shifts (bit_index % 2 == 0) */
		renard_phy_s2lp_hal_interrupt_wait();
		if (bit == 0) {
			if (bit_index % 2 == 0) {
				renard_phy_s2lp_symbol(FIFO_POLAR_ZERO_FDEV_NEG, sizeof(FIFO_POLAR_ZERO_FDEV_NEG), rc_profile);
			} else {
				renard_phy_s2lp_symbol(FIFO_POLAR_ZERO_FDEV_POS, sizeof(FIFO_POLAR_ZERO_FDEV_POS), rc_profile);
			}
		} else {
			renard_phy_s2lp_symbol(FIFO_POLAR_ONE, sizeof(FIFO_POLAR_ONE), rc_profile);
		}

		/* Go to next bit */
		if (bit_index == 0) {
			bit_index = 7;
			byte_index++;
		} else {
			bit_index--;
		}
	}

	/* Transmit first part of "Extra Symbol After Frame" */
	renard_phy_s2lp_hal_interrupt_wait();
	renard_phy_s2lp_symbol(FIFO_POLAR_AFTERFRAME_1, sizeof(FIFO_POLAR_AFTERFRAME_1), rc_profile);
	renard_phy_s2lp_hal_interrupt_wait();

	/* Transmit final part of "Extra Symbol After Frame" - set FIFO almost empty threshold to zero so that
	   complete FIFO contents get transmitted */
	renard_phy_s2lp_write(FIFO_CONFIG0_ADDR, 0x00);
	renard_phy_s2lp_symbol(FIFO_POLAR_AFTERFRAME_2, sizeof(FIFO_POLAR_AFTERFRAME_2), rc_profile);
	renard_phy_s2lp_hal_interrupt_wait();

	/* Stop S2-LP transmission */
	renard_phy_s2lp_cmd(CMD_SABORT);
	renard_phy_s2lp_cmd(CMD_FLUSHTXFIFO);
	renard_phy_s2lp_hal_interrupt_clear();
#if RENARD_PHY_S2LP_HAVE_FEM == 1
	fem_mode(S2LP_FEM_MODE_SHUTDOWN);
#endif
}

void renard_phy_s2lp_frequency(uint32_t frequency)
{
	/*
	 * Frequency selection is explained in datasheet section 5.3.1, equations 6 / 7 / 8 / 9:
	 * Since CHNUM = 0, f_c = f_base here, so equation 7 effectively defines frequency selection:
	 * --> synth = 2^19 * B * D * f_base / f_xo
	 * --> where B = 4 since we are in the 826-1055MHz band.
	 * --> where D = 1 since REFDIV XO_RCO_CONFIG0 is 0 (has nothing to do with digital domain clock divider)
	 * The maximum error we make by rounding synth to an integer is on the order of 25Hz which is safe to ignore
	 * considering the error due to XTAL imperfections can be >15kHz (20ppm XTAL)
	 */
	uint32_t synth_freq = 4 * frequency;
	uint32_t synth = ((uint64_t)1 << 19) * synth_freq / S2LP_XTAL_FREQ;

	/*
	 * Charge pump configuration, see datsheet section 5.3 and table 37: "Charge pump words"
	 * The logic for selecting PLL_PFD_SPLIT_EN and PLL_CP_ISEL has been taken from X-CUBE-SFXS2LP1
	 */
	uint8_t pll_pfd_split_en = DISABLE_CLKDIV ? 1 : 0;
	uint8_t pll_cp_isel = (synth_freq < 3600000000) ?
		(DISABLE_CLKDIV ? 0x02 : 0x03) :
		(DISABLE_CLKDIV ? 0x01 : 0x02);

	renard_phy_s2lp_write(SYNTH_CONFIG2_ADDR, (renard_phy_s2lp_read(SYNTH_CONFIG2_ADDR) & (~0x04)) |
			(pll_pfd_split_en << 2));
	renard_phy_s2lp_write(SYNT3_ADDR, ((synth >> 24) & 0x0f) | (pll_cp_isel << 5));
	renard_phy_s2lp_write(SYNT2_ADDR, (synth >> 16) & 0xff);
	renard_phy_s2lp_write(SYNT1_ADDR, (synth >> 8) & 0xff);
	renard_phy_s2lp_write(SYNT0_ADDR, (synth >> 0) & 0xff);
}

bool renard_phy_s2lp_rx(uint8_t *frame, int16_t *rssi)
{
#if RENARD_PHY_S2LP_HAVE_FEM == 1
	/*
	 * Optional, if present: Configure front-end module
	 */
	fem_mode(S2LP_FEM_MODE_RX);
#endif

	/* disable all IRQs except for RX DATA READY */
	renard_phy_s2lp_write(IRQ_MASK3_ADDR, 0x00);
	renard_phy_s2lp_write(IRQ_MASK2_ADDR, 0x00);
	renard_phy_s2lp_write(IRQ_MASK1_ADDR, 0x00);
	renard_phy_s2lp_write(IRQ_MASK0_ADDR, 0x01);

	/* GPIO configuration: nIRQ (interrupt request, active low --> falling edge on MCU) on GPIO3 */
	renard_phy_s2lp_write(GPIO3_CONF_ADDR, 0x02);
	renard_phy_s2lp_hal_interrupt_gpio(false);

	/* clean up: flush FIFO and clear IRQ STATUS registers */
	renard_phy_s2lp_cmd(CMD_FLUSHRXFIFO);
	renard_phy_s2lp_read(IRQ_STATUS3_ADDR);
	renard_phy_s2lp_read(IRQ_STATUS2_ADDR);
	renard_phy_s2lp_read(IRQ_STATUS1_ADDR);
	renard_phy_s2lp_read(IRQ_STATUS0_ADDR);

	/* start RX */
	renard_phy_s2lp_cmd(CMD_RX);

	/*
	 * Downlink procedure is over if either:
	 * --> a downlink timer interrupt occurs
	 * --> the S2-LP received some data; only in that case will the S2-LP generate a GPIO interrupt
	 */
	bool is_gpio_ir = renard_phy_s2lp_hal_interrupt_wait();

	/* stop RX, disable interrupts */
	renard_phy_s2lp_cmd(CMD_SABORT);
#if RENARD_PHY_S2LP_HAVE_FEM == 1
	fem_mode(S2LP_FEM_MODE_SHUTDOWN);
#endif

	if (is_gpio_ir) {
		uint8_t length = renard_phy_s2lp_read(RX_FIFO_STATUS_ADDR);
		for (uint8_t i = 0; i < length; i++)
			frame[i] = renard_phy_s2lp_read(FIFO_ADDR);

		*rssi = renard_phy_s2lp_read(RSSI_LEVEL_ADDR) - 146;
	}

	return is_gpio_ir;
}
