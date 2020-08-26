/*
 * Front-End module configuration:
 * No front-end module!
 */
#define RENARD_PHY_S2LP_HAVE_FEM 0

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
