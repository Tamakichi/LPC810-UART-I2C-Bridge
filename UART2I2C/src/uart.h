/*
 * uart.h
 *
 *  Created on: 2015/04/06
 *      Author: tamakichi
 */

#ifndef UART_H_
#define UART_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* Configuration register bit def. except data bit, parity, stop bits setting. */
#define UART_EN       (0x01<<0)
#define DATA_LENG_7   (0x00<<2)
#define DATA_LENG_8	  (0x01<<2)
#define DATA_LENG_9	  (0x02<<2)
#define PARITY_NONE   (0x00<<4)
#define PARITY_NC     (0x01<<4)
#define PARITY_EVEN   (0x02<<4)
#define PARITY_ODD    (0x03<<4)
#define STOP_BIT_1    (0x00<<6)
#define STOP_BIT_2	  (0x01<<6)
#define MODE_32K      (0x01<<7)
#define EXT_CTS_EN    (0x01<<9)
#define INT_CTS_EN    (0x01<<10)
#define SYNC_EN       (0x01<<11)
#define CLK_POL       (0x01<<12)
#define SYNC_MS       (0x01<<14)
#define LOOPBACK      (0x01<<15)

/* UART Control register */
#define TXBRK_EN      (0x01<<1)
#define ADDR_DET      (0x01<<2)
#define TXDIS         (0x01<<6)
#define CC            (0x01<<8)
#define CCCLR         (0x01<<9)

/* UART status register bit definition. */
#define RXRDY         (0x01<<0)
#define RXIDLE        (0x01<<1)
#define TXRDY         (0x01<<2)
#define TXIDLE        (0x01<<3)
#define CTS           (0x01<<4)
#define CTS_DELTA     (0x01<<5)
#define TXINT_DIS     (0x01<<6)

#define OVRN_ERR      (0x01<<8)
#define RXBRK         (0x01<<10)
#define DELTA_RXBRK   (0x01<<11)
#define START_DETECT  (0x01<<12)
#define FRM_ERR       (0x01<<13)
#define PAR_ERR       (0x01<<14)
#define RXNOISE       (0x01<<15)

void uart_init(LPC_USART_TypeDef *UARTx, uint32_t Baudrate);
void uart_write(const char c);
int  uart_available(void);
char uart_read(void);
int  uart_readline(char *s , uint16_t len, uint32_t tmout);
void uart_print(const char *s);
void uart_writeln(void);
void uart_println(const char *s);
void uart_clear(void);
void uart_print_dec(uint8_t d);
void uart_print_hex(uint8_t d);
#ifdef __cplusplus
}
#endif

#endif /* UART_H_ */
