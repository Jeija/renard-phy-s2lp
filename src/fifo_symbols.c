#include <stdint.h>

#include "s2lp_registers.h"
#include "fifo_symbols.h"
#include "conf_hardware.h"

/*
 * FIFO direct polar mode symbol definitions. See datasheet "5.4.3 Direct polar mode" for more information.
 * Each Sigfox symbol consists of 40 FIFO byte couples, where for every byte couple the first byte controls
 * the instantaneous frequency and the second byte controls the instantaneous power.
 * Write command (0x00) and write address of FIFO (0xff) are directly included in the ramps so that the ramps
 * don't need to be copied to prepend these values when outputting them.
 *
 * FIFO_POLAR_ZERO_FDEV_NEG     = Sigfox binary '0' = 180° phase change symbol, phase shift through short negative
 *                                frequency deviation.
 * FIFO_POLAR_ZERO_FDEV_POS     = Sigfox binary '0' = 180° phase change symbol, phase shift through short positive
 *                                frequency deviation.
 * FIFO_POLAR_ONE               = Sigfox binary '1' = 0° phase change symbol, constant maximum TX power
 * FIFO_POLAR_BEFOREFRAME_1     = Ramp-up before every uplink, part 1
 * FIFO_POLAR_BEFOREFRAME_2     = Ramp-up before every uplink, part 2
 * FIFO_POLAR_AFTERFRAME_1      = Ramp-down after every uplink, part 1
 * FIFO_POLAR_AFTERFRAME_2      = Ramp-down after every uplink, part 2
 */
#define SPI_WRITE 0x00
#define FDEV_POS 0x7f
#define FDEV_NEG 0x81



#if (RENARD_PHY_S2LP_HAVE_FEM == 0)
/*
 * Symbol / pre-frame / after-frame waveforms for operation *WITHOUT* front-end-module
 */
const uint8_t FIFO_POLAR_ZERO_FDEV_NEG[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 0, 2, 0, 3, 0, 4, 0, 6, 0, 8, 0, 11, 0, 15, 0, 20,
	0, 24, 0, 30, 0, 39, 0, 54, 0, 220, FDEV_NEG, 220, 0, 54, 0, 39, 0, 30, 0, 24, 0, 20, 0, 15, 0, 11, 0, 8, 0, 6,
	0, 4, 0, 3, 0, 2, 0, 2, 0, 1, 0, 1
};
const uint8_t FIFO_POLAR_ZERO_FDEV_POS[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 0, 2, 0, 3, 0, 4, 0, 6, 0, 8, 0, 11, 0, 15, 0, 20,
	0, 24, 0, 30, 0, 39, 0, 54, 0, 220, FDEV_POS, 220, 0, 54, 0, 39, 0, 30, 0, 24, 0, 20, 0, 15, 0, 11, 0, 8, 0, 6,
	0, 4, 0, 3, 0, 2, 0, 2, 0, 1, 0, 1
};
const uint8_t FIFO_POLAR_ONE[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
	1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
	0, 1
};
const uint8_t FIFO_POLAR_BEFOREFRAME_1[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 80, 0, 70, 0, 67, 0, 60, 0, 51, 0, 45, 0, 40, 0, 35, 0, 31, 0, 26, 0, 24, 0, 22, 0, 20, 0, 18, 0, 17, 0, 16, 0,
	15, 0, 13, 0, 13, 0, 11, 0, 11, 0, 10, 0, 10, 0, 9, 0, 9, 0, 8, 0, 8, 0, 7, 0, 7, 0, 7, 0, 7, 0, 6, 0, 6, 0, 6, 0,
	6, 0, 6, 0, 5, 0, 5, 0, 5, 0, 5
};
const uint8_t FIFO_POLAR_BEFOREFRAME_2[FIFO_CMD_LENGTH + 64] = {
	SPI_WRITE, FIFO_ADDR,
	0, 5, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0,
	2, 0, 2, 0, 2, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1
};
const uint8_t FIFO_POLAR_AFTERFRAME_1[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 2, 0,
	2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4, 0, 4,
	0, 5
};
const uint8_t FIFO_POLAR_AFTERFRAME_2[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 5, 0, 5, 0, 5, 0, 5, 0, 6, 0, 6, 0, 6, 0, 6, 0, 6, 0, 7, 0, 7, 0, 7, 0, 7, 0, 8, 0, 8, 0, 9,	0, 9, 0, 10, 0, 10,
	0, 11, 0, 11, 0, 13, 0, 13, 0, 15, 0, 16, 0, 17, 0, 18, 0, 20, 0, 22, 0, 24, 0, 26, 0, 31, 0, 35, 0, 40, 0, 45, 0,
	51, 0, 60, 0, 67, 0, 70, 0, 80
};

