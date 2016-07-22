/*
 *  i2c.c
 *  LPC810 LPCXpresso I2C用関数 Author: たま吉さん
 *  Created  on: 2015/04/06
 *  Modified on: 2015/04/18
 *  Modified on: 2016/07/17, i2c_setTimeOut()追加,i2c_waitReady()のタイムアウト時間を変更可能にする
 *  Modified on: 2016/07/19, I2C プルアップ、オープンドレイン設定ミスの修正
 *
 */

#include "LPC8xx.h"
#include "i2c.h"
#include "uart.h"
#include "mrt.h"
#include "lpc_types.h"

static uint32_t _timeout = 500;

// I2Cタイムアウト設定
void i2c_setTimeOut(uint32_t t) {
	_timeout = t;
}

// I2C送受信レディ待ち
// 500msecでタイムアウト
bool i2c_waitReady() {
	uint32_t cnt = mrt_read_counter()+_timeout; // 2016/07/17 変更
	while(!(LPC_I2C->STAT & STAT_MSTPEND)) {
		if (mrt_read_counter() > cnt) {
			return false;
		}
	}
	return true;
}

// I2Cアドレス送信
int i2c_write_address(uint32_t addr) {
  // レディ待ち
  if (i2c_waitReady() == false) {
	  return I2C_ERR_TIMEOUT;
  }

  // START + I2Cアドレス送信
  LPC_I2C->MSTDAT = addr;
  LPC_I2C->MSTCTL = CTL_MSTSTART;
  if (i2c_waitReady() == false) {
	  // タイムアウト（処理を終了する)
	  LPC_I2C->MSTCTL = CTL_MSTSTOP;
	  return I2C_ERR_TIMEOUT;
  }

  // スレーブからのアドレス送信 ACK/NACKのチェック
  if((LPC_I2C->STAT & MASTER_STATE_MASK) == STAT_MSTNACKADDR) {
	  return I2C_ERR_ADDRSND; // (CTL_MSTSTOPは自動送信される)
  }
  return 0;
}

// 1バイト送信
int i2c_write_byte(uint8_t b) {

	// 送信可能か?
	if(!(LPC_I2C->STAT & STAT_MSTTX)) {
			LPC_I2C->MSTCTL = CTL_MSTSTOP;
			return I2C_ERR_DATASND;
	 }

	// 1バイト送信
	LPC_I2C->MSTDAT = b;
	LPC_I2C->MSTCTL = CTL_MSTCONTINUE; // 処理を継続

	if (i2c_waitReady() == false) {
	  // タイムアウト（処理を終了する)
	  LPC_I2C->MSTCTL = CTL_MSTSTOP;
	  return I2C_ERR_TIMEOUT;
	}

	// スレーブからのデータ送信ACK/NACKのチェック
	if((LPC_I2C->STAT & MASTER_STATE_MASK) == STAT_MSTNACKTX)
		return I2C_ERR_DATASND; // (CTL_MSTSTOPは自動送信される)

	return 0;
}

// 1バイト受信
int i2c_read_byte(bool flgrpt) {
	int rc;

	// データ受信済チェック
	if (i2c_waitReady() == false) {
	  // タイムアウト（処理を終了する)
	  LPC_I2C->MSTCTL = CTL_MSTSTOP;
	  return I2C_ERR_TIMEOUT;
	}

	// データ受信可能?
	if(!(LPC_I2C->STAT & STAT_MSTRX)) {
		LPC_I2C->MSTCTL = CTL_MSTSTOP;
		return I2C_ERR_DATARCV;
	}

	// データ受信
	rc = LPC_I2C->MSTDAT;

	// 継続受信?
	if (flgrpt) {
		LPC_I2C->MSTCTL = CTL_MSTCONTINUE;
		if (i2c_waitReady() == false) {
		  LPC_I2C->MSTCTL = CTL_MSTSTOP;
		  return I2C_ERR_TIMEOUT;
		}
	}
	return rc;
}

//
// I2Cマルチバイトデータ送信
//   addr  : i2cスレーブアドレス(8ビット)
//   tx    : 送信データ格納アドレス
//   Length: 送信データサイズ
// 戻り値   0:正常終了   0以外:異常終了
//
int i2c_msend(uint32_t addr, uint8_t *tx, uint32_t Length ) {
  uint32_t i;
  int rc;

  // アドレス送信(START+I2Cスレーブアドレス送信、ACK/NACK受信、タイムアウト)
  if ( (rc = i2c_write_address(addr)) )
	  return rc;

  // データ送信(データ送信、ACK/NACK受信、タイムアウト)
  for ( i = 0; i < Length; i++ ) {
	  if ( (rc = i2c_write_byte(tx[i])) ) {
		  return rc;
	  }
  }

  // 処理の終了（ストップコンディション）
  LPC_I2C->MSTCTL = CTL_MSTSTOP;
  if (i2c_waitReady() == false) {
	  return I2C_ERR_TIMEOUT;
  }
  return 0;
}

