/*
 * Sigfox radio configuration zone profiles for renard-phy-s2lp, see C source files for definitions
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef _RENARD_PHY_S2LP_RC_PROFILES_H
#define _RENARD_PHY_S2LP_RC_PROFILES_H

extern const uint32_t renard_phy_s2lp_freq_bound_low_by_rc[];
extern const uint32_t renard_phy_s2lp_freq_bound_high_by_rc[];
extern const uint32_t renard_phy_s2lp_freq_ul_dl_gap_by_rc[];
extern const uint32_t renard_phy_s2lp_freq_interframe_gap_by_rc[];
extern const bool renard_phy_s2lp_baudrates_allowed_by_rc[][2];

/* the following configuration is only relevant if a front-end module (FEM) is present */
extern const bool renard_phy_s2lp_bypass_fem_by_rc[];
extern const int8_t renard_phy_s2lp_fem_power_adjustment_by_rc[];


#endif
