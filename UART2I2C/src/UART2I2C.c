/*
===============================================================================
 Name        : UART2I2C.c
 Author      : Tamakichi
 Version     : 2.00
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC8xx.h"
#endif

#define LPC810
#include "lpc_types.h"
#include "uart.h"
#include "mrt.h"
#include "i2c.h"

#define MYVERSION			"[v2.00 2016/07/19 by Tamakichi]"
#define CMD_ERR_BADCMD		-10 // コマンドエラー
#define CMD_ERR_STRHEX		-11 // 16進数文字異常
#define CMD_ERR_CMDPRM		-12 // パラメタエラー
#define CMD_ERR_KNWCMD		-13 // 未定義コマンド
#define CMD_ERR_NOCMD		-14 // 補助コマンドのみ指定
#define CMD_ERR_NODATA		-15 // コマンドのデータがない

#define I2C_ADR_SHIFT		1	// 7ビットアドレスを8ビットに変換(0で無変換 = 8ビット指定）

bool    flginf;		// I2C通信エラー通知フラグ
uint8_t modeOut;	// 受信データ出力形式
bool    flgdebug;	// デバッグ情報出力
uint8_t cmdwait;	// コマンド実行時のウエイト
bool    flgichigo;	// IchigoJam用REM(')文字を先頭付加フラグ
uint8_t modeECHO;	// ローカルエコー

void for_ichigo_rem() {
	if (flgichigo) {
		uart_write('\'');
	}
}

// 16進数ダンプ出力
void uart_dump(uint8_t* buf, int len) {
	int i;
	for (i=0; i < len; i++) {
		if (i%16 == 0) {
			for_ichigo_rem();
			uart_print_hex(i);
			uart_write(':');
		}
		uart_print_hex(buf[i]);
		if (i%16 == 15)
			uart_writeln();
		else
			uart_write(' ');
	}
	uart_writeln();
}


void SwitchMatrix_Init() {
    /* Enable SWM clock */
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

#ifdef LPC810
    /* Pin Assign 8 bit Configuration */
    /* U0_TXD */
    /* U0_RXD */
    LPC_SWM->PINASSIGN0 = 0xffff0004UL;
    /* I2C0_SDA */
    LPC_SWM->PINASSIGN7 = 0x02ffffffUL;
    /* I2C0_SCL */
    LPC_SWM->PINASSIGN8 = 0xffffff03UL;

    /* Pin Assign 1 bit Configuration */
    /* RESET */
    LPC_SWM->PINENABLE0 = 0xffffffbfUL;
#else
    /* Enable SWM clock */
     LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

     /* Pin Assign 8 bit Configuration */
     /* U0_TXD */
     /* U0_RXD */
     LPC_SWM->PINASSIGN0 = 0xffff0004UL;
     /* I2C0_SDA */
     LPC_SWM->PINASSIGN7 = 0x0affffffUL;
     /* I2C0_SCL */
     LPC_SWM->PINASSIGN8 = 0xffffff0bUL;

     /* Pin Assign 1 bit Configuration */
     /* SWCLK */
     /* SWDIO */
     /* RESET */
     LPC_SWM->PINENABLE0 = 0xffffffb3UL;

#endif
}



// 2桁16進数文字列を1バイト整数に変換する
// 変換エラーの場合、-1を返す
int hex2val(char *text) {
	uint8_t val = 0;
	if (*text >= '0' && *text <= '9') {
		val= *text - '0';
	} else if (*text >= 'a' && *text <= 'f') {
		val= *text - 'a' + 10;
	} else if (*text >= 'A' && *text <= 'F') {
		val= *text - 'A' + 10;
	} else {
		return -1;
	}
	val<<=4;
	text++;
	if (*text >= '0' && *text <= '9') {
		val+= *text - '0';
	} else if (*text >= 'a' && *text <= 'f') {
		val+= *text - 'a' + 10;
	} else if (*text >= 'A' && *text <= 'F') {
		val+= *text - 'A' + 10;
	} else {
		return -1;
	}
	return (int)val;
}

