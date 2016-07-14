/*
 * mrt.c
 *
 *  Created on: 2015/04/07
 *      Author: たま吉さん
 *      32ビット1msecカウンター
 *      0-4294967296msec(49.7日)
 */

#include "LPC8xx.h"
#include "mrt.h"

// グローバルタイマーカウンター
volatile static uint32_t mrt_counter = 0;

// インターバルタイマー割り込み関数
void MRT_IRQHandler(void) {
  if ( LPC_MRT->Channel[0].STAT & MRT_STAT_IRQ_FLAG ) {
		LPC_MRT->Channel[0].STAT = MRT_STAT_IRQ_FLAG;
		mrt_counter++;
  }
  return;
}

// インターバルタイマーの初期化
// 1msecの間隔でタイマーカウンターをインクリメントする
void mrt_init() {
	mrt_counter = 0;
	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1<<10);
	LPC_SYSCON->PRESETCTRL &= ~(0x1<<7);
	LPC_SYSCON->PRESETCTRL |= (0x1<<7);
	LPC_MRT->Channel[0].INTVAL = SystemCoreClock/1000;
	LPC_MRT->Channel[0].INTVAL |= 0x1UL<<31;
	LPC_MRT->Channel[0].CTRL = MRT_REPEATED_MODE|MRT_INT_ENA;
	NVIC_EnableIRQ(MRT_IRQn);
	return;
}

// ディレイ関数
// ms: ミリ秒
void delay(uint32_t ms) {
	uint32_t cnt = mrt_counter+ms;
	while(mrt_counter < cnt);
}

// カウンターの値を取得する
inline uint32_t mrt_read_counter() {
	  return mrt_counter;
}