#else

/*
 * Symbol / pre-frame / after-frame waveforms for operation *WITH* front-end-module
 */
const uint8_t FIFO_POLAR_ZERO_FDEV_NEG[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 36, 0, 36, 0, 37, 0, 38, 0, 40, 0, 42, 0,
	45, 0, 48, 0, 53, 0, 57, 0, 61, 0, 65, 0, 78, 0, 220, FDEV_NEG, 220, 0, 78, 0, 65, 0, 61, 0, 57, 0, 53, 0, 48,
	0, 45, 0, 42, 0, 40, 0, 38, 0, 37, 0, 36, 0, 36, 0, 35, 0, 35
};
const uint8_t FIFO_POLAR_ZERO_FDEV_POS[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 36, 0, 36, 0, 37, 0, 38, 0, 40, 0, 42, 0,
	45, 0, 48, 0, 53, 0, 57, 0, 61, 0, 65, 0, 78, 0, 220, FDEV_POS, 220, 0, 78, 0, 65, 0, 61, 0, 57, 0, 53, 0, 48,
	0, 45, 0, 42, 0, 40, 0, 38, 0, 37, 0, 36, 0, 36, 0, 35, 0, 35
};
const uint8_t FIFO_POLAR_ONE[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0,
	35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35,
	0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35
};
const uint8_t FIFO_POLAR_BEFOREFRAME_1[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 114, 0, 104, 0, 101, 0, 94, 0, 85, 0, 79, 0, 74, 0, 69, 0, 65, 0, 60, 0, 58, 0, 56, 0, 54, 0, 52, 0, 51, 0, 50,
	0, 49, 0, 47, 0, 47, 0, 45, 0, 45, 0, 44, 0, 44, 0, 43, 0, 43, 0, 42, 0, 42, 0, 41, 0, 41, 0, 41, 0, 41, 0, 40, 0,
	40, 0, 40, 0, 40, 0, 40, 0, 39, 0, 39, 0, 39, 0, 39
};
const uint8_t FIFO_POLAR_BEFOREFRAME_2[FIFO_CMD_LENGTH + 64] = {
	SPI_WRITE, FIFO_ADDR,
	0, 39, 0, 38, 0, 38, 0, 38, 0, 38, 0, 38, 0, 38, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37, 0, 36, 0, 36, 0,
	36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35
};
const uint8_t FIFO_POLAR_AFTERFRAME_1[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0, 35, 0,
	35, 0, 35, 0, 36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 36, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37, 0, 37,
	0, 38, 0, 38, 0, 38, 0, 38, 0, 38, 0, 38, 0, 39
};
const uint8_t FIFO_POLAR_AFTERFRAME_2[FIFO_CMD_LENGTH + FIFO_SYMBOL_LENGTH] = {
	SPI_WRITE, FIFO_ADDR,
	0, 39, 0, 39, 0, 39, 0, 39, 0, 40, 0, 40, 0, 40, 0, 40, 0, 40, 0, 41, 0, 41, 0, 41, 0, 41, 0, 42, 0, 42, 0, 43, 0,
	43, 0, 44, 0, 44, 0, 45, 0, 45, 0, 47, 0, 47, 0, 49, 0, 50, 0, 51, 0, 52, 0, 54, 0, 56, 0, 58, 0, 60, 0, 65, 0, 69,
	0, 74, 0, 79, 0, 85, 0, 94, 0, 101, 0, 104, 0, 114
};

#endif