//
// i2c_cmdのデータの表示(デバッグ用)
//
void debug_uartToi2c(int n, uint8_t* i2c_cmd,char *text) {
	int i;
	for_ichigo_rem();
	uart_print("[text="); uart_print(text);
	uart_print(" len=");  uart_print_dec(n); //uart_writeln();
	if (n >0) {
		uart_print(" cmd=");  uart_write(i2c_cmd[0]); uart_write(' ');
		for (i=1; i < n; i++) {
			uart_print_hex(i2c_cmd[i]); uart_print(" ");
		}
	}
	uart_println("]");
}

//
// I2Cバスに接続しているスレーブのアドレス（7ビット）を表示する
//
void i2cdetect(uint8_t mode) {
	uint8_t i;
	uint8_t dummy;
	int rc;

	// タイムアウト時間を一時的に100msecに変更
	i2c_setTimeOut(100);

	// 上ヘッダーの出力
	if (mode) {
		for_ichigo_rem();
		uart_print("   ");
		for (i=0; i < 16 ; i++) {
			uart_print_hex(i); uart_print(" ");
		}
		uart_writeln();
	}

	// スキャン結果の出力(スキャン範囲 0x08～0x77)
	for (i=0; i<128; i++) {
		if (!(i & 0x0f) && mode) {
			// 左ヘッダ表示
			for_ichigo_rem();
			uart_print_hex(i & 0xf0 );
			uart_print(" ");
		}
		if (i < 0x08 || i > 0x77 ) {
			if (mode)
				uart_print("  ");
		} else {
			rc = i2c_msend(i<<I2C_ADR_SHIFT, &dummy, 0);
			if (!rc) {
				uart_print_hex(i);
			} else {
				if (mode)
					uart_print("--");
			}
		}
		if (mode)
			uart_print(" ");

		if ( ((i & 0x0f) == 0x0f) && mode) {
			uart_writeln(); // 1行改行
		}
	}
	uart_writeln(); // 1行改行

	// タイムアウト時間を一時的に500msecに戻す
	i2c_setTimeOut(500);

}

//
// I2Cコマンド発行
//
int execI2Ccmd(uint8_t* i2c_cmd, int len,uint8_t* rcv) {
	int i;
	int rc = 0;
	bool flgexec = false;
	if (i2c_cmd[0] == 'w') {
		// 一括送信コマンドの実行
		rc = i2c_msend(i2c_cmd[1]<<I2C_ADR_SHIFT, &i2c_cmd[2], len-2);
		flgexec = true;
	} else if (i2c_cmd[0] == 'r') {
		// 送受信コマンドの実行
		rc = i2c_msendRcv(i2c_cmd[1]<<I2C_ADR_SHIFT, &i2c_cmd[3], len-3, rcv, i2c_cmd[2]);
		flgexec = true;
	} else if (i2c_cmd[0] == 'c') {
		// 1バイト単位送信コマンドの実行
		for (i=3; i <len; i++) {
			rc = i2c_send(i2c_cmd[1]<<I2C_ADR_SHIFT, i2c_cmd[2] , i2c_cmd[i]);
			if (rc) break;
			if (cmdwait>0)
				delay(cmdwait);
		}
		flgexec = true;
	} else if (i2c_cmd[0] == 'g') {
		// 受信コマンドの実行
		rc = i2c_mreceive(i2c_cmd[1]<<I2C_ADR_SHIFT, rcv, i2c_cmd[2]);
		flgexec = true;
	} else if (i2c_cmd[0] == 'd') {
		// ダミーコマンドの実行
		rc = 0;
		flgexec = true;
	}
	if (flgexec) {
		if (flginf) {
			// 実行結果の出力
			rc=0-rc;
			for_ichigo_rem();
   	       	uart_print_hex((uint8_t)rc);
   	       	uart_writeln();
		}
		if ((i2c_cmd[0]=='r' || i2c_cmd[0]=='g' ) && !rc) {
			// 受信データの出力(正常終了時のみ)
			if ( modeOut == 0) {
				// HEX出力
				for_ichigo_rem();
				for (i=0; i < i2c_cmd[2]; i++) {
		   	       	uart_print_hex(rcv[i]);
				}
	   	       	uart_writeln();
			} else if ( modeOut == 1 ){
				// バイナリ出力
				for_ichigo_rem();
				for (i=0; i < i2c_cmd[2]; i++) {
		   	       	uart_write(rcv[i]);
				}
			} else {
				// ダンプ出力
				uart_dump(rcv, i2c_cmd[2]);
			}
		}
	}
	return rc;
}

