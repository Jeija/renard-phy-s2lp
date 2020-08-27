#include "renard_phy_s2lp_rc_profiles.h"

/*
 * See public Sigfox specifications "2.2 Frequency ranges, macro- and micro- channels",
 * 4.9.1 Time intervals in B-procedures, "4.9.2 Frequency selection in B-procedure".
 * Frequencies in Hz, durations in ms.
 */

/* Sigfox Specifications: Table 2-1 */
const uint32_t renard_phy_s2lp_freq_bound_low_by_rc[] = {
	868034000, // RC1
	902104000  // RC2
};

const uint32_t renard_phy_s2lp_freq_bound_high_by_rc[] = {
	868226000, // RC1
	902296000  // RC2
};

const uint32_t renard_phy_s2lp_freq_ul_dl_gap_by_rc[] = {
	1395000, // RC1
	3000000  // RC2
};

/* Sigfox Specifications: Table 4-3 */
const uint32_t renard_phy_s2lp_freq_interframe_gap_by_rc[] = {
	6000, // RC1
	25000 // RC2
};

/* Sigfox Specifications: Table 2-4 */
const bool renard_phy_s2lp_baudrates_allowed_by_rc[][2] = {
	{true, true}, // RC1: 100bps supported, 600bps supported
	{false, true} // RC1: 100bps unsupported, 600bps supported
};

/* the following configuration is only relevant if a front-end module (FEM) is present */
const bool renard_phy_s2lp_bypass_fem_by_rc[] = {
	true, // RC1: bypass, only use S2-LP PA
	false // RC2: additionally use FEM to boost signal
};

const int8_t renard_phy_s2lp_fem_power_adjustment_by_rc[] = {
	30, // RC1: Increase output power because we bypass the FEM
	27  // RC2: Increase output power since higher output powers are allowed
};