// コマンド送信(2バイトデータ送信)
uint8_t i2c_send(uint8_t i2c_addr, uint8_t ctrl , uint8_t data ) {
	uint8_t sndbuf[2];
	sndbuf[0] = ctrl;
	sndbuf[1] = data;
	return i2c_msend(i2c_addr, sndbuf, 2 );
}

//
// I2Cマルチバイトデータ受信
//  addr : i2cスレーブアドレス
//  rx   : 受信データ格納アドレス
// Length: 受信データ長さ
// 戻り値   0:正常終了   0以外:異常終了
// 補足 アドレスにはR/Wビットをつけないこと。関数内部で設定する
//
int i2c_mreceive(uint32_t addr, uint8_t *rx, uint32_t rxlen ) {
  uint32_t i;
  int rc;

  // アドレス送信(START+I2Cスレーブアドレス送信、ACK/NACK受信、タイムアウト)
  if ( (rc = i2c_write_address(addr|RD_BIT)) )
	  return rc;

  // データ受信(受信、ACK/NACK送信)
  for ( i = 0; i < rxlen; i++ ) {
	  if ( (rc = i2c_read_byte(i != rxlen -1)) < 0 )
		  return rc;
	  *rx++ = (uint8_t)rc;
  }

  // 処理の終了（ストップコンディション）
  LPC_I2C->MSTCTL = CTL_MSTSTOP;
  if (i2c_waitReady() == false) {
	  return I2C_ERR_TIMEOUT;
  }

  return 0;
}

//
// I2Cマルチバイトデー送受信
//  addr　: i2cスレーブアドレス
//  tx   : 送信データ格納アドレス
//  ｔｘｌｅｎ　: 送信データサイズ
//  rx   : 受信データ格納アドレス
//  rxlen: 受信データ長さ
// 戻り値   0:正常終了   0以外:異常終了
// 補足 アドレスにはR/Wビットをつけないこと。関数内部で設定する
//
int i2c_msendRcv(uint32_t addr, uint8_t *tx, uint32_t txlen, uint8_t *rx, uint32_t rxlen ) {

  uint32_t i;
  int rc;

  // アドレス送信(START+I2Cスレーブアドレス送信、ACK/NACK受信、タイムアウト)
  if ( (rc = i2c_write_address(addr)) )
	  return rc;

  // データ送信(データ送信、ACK/NACK受信、タイムアウト)
  for ( i = 0; i < txlen; i++ ) {
	  if ( (rc = i2c_write_byte(tx[i])) ) {
		  return rc;
	  }
  }

  // アドレス送信(START+I2Cスレーブアドレス送信、ACK/NACK受信、タイムアウト)
  if ( (rc = i2c_write_address(addr|RD_BIT)) )
	  return rc;

  // データ受信(受信、ACK/NACK送信)
  for ( i = 0; i < rxlen; i++ ) {
	  if ( (rc = i2c_read_byte(i != rxlen -1)) < 0 )
		  return rc;
	  *rx++ = (uint8_t)rc;
  }

  // 処理の終了（ストップコンディション）
  LPC_I2C->MSTCTL = CTL_MSTSTOP;
  if (i2c_waitReady() == false) {
	  return I2C_ERR_TIMEOUT;
  }
  return 0;
}

//
// I2C初期化
//  LPC810用(呼び出し前に スイッチマトリックスで PIN02:SDA , PIN03:SCLとすること)
//  通信速度 100kbps,割り込みは利用していない
//
void i2c_init() {
#if LPC810
	// オープンドレイン、プルアップ設定(LPC810のPIN2,PIN3をI2Cに設定) 2016/07/19 修正
	LPC_IOCON->PIO0_2 |= 0x410;
	LPC_IOCON->PIO0_3 |= 0x410;
#else
	// オープンドレイン、プルアップ設定(LPC812MAX設定)
	LPC_IOCON->PIO0_10 |= 0x410;
	LPC_IOCON->PIO0_11 |= 0x410;
#endif
	// クロック供給開始
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<5);

	// I2Cリセット
	LPC_SYSCON->PRESETCTRL &= ~(0x1<<6);
	LPC_SYSCON->PRESETCTRL |= (0x1<<6);

	// 条件設定(割り込みは使わない)
	LPC_I2C->DIV = I2C_SMODE_PRE_DIV;
	LPC_I2C->CFG &= ~(CFG_MSTENA);
	LPC_I2C->MSTTIME = TIM_MSTSCLLOW(0x00) | TIM_MSTSCLHIGH(0x00);
	NVIC_DisableIRQ(I2C_IRQn);
	LPC_I2C->CFG |= CFG_MSTENA;

	i2c_setTimeOut(500);	// 2016/07/17 追加
}