//
// テキストコマンドをバイナリ変換
//  @wXX@hYYZZ・・ 改行     :データ送信(一括)     XX:I2Cアドレス(16進数2桁), YYZZ・・ :送信データ(16進数2桁可変長さ)
//  @cXXCC@hYYZZ・・ 改行 :データ送信(1バイトづつ) XX:I2Cアドレス(16進数2桁), CC:コントロールコード,YYZZ・・ :送信データ(16進数2桁可変長さ)
//  @rXXNN@hYYZZ・・改行 :データ送受信  XX:I2Cアドレス(16進数2桁), NN：受信データ長(16進数2桁) ,YYZZ・・ :送信データ(16進数2桁可変長さ)
//  @gXXNN改行                   :データ受信 XX:I2Cアドレス(16進数2桁), NN：受信データ長(16進数2桁)
//  @eN改行                          :エラー通知設定 N:0or1 0:通知なし(デフォルト) 1:通知あり
//  @oC改行                          :受信データ出力モード設定 0:HEX(デフォルト) 1:バイナリ
//  @DN改行                          :デバッグ情報表示 0:OFF 1:ON
//  @tCC改行                       :データ送信(1バイトづつ)のウエイト時間(msec)指定 00 ～FF
//  ※ @dの代わりに @sSSSSS・・文字列 も指定可能,@dYYZZ@sHello!と混ぜることも可能
//
int text2cmd(uint8_t* i2c_cmd, char *text) {
	bool flgcmd = false;
	bool flgdata = false;
	int i = 0;
	int val;

	i2c_cmd[0] = 0;
	if (*text!='@') {
		return 0;
	}

	while ( *text ) {
		if (*text == 0x0a || *text == 0x0d )
			break; // テキストがnul, 改行コード
		if (*text != '@')
			return CMD_ERR_BADCMD; // 先頭が@でない場合は処理を終了する
		text++;

		// 補助コマンドの処理
		if ( *text=='e') { //エラー通知設定コマンド
			text++;
			if (*text == '0') {
				flginf = false;
			} else if (*text == '1') {
				flginf = true;
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='o') { //出力形式設定コマンド
			text++;
			if (*text >= '0' && *text <= '2') {
				modeOut = *text - '0';
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='D') { //DEBUG情報出力
			text++;
			if (*text == '0') {
				flgdebug = false;
			} else if (*text == '1') {
				flgdebug = true;
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='n') { //NewLine(改行)指定
			text++;
			if (*text == '0') {
				uart_setNewLine(0);
			} else if (*text == '1') {
				uart_setNewLine(1);
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='v') {
			for_ichigo_rem();
			uart_println(MYVERSION);
			text++;
			continue;
		}
		if ( *text=='x') {	// I2Cスレーブ調査
			text++;
			if (*text == '0') {
				i2cdetect(0);
			} else if (*text == '1') {
				i2cdetect(1);
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='t') { //ディレイ設定
			val = hex2val(++text); // 引数（時間）取得
			if ( val < 0 ) {
				return CMD_ERR_CMDPRM; // 変換エラー
			}
			cmdwait = val;
			text+=2;
			continue;
		}
		if ( *text=='T') { //即時ディレイ実行
			val = hex2val(++text); // 引数（時間）取得
			if ( val < 0 ) {
				return CMD_ERR_CMDPRM; // 変換エラー
			}
			delay(val);
			text+=2;
			continue;
		}
		if ( *text=='i') { //IchigoJam対応
			text++;
			if (*text == '0') {
				flgichigo = false;
			} else if (*text == '1') {
				flgichigo = true;
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		if ( *text=='l') { //ローカルエコー設定
			text++;
			if (*text == '0') {
				modeECHO = 0;
			} else if (*text == '1') {
				modeECHO = 1;
			} else { // パラメタエラー
				return CMD_ERR_CMDPRM;
			}
			text++;
			continue;
		}
		// コマンドの処理
		if ( !flgcmd ) {
			if ( *text=='w') { //一括送信コマンド
				val = hex2val(++text); // アドレス取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]='w'; i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				flgcmd=true;
			} else if ( *text=='c' ) { //1バイト単位送信コマンド
				val = hex2val(++text); // アドレス取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]='c'; i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				val = hex2val(text); //コントロールコード取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				flgcmd=true;
			} else if ( *text=='r' ) { //送受信コマンド
				val = hex2val(++text); // アドレス取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]='r'; i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				val = hex2val(text); // 受信データ長取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				flgcmd=true;
			} else if ( *text=='g' ) { //受信コマンド
				val = hex2val(++text); // アドレス取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]='g'; i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				val = hex2val(text); // 受信データ長取得
				if ( val < 0 ) {
					return CMD_ERR_BADCMD; // 変換エラー
				}
				i2c_cmd[i++]= (uint8_t)val;
				text+=2;
				flgcmd = true;
				flgdata = true;
			} else if ( *text=='d' ) { //ダミーコマンド
				i2c_cmd[i++]='d';
				text++;
				flgcmd = true;
				flgdata = true;
			} else {
				return CMD_ERR_KNWCMD; // コマンド指定エラー
			}
		} else {
			// コマンドデータ部の処理
			if (*text == 'h')  {// 16進数のデータ指定
				text++;
				while (1) {
					val = hex2val(text); // データ取得
					if ( val < 0 ) {
						return CMD_ERR_BADCMD; // 変換エラー
					}
					i2c_cmd[i++]= (uint8_t)val;
					text+=2;
					if ( *text == '@' || *text <32) {
						flgdata = true;
						break;
					}
				}
			} else if (*text == 's') { //文字列のデータ
				text++;
				while (1) {
					if ( *text == '@' || *text <32) {
						flgdata = true;
						break;
					}
					i2c_cmd[i++]= *text++;
				}
			}
		}
	}

	if (i==0) {
		return 0;
	}
	// コマンド整合性チェック
	if (!flgcmd) {
		return CMD_ERR_NOCMD; // コマンド指定エラー
	}
	if (!flgdata) {
		return CMD_ERR_NODATA; // データ指定エラー
	}
	return i; // コマンドデータ長を返す
}

//
// シリアル通信・I2Cブリッジ処理
//
int doUART2I2C() {
	int  n;
	int len;
	int rc = 0;

	char text[128];			// UART受信データ
	uint8_t i2c_cmd[64];	// I2Cコマンドデータ
	uint8_t rcv[256];		// I2C受信データ

	cmdwait  = 0;			// 1バイト単位送信時のウエイト時間msec

	// シリアルポートからデータの取得
	n = uart_readline(text,128,0, modeECHO);
	if (n>0) {
		// テキストの解析
		len = text2cmd(i2c_cmd,text);
		if (len <0) {
			// コマンドエラー
			if (flginf) {
				// 実行結果の出力
				len=0-len;
				for_ichigo_rem();
				uart_print_hex((uint8_t)len);
				uart_writeln();
			}
			return -1;
		}
		if (flgdebug==true)
			debug_uartToi2c(len,i2c_cmd,text);

		// コマンドの実行
		rc = execI2Ccmd(i2c_cmd, len, rcv);
	}
	return rc;
}

int main(void) {
	// システム初期設定
	SystemCoreClockUpdate();		// クロック設定
	SwitchMatrix_Init() ;			// スイッチマトリックス設定
	mrt_init();						// タイマーカウンター初期化
	uart_init(LPC_USART0,115200);	// UART0の初期化
	i2c_init();						// I2C初期化

	// 初期値設定
	flginf   = true;		// エラー情報を出力する
	modeOut  = 0;			// 16進数HEXテキスト
	flgdebug = false;		// デバッグ情報出力フラグ
	flgichigo = false;		// IchigoJam用REM(')付加フラグ
	uart_setNewLine(0);		// 改行形式（CR+LF)
	modeECHO = 0;			// ローカルエコーなし

	while(1) {
		doUART2I2C();
    }
    return 0 ;
}
