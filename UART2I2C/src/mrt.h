/*
 * mrt.h
 *
 *  Created on: 2015/04/07
 *      Author: tamakichi
 */

#ifndef MRT_H_
#define MRT_H_

#include "lpc_types.h"

/* Control register bit definition. */
#define MRT_INT_ENA			(0x1<<0)
#define MRT_REPEATED_MODE	(0x00<<1)
#define MRT_ONE_SHOT_INT	(0x01<<1)
#define MRT_ONE_SHOT_STALL	(0x02<<1)

/* Status register bit definition */
#define MRT_STAT_IRQ_FLAG	(0x1<<0)
#define MRT_STAT_RUN		(0x1<<1)

void delay(uint32_t delayInMs);
void MRT_IRQHandler(void);
void mrt_init(void);
uint32_t mrt_read_counter(void);

#endif /* MRT_H_ */
