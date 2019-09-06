#include <stdbool.h>

/*
 * renard-phy-s2lp - Free Sigfox physical layer for STMicro's S2-LP
 *
 * - renard-phy-s2lp contains the pure physical layer implementation (DBPSK uplinks, GFSK downlinks) without any
 *   awareness of Sigfox protocol definitions
 * - renard-phy-s2lp-protocol has bindings to librenard and implements encoding / decoding of frames as well as
 *   scheduling aspects
 */

#ifndef _RENARD_PHY_S2LP_H
#define _RENARD_PHY_S2LP_H

typedef enum
{
	UL_DATARATE_100BPS = 0,
	UL_DATARATE_600BPS
} renard_phy_s2lp_ul_datarate_t;

typedef enum
{
	S2LP_MODE_TX = 0,
	S2LP_MODE_RX
} renard_phy_s2lp_mode_t;

bool renard_phy_s2lp_init(void);

void renard_phy_s2lp_mode(renard_phy_s2lp_mode_t mode);
void renard_phy_s2lp_stop(void);

void renard_phy_s2lp_tx(uint8_t *stream, renard_phy_s2lp_ul_datarate_t datarate, uint8_t size);
bool renard_phy_s2lp_rx(uint8_t *frame, int16_t *rssi);

void renard_phy_s2lp_frequency(uint32_t frequency);

#endif
