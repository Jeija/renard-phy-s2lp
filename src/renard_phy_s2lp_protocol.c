#include <stdint.h>

/* librenard */
#include "uplink.h"
#include "downlink.h"

#include "renard_phy_s2lp_hal.h"

#include "renard_phy_s2lp_protocol.h"
#include "renard_phy_s2lp.h"

/*
 * See public Sigfox specifications "2.2 Frequency ranges, macro- and micro- channels",
 * 4.9.1 Time intervals in B-procedures, "4.9.2 Frequency selection in B-procedure".
 * Frequencies in Hz, durations in ms. Definitions here are only valid for RC1!
 */
#define FREQUENCY_BOUND_LOW 868030000
#define FREQUENCY_BOUND_HIGH 868230000
#define FREQUENCY_MF 6000
#define FREQUENCY_GAP 1395000

#define INTERVAL_INTERFRAME 500
#define INTERVAL_UL_TO_DL 20000
#define INTERVAL_DL_WINDOW 25000

static uint16_t m_random_current;
static renard_phy_s2lp_ul_datarate_t m_datarate;

/*
 * 16-bit XORshift
 * For internal use only, chooses next pseudorandom number (used for first uplink frequency) using.
 */
uint16_t random_next()
{
	m_random_current ^= m_random_current << 7;
	m_random_current ^= m_random_current >> 9;
	m_random_current ^= m_random_current << 8;

	return m_random_current;
}

void renard_phy_s2lp_protocol_init(uint16_t random, renard_phy_s2lp_ul_datarate_t datarate)
{
	m_random_current = random == 0 ? 1 : random;
	m_datarate = datarate;
}

renard_phy_s2lp_protocol_error_t renard_phy_s2lp_protocol_transfer(sfx_commoninfo *common, sfx_ul_plain *uplink,
		sfx_dl_plain *downlink, int16_t *downlink_rssi)
{
	/*
	 * Encode uplink using librenard
	 */
	sfx_ul_encoded uplink_encoded;
	if (sfx_uplink_encode(*uplink, *common, &uplink_encoded))
		return PROTOCOL_ERROR_ULENCODE;

	/*
	 * Randomly choose initial uplink's carrier frequency
	 */
	uint32_t lowerbound = FREQUENCY_BOUND_LOW + (uplink->replicas ? FREQUENCY_MF : 0);
	uint32_t upperbound = FREQUENCY_BOUND_HIGH - (uplink->replicas ? FREQUENCY_MF : 0);
	uint32_t initial_uplink_frequency = lowerbound + (uint64_t)(upperbound - lowerbound) * random_next() / 0xffff;

	/*
	 * Transmit uplink: Depending on whether or not replicas were requested, once or multiple times
	 */
	uint8_t ul_bytestream_len = 0;
	renard_phy_s2lp_mode(S2LP_MODE_TX);
	for (uint8_t fcount = 0; fcount < (uplink->replicas ? 3 : 1); fcount++) {
		ul_bytestream_len = (uplink_encoded.framelen_nibbles + 5) / 2;
		uint8_t bytestream[SFX_UL_MAX_FRAMELEN] = {0xaa, 0xaa, 0xa0};

		for (uint8_t i = 0; i < uplink_encoded.framelen_nibbles; i++) {
			bytestream[(i + 5) / 2] = i % 2 == 0 ?
					(bytestream[(i + 5) / 2] & 0xf0) | ((uplink_encoded.frame[fcount][i / 2] & 0xf0) >> 4) :
					(bytestream[(i + 5) / 2] & 0x0f) | ((uplink_encoded.frame[fcount][i / 2] & 0x0f) << 4);
		}

		/* Switch to correct frequency: Initial frame, first replica or second replica frequency */
		uint32_t frequency = initial_uplink_frequency;
		if (fcount == 1)
			frequency = initial_uplink_frequency + FREQUENCY_MF;
		else if (fcount == 2)
			frequency = initial_uplink_frequency - FREQUENCY_MF;
		renard_phy_s2lp_frequency(frequency);

		/* Transmit actual uplink */
		renard_phy_s2lp_tx(bytestream, m_datarate, ul_bytestream_len);

		/* Wait interframe period */
		if (uplink->replicas && fcount < 2) {
			renard_phy_s2lp_hal_interrupt_timeout(INTERVAL_INTERFRAME);
			renard_phy_s2lp_hal_interrupt_wait();
		}
	}

	/*
	 * If downlink was requested, wait for downlink
	 */
	bool timeout = false;

	if (uplink->request_downlink)
	{
		/*
		 * Wait until downlink window starts
		 * INTERVAL_UL_TO_DL is the time between the end of the final uplink replica and the start of the downlink
		 * listening window. Since we need the timeout interrupt for interframe timing during uplink procedure,
		 * we just compute how much time the replicas must have taken (if any).
		 */
		uint32_t replica_duration = 0;
		if (uplink->replicas) {
			replica_duration += 2 * INTERVAL_INTERFRAME;
			uint32_t symbol_duration_us = (m_datarate == UL_DATARATE_600BPS ? 1667 : 10000);
			replica_duration += 2 * ul_bytestream_len * 8 * symbol_duration_us / 1000;
		}
		renard_phy_s2lp_hal_interrupt_timeout(INTERVAL_UL_TO_DL - replica_duration);
		renard_phy_s2lp_hal_interrupt_wait();

		/* Put S2-LP in RX mode and start downlink window timer */
		sfx_dl_encoded dl_encoded;
		renard_phy_s2lp_mode(S2LP_MODE_RX);
		renard_phy_s2lp_frequency(initial_uplink_frequency + FREQUENCY_GAP);
		renard_phy_s2lp_hal_interrupt_timeout(INTERVAL_DL_WINDOW);

		while (true) {
			if (renard_phy_s2lp_rx(dl_encoded.frame, downlink_rssi))
			{
				/* received frame - might be valid, might be not - decode and check! */
				sfx_downlink_decode(dl_encoded, *common, downlink);

				if (downlink->crc_ok && downlink->mac_ok)
					break;
			} else {
				timeout = true;
				break;
			}
		}
	}

	/* increment uplink sequence number - must be done last, since downlink also uses SN! */
	common->seqnum = (common->seqnum + 1) % 4096;

	return timeout ? PROTOCOL_ERROR_TIMEOUT : PROTOCOL_ERROR_NONE;
}
