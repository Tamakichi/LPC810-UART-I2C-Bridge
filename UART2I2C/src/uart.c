/*
 * uart.c
 *
 *  Created on: 2015/04/06
 *      Author: たま吉さん
 */

#include "LPC8xx.h"
#include "uart.h"
#include "mrt.h"

uint8_t uart_prevBuffover; // 直前の１行読み込みでバッファーオーバーがあったかの状態

// UARTクロック供給設定
void uart_clockinit( LPC_USART_TypeDef *UARTx ) {
	LPC_SYSCON->UARTCLKDIV = 1;
	if (UARTx == LPC_USART0) {
		NVIC_DisableIRQ(UART0_IRQn); // 割り込みは使わない
		LPC_SYSCON->SYSAHBCLKCTRL |= (1<<14);
		LPC_SYSCON->PRESETCTRL &= ~(0x1<<3);
		LPC_SYSCON->PRESETCTRL |= (0x1<<3);
	}
	return;
}

// UARTの初期化
void uart_init(LPC_USART_TypeDef *UARTx, uint32_t baudrate) {
	uint32_t UARTSysClk;
	uart_clockinit(UARTx);
	UARTSysClk = SystemCoreClock/LPC_SYSCON->UARTCLKDIV;
	UARTx->CFG = DATA_LENG_8|PARITY_NONE|STOP_BIT_1; // 8 bits, no Parity, 1 Stop bit
	UARTx->BRG = UARTSysClk/16/baudrate-1;
	LPC_SYSCON->UARTFRGDIV = 0xFF;
	LPC_SYSCON->UARTFRGMULT = (((UARTSysClk/16)*(LPC_SYSCON->UARTFRGDIV+1))/(baudrate*(UARTx->BRG+1)))-(LPC_SYSCON->UARTFRGDIV+1);
	UARTx->STAT = CTS_DELTA | DELTA_RXBRK;
	UARTx->CFG |= UART_EN;
	uart_prevBuffover =0;
	return;
}

// 1文字出力
void uart_write(const char c){
 	while (!(LPC_USART0->STAT & TXRDY));
	LPC_USART0->TXDATA = c;
}

// 受信データチェック
int uart_available(void){
	if (LPC_USART0->STAT & RXRDY){
		// 受信データあり
		return 1;
	}
	return 0;
}

// 1文字読み取り
inline char uart_read(void){
	return (char) ( LPC_USART0->RXDATA );
}

//
// 1行読み込み
// s    : 読み込み文字列格納先
// len  : 最大読み込み文字列長
// tmout: タイムアウト(msec： 0の場合、タイムアウトなし)
// (補足) タイマーを使ってタイムアウト処理を行っている
//       直前の1行読み込み時にバッファーオーバー発生の場合、受信済み1バイトを廃棄する
//
int uart_readline(char *s , uint16_t len, uint32_t tmout) {
	int n = 0;
	uint32_t cnt = mrt_read_counter();
	uart_clear();
	while(1) {
		if ( uart_available() ) {
			*s = uart_read();
			if ( *s == 0x0d )
				continue;
			if ( *s == 0x0a )
				break;
			n++; s++; cnt = mrt_read_counter();
		} else {
			if ( tmout )
				if ( tmout > mrt_read_counter()-cnt )
					break;
		}
		if ( n >= len-1) {
			uart_prevBuffover =1;
			break;
		}
	}
	*s = 0;
	return n;
}

// 受信済みデータの破棄
void uart_clear() {
	if (uart_prevBuffover) {
		if (uart_available()) {
			uart_read();
		}
		uart_prevBuffover =0;
	}
}

// 文字列出力(改行なし)
void uart_print(const char *s) {
    int n;
    for (n = 0; *s; s++, n++) {
        uart_write(*s);
    }
    return;
}

// 改行出力
void uart_writeln(void) {
	uart_write (0x0d);
	uart_write (0x0a);
	return;
}

// 1行出力(改行あり)
void uart_println(const char *s) {
	uart_print(s);
	uart_writeln();
    return;
}

// 1バイト整数文字列出力
void uart_print_dec(uint8_t d) {
	if (d>100) {
		uart_write(d/100+'0');
		d%=100;
	}
	if (d>10) {
		uart_write(d/10+'0');
		d%=10;
	}
	uart_write(d+'0');
}

// 1バイト16進数文字列出力
void uart_print_hex(uint8_t d) {
	uint8_t v = d>>4;
	uart_write(v>9 ? v-10+'A': v+'0');
	v = d & 0xf;
	uart_write(v>9 ? v-10+'A': v+'0');
}

