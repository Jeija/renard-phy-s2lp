#include <stdbool.h>

#ifndef _RENARD_PHY_S2LP_HAL_H
#define _RENARD_PHY_S2LP_HAL_H

/*
 * Initialization
 */
void renard_phy_s2lp_hal_init(void);

/*
 * Output: SPI and Shutdown
 */
void renard_phy_s2lp_hal_spi(uint8_t length, uint8_t *in, uint8_t *out);
void renard_phy_s2lp_hal_shutdown(bool shutdown);

/*
 * Input: Timeout and GPIO interrupts
 */
void renard_phy_s2lp_hal_interrupt_timeout(uint32_t milliseconds);
void renard_phy_s2lp_hal_interrupt_gpio(bool risingTrigger);
void renard_phy_s2lp_hal_interrupt_clear(void);
bool renard_phy_s2lp_hal_interrupt_wait(void);

#endif
