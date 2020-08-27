/*
 * Front-End module configuration:
 * Have front-end-module (SKY66420-11), connected to S2-LP's GPIOs:
 *
 * ---------------          -------------------
 *          GPIO0 |--------| CTX
 *  S2-LP   GPIO1 |--------| CPS  SKY66420-11
 *          GPIO2 |--------| CSD
 * ---------------          -------------------
 *
 * Always *bypass* (don't amplify) FEM in TX mode.
 */
#undef RENARD_PHY_S2LP_HAVE_FEM
#define RENARD_PHY_S2LP_HAVE_FEM 1

#define RENARD_PHY_S2LP_FEM_CSD_GPIO 2
#define RENARD_PHY_S2LP_FEM_CTX_GPIO 0
#define RENARD_PHY_S2LP_FEM_CPS_GPIO 1

/*
 * S2-LP output power configuration
 * Power is increased by the following amount compared to a baseline; measure output power to determine effect.
 * Value must be between -2 and 34. This makes it possible to tweak the S2-LP's output power based on whether the FEM
 * is in bypass or amplification
 * mode.
 */

// TODO: The following configuration is only for RCZ1, make RCZs configurable!
#define RENARD_PHY_S2LP_FEM_POWER_ADJUSTMENT 30

/*
 * 50MHz crystal, default values.
 * See conf_hardware.h for explanations.
 */
#define S2LP_XTAL_FREQ                  50000000
#define DISABLE_CLKDIV                  0

#define UPLINK_100BPS_DATARATE_M        20363
#define UPLINK_100BPS_DATARATE_E        1
#define UPLINK_600BPS_DATARATE_M        63313
#define UPLINK_600BPS_DATARATE_E        3

#define DOWNLINK_DATARATE_M             37543
#define DOWNLINK_DATARATE_E             1

#define UPLINK_100BPS_FDEV_M            168
#define UPLINK_100BPS_FDEV_E            0
#define UPLINK_600BPS_FDEV_M            247
#define UPLINK_600BPS_FDEV_E            2

#define DOWNLINK_FDEV_M                 67
#define DOWNLINK_FDEV_E                 0
