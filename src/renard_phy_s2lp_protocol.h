#include <stdint.h>

#include "renard_phy_s2lp.h"

/* librenard */
#include "downlink.h"
#include "uplink.h"

#ifndef _RENARD_PHY_S2LP_PROTOCOL_H
#define _RENARD_PHY_S2LP_PROTOCOL_H

typedef enum
{
	PROTOCOL_ERROR_NONE = 0,
	PROTOCOL_ERROR_ULENCODE,
	PROTOCOL_ERROR_TIMEOUT,
	PROTOCOL_ERROR_INVALID_PROFILE
} renard_phy_s2lp_protocol_error_t;

void renard_phy_s2lp_protocol_init(uint16_t random);
renard_phy_s2lp_protocol_error_t renard_phy_s2lp_protocol_transfer(sfx_commoninfo *common, sfx_ul_plain *uplink,
		sfx_dl_plain *downlink, renard_phy_s2lp_rc_t rc_profile, renard_phy_s2lp_ul_datarate_t datarate,
		int16_t *downlink_rssi);

#endif
