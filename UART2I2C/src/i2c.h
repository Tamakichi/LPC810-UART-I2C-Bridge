/*
 * i2c.h
 *
 *  Created on: 2015/04/06
 *      Author: たま吉さん
 *  Modified on: 2016/07/17, i2c_setTimeOut(uint32_t t)の追加
*/

#ifndef I2C_H_
#define I2C_H_


#include "LPC8xx.h"
#include "lpc_types.h"

#ifdef __cplusplus
extern "C"
{
#endif


#define RD_BIT              0x01

#define I2C_FMODE_PLUS_PRE_DIV	(9-1)		/* 1MHz */
#define I2C_FMODE_PRE_DIV		(23-1)		/* 400KHz */
#define I2C_SMODE_PRE_DIV		(90-1)		/* 100KHz */

#define CFG_MSTENA			(1 << 0)
#define CFG_SLVENA			(1 << 1)
#define CFG_MONENA			(1 << 2)
#define CFG_TIMEOUTENA		(1 << 3)
#define CFG_MONCLKSTR		(1 << 4)

#define CTL_MSTCONTINUE		(1 << 0)
#define CTL_MSTSTART		(1 << 1)
#define CTL_MSTSTOP 		(1 << 2)
#define CTL_SLVCONTINUE		(1 << 0)
#define CTL_SLVNACK			(1 << 1)

#define TIM_MSTSCLLOW(d)	((d) << 0)
#define TIM_MSTSCLHIGH(d)	((d) << 4)

#define STAT_MSTPEND  		(1 << 0)
#define MASTER_STATE_MASK	(0x7<<1)
#define STAT_MSTIDLE	 	(0x0 << 1)
#define STAT_MSTRX	 		(0x1 << 1)
#define STAT_MSTTX	 		(0x2 << 1)
#define STAT_MSTNACKADDR	(0x3 << 1)
#define STAT_MSTNACKTX		(0x4 << 1)
#define STAT_MSTARBLOSS		(1 << 4)
#define STAT_MSTSSERR	 	(1 << 6)
#define STAT_MST_ERROR_MASK	(MSTNACKADDR|STAT_MSTNACKTX|STAT_MSTARBLOSS|STAT_MSTSSERR)

#define STAT_SLVPEND 		(1 << 8)
#define SLAVE_STATE_MASK	(0x3<<9)
#define STAT_SLVADDR		(0x0 << 9)
#define STAT_SLVRX  	 	(0x1 << 9)
#define STAT_SLVTX  	 	(0x2 << 9)
#define STAT_SLVNOTSTR		(1 << 11)
#define STAT_SLVSEL		 	(1 << 14)
#define STAT_SLVDESEL		(1 << 15)

#define STAT_MONRDY			(1 << 16)
#define STAT_MONOVERRUN 	(1 << 17)
#define STAT_MONACTIVE		(1 << 18)
#define STAT_MONIDLE		(1 << 19)

#define STAT_EVTIMEOUT		(1 << 24)
#define STAT_SCLTIMEOUT		(1 << 25)

#define I2C_ERR_TIMEOUT		-1	// タイムアウト
#define I2C_ERR_SLVNACK		-2	// スレーブからNACKを受信
#define I2C_ERR_ADDRSND		-3  // I2Cアドレス送信エラー(NACK)
#define I2C_ERR_DATASND		-4  // スレーブにデータ送信でNACKを受信
#define I2C_ERR_DATARCV		-5  // データ受信失敗

extern void i2c_setTimeOut(uint32_t t);
extern bool i2c_waitReady();
extern int i2c_msend(uint32_t addr, uint8_t *tx, uint32_t Length );
extern int i2c_mreceive(uint32_t addr, uint8_t *rx, uint32_t Length );
extern int i2c_msendRcv(uint32_t addr, uint8_t *tx, uint32_t txlen, uint8_t *rx, uint32_t rxlen );
extern void i2c_init(void);
extern uint8_t i2c_send(uint8_t i2c_addr, uint8_t ctrl , uint8_t data );

#ifdef __cplusplus
}
#endif

#endif /* I2C_H_ */
