/*******************************************************************************
Raspberry Pi用 ソフトウェアI2C ライブラリ  soft_i2c

本ソースリストおよびソフトウェアは、ライセンスフリーです。(詳細は別記)
利用、編集、再配布等が自由に行えますが、著作権表示の改変は禁止します。

Arduino標準ライブラリ「Wire」は使用していない(I2Cの手順の学習用サンプル)

                               			Copyright (c) 2014-2017 Wataru KUNINO
                               			http://www.geocities.jp/bokunimowakaru/
*******************************************************************************/

//	通信の信頼性確保のため、戻り値の仕様を変更・統一しました。
//	ヘッダファイルも変更しています。ご理解のほど、お願いいたします。
//	0:成功 1:失敗
//														2017/6/16	国野亘

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>						// uint32_t
#include <unistd.h>         			// usleep用
#include <ctype.h>						// isprint用
#include <sys/time.h>					// gettimeofday用
#include <string.h>						// strncpy用

#define I2C_lcd 0x3E							// LCD の I2C アドレス 
#define PORT_SCL	"/sys/class/gpio/gpio3/value"		// I2C SCLポート
#define PORT_SDA	"/sys/class/gpio/gpio2/value"		// I2C SDAポート
#define PORT_SDANUM	2									// I2C SDAポートの番号
														// SCLはSDA+1(固定)
#define INPUT		"in"
#define OUTPUT		"out"
#define LOW			0
#define HIGH		1
#define	I2C_RAMDA	15					// I2C データシンボル長[us]
#define GPIO_RETRY  50      			// GPIO 切換え時のリトライ回数
#define S_NUM       16       			// 文字列の最大長
//	#define DEBUG               		// デバッグモード

typedef unsigned char byte; 
FILE *fgpio;
char buf[S_NUM];
struct timeval micros_time;				//time_t micros_time;
int micros_prev,micros_sec;
int ERROR_CHECK=1;								// 1:ACKを確認／0:ACKを無視する
static byte _lcd_size_x=8;
static byte _lcd_size_y=2;

int _micros(){
	int micros;
	gettimeofday(&micros_time, NULL);    // time(&micros_time);
	micros = micros_time.tv_usec;
	if(micros_prev > micros ) micros_sec++;
	micros_prev = micros;
	micros += micros_sec * 1000000;
	return micros;
}

void _micros_0(){
	micros_sec=0;
}

void _delayMicroseconds(int i){
	usleep(i);
}

void delay(int i){
	while(i){
		_delayMicroseconds(1000);
		i--;
	}
}

void i2c_debug(const char *s,byte priority){
	if(priority>3)	fprintf(stderr,"[%10d] ERROR:%s\n",_micros(),s);
    #ifdef DEBUG
	else 			fprintf(stderr,"[%10d]      :%s\n",_micros(),s);
    #endif
}

void i2c_error(const char *s){
	i2c_debug(s,5);
}
void i2c_log(const char *s){
	i2c_debug(s,1);
}

byte pinMode(char *port, char *mode){
// 戻り値：０の時はエラー
	int i=0;
	char dir[]="/sys/class/gpio/gpio3/direction";
	         // 0123456789012345678901234567890
	dir[20]=port[20];
    #ifdef DEBUG
    //	fprintf(stderr,"pinMode %s %s\n",dir,mode);
    #endif
	while(i<GPIO_RETRY){
		fgpio = fopen(dir, "w");
		if(fgpio){
			fprintf(fgpio,mode);
			fclose(fgpio);
			return 1;
		}
		delay(1);
		i++;
	}
    #ifdef DEBUG
    //	fprintf(stderr,"pinMode / GPIO_RETRY (%d/%d)\n",i,GPIO_RETRY);
    #endif
    return 0;
}

byte digitalRead(char *port){
// 戻り値：０の時はエラー
    fgpio = fopen(port, "r");
	if( fgpio ){
	    fgets(buf, S_NUM, fgpio);
	    fclose(fgpio);
	}
    #ifdef DEBUG
    //	fprintf(stderr,"digitalRead %s %s\n",port,buf);
    #endif
    return (byte)atoi(buf);
}

byte digitalWrite(char *port, int value){
// 戻り値：０の時はエラー
    fgpio = fopen(port, "w");
	if( fgpio ){
	    fprintf(fgpio,"%d\n",value);
	    fclose(fgpio);
	    return 1;
	}
    #ifdef DEBUG
    //	fprintf(stderr,"digitalWrite %s %d\n",port,value);
    #endif
    return 0;
}

byte i2c_hard_reset(int port){
	// 戻り値：０の時はエラー
	FILE *pp;
	char buf[9];
	char com[]="/home/pi/RaspberryPi/gpio/raspi_gpo 00 0 &> /dev/null";
			//	01234567890123456789012345678901234567890123456789012
	if(port<1 || port>99) return 0;
	com[36] = '\0';
	sprintf(com,"%s%2d 0 &> /dev/null",com,port);
	#ifdef DEBUG
		printf("%s\n",com);
	#endif
	pp=popen(com,"r");
	if(pp){
		fgets(buf,8,pp);
		pclose(pp);
		if(buf[0] != '0'){
			i2c_error("I2C_RESET(L) / IO Settiong Error");
			return 0;
		}
	}
	delay(10);
	com[36] = '\0';
	sprintf(com,"%s%2d 1 &> /dev/null",com,port);
	#ifdef DEBUG
		printf("%s\n",com);
	#endif
	pp=popen(com,"r");
	if(pp){
		fgets(buf,8,pp);
		pclose(pp);
		if(buf[0] != '1'){
			i2c_error("I2C_RESET(H) / IO Settiong Error");
			return 0;
		}
	}
	delay(10);
	return 1;
}

byte i2c_SCL(byte level){
// 戻り値：０の時はエラー
	byte ret=0;
	if( level ){
		ret += !pinMode(PORT_SCL, INPUT);
	}else{
		ret += !pinMode(PORT_SCL, OUTPUT);
		ret += !digitalWrite(PORT_SCL, LOW);
	}
	_delayMicroseconds(I2C_RAMDA);
	return !ret;
}

byte i2c_SDA(byte level){
// 戻り値：０の時はエラー
	byte ret=0;
	if( level ){
		ret += !pinMode(PORT_SDA, INPUT);
	}else{
		ret += !pinMode(PORT_SDA, OUTPUT);
		ret += !digitalWrite(PORT_SDA, LOW);
	}
	_delayMicroseconds(I2C_RAMDA);
	return !ret;
}

byte i2c_tx(const byte in){
// 戻り値：０の時はエラー
	int i;
    #ifdef DEBUG
    	char s[32];
		sprintf(s,"tx data = [%02X]",in);
		i2c_log(s);
    #endif
	for(i=0;i<8;i++){
		if( (in>>(7-i))&0x01 ){
				i2c_SDA(1);					// (SDA)	H Imp
		}else	i2c_SDA(0);					// (SDA)	L Out
		/*Clock*/
		i2c_SCL(1);							// (SCL)	H Imp
		i2c_SCL(0);							// (SCL)	L Out
	}
	/* ACK処理 */
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp  2016/6/26 先にSDAを終わらせる
	i2c_SCL(1);								// (SCL)	H Imp
	for(i=3;i>0;i--){						// さらにクロックを上げた瞬間には確定しているハズ
		if( digitalRead(PORT_SDA) == 0 ) break;	// 速やかに確認
		_delayMicroseconds(I2C_RAMDA/2);
	}
	if(i==0 && ERROR_CHECK ){
		i2c_SCL(0);							// (SCL)	L Out
		i2c_log("no ACK");
		return 0;
	}
    #ifdef DEBUG
    //	fprintf(stderr,"i2c_tx / GPIO_RETRY (%d/%d)\n",GPIO_RETRY-i,GPIO_RETRY);
    #endif
	return (byte)i;
}

byte i2c_init(void){
// 戻り値：０の時はエラー
	int i;

	_micros_0();
	i2c_log("I2C_Init");
    for(i=0;i<2;i++){
		fgpio = fopen("/sys/class/gpio/export","w");
	    if(fgpio==NULL ){
	        i2c_error("I2C_Init / IO Settiong Error\n");
	        printf("9\n");
	        return 0;
	    }
	    fprintf(fgpio,"%d\n",i + PORT_SDANUM);
	    fclose(fgpio);
	}
	for(i=GPIO_RETRY;i>0;i--){						// リトライ50回まで
		i2c_SDA(1);							// (SDA)	H Imp
		i2c_SCL(1);							// (SCL)	H Imp
		if( digitalRead(PORT_SCL)==1 &&
			digitalRead(PORT_SDA)==1  ) break;
		delay(1);
	}
	if(i==0) i2c_error("I2C_Init / Locked Lines");
    #ifdef DEBUG
    //	fprintf(stderr,"i2c_init / GPIO_RETRY (%d/%d)\n",GPIO_RETRY-i,GPIO_RETRY);
    #endif
	_delayMicroseconds(I2C_RAMDA*8);
	return (byte)i;
}

byte i2c_close(void){
// 戻り値：０の時はエラー
	byte i;
	i2c_log("i2c_close");
    for(i=0;i<2;i++){
		fgpio = fopen("/sys/class/gpio/unexport","w");
	    if(fgpio==NULL ){
	        fprintf(stderr,"IO Error\n");
	        printf("9\n");
	        return 0;
	    }
	    fprintf(fgpio,"%d\n",i + PORT_SDANUM);
	    fclose(fgpio);
	}
	return 1;
}

byte i2c_start(void){
// 戻り値：０の時はエラー
//	if(!i2c_init())return(0);				// SDA,SCL  H Out
	int i;

	for(i=5000;i>0;i--){					// リトライ 5000ms
		i2c_SDA(1);							// (SDA)	H Imp
		i2c_SCL(1);							// (SCL)	H Imp
		if( digitalRead(PORT_SCL)==1 &&
			digitalRead(PORT_SDA)==1  ) break;
		delay(1);
	}
	i2c_log("i2c_start");
	if(i==0 && ERROR_CHECK) i2c_error("i2c_start / Locked Lines");
	_delayMicroseconds(I2C_RAMDA*8);
	i2c_SDA(0);								// (SDA)	L Out
	_delayMicroseconds(I2C_RAMDA);
	i2c_SCL(0);								// (SCL)	L Out
	return (byte)i;
}

byte i2c_check(byte adr){
/*
入力：byte adr = I2Cアドレス(7ビット)
戻り値：０の時はエラー
*/
	byte ret;
	if( !i2c_start() ) {
		i2c_error("i2c_check / aborted i2c_start");
		return 0;
	}
	adr <<= 1;								// 7ビット->8ビット
	adr &= 0xFE;							// RW=0 送信モード
	ret=i2c_tx(adr);

	/* STOP */
	i2c_SDA(0);								// (SDA)	L Out
	i2c_SCL(0);								// (SCL)	L Out
	_delayMicroseconds(I2C_RAMDA);
	i2c_SCL(1);								// (SCL)	H Imp
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp
	return ret;
}


byte i2c_read(byte adr, byte *rx, byte len){
/*
入力：byte adr = I2Cアドレス(7ビット)
出力：byte *rx = 受信データ用ポインタ
入力：byte len = 受信長
戻り値：byte 受信結果長、０の時はエラー
*/
	byte ret,i;
	
	if( !i2c_start() && ERROR_CHECK) return 0;
	adr <<= 1;								// 7ビット->8ビット
	adr |= 0x01;							// RW=1 受信モード
	if( i2c_tx(adr)==0 && ERROR_CHECK ){	// アドレス設定
		i2c_error("I2C_RX / no ACK (Address)");
		return 0;		
	}
	
	/* スレーブ待機状態待ち */
	for(i=GPIO_RETRY;i>0;i--){
		_delayMicroseconds(I2C_RAMDA);
		if( digitalRead(PORT_SDA)==0  ) break;
	}
	if(i==0 && ERROR_CHECK){
		i2c_error("I2C_RX / no ACK (Reading)");
		return 0;
	}
	for(i=10;i>0;i--){
		_delayMicroseconds(I2C_RAMDA);
		if( digitalRead(PORT_SCL)==1  ) break;
	}
	if(i==0 && ERROR_CHECK){
		i2c_error("I2C_RX / Clock Line Holded");
		return 0;
	}
	/* 受信データ */
	for(ret=0;ret<len;ret++){
		i2c_SCL(0);							// (SCL)	L Out
		i2c_SDA(1);							// (SDA)	H Imp
		rx[ret]=0x00;
		for(i=0;i<8;i++){
			i2c_SCL(1);						// (SCL)	H Imp
			rx[ret] |= (digitalRead(PORT_SDA))<<(7-i);		//data[22] b4=Port 12(SDA)
			i2c_SCL(0);						// (SCL)	L Out
		}
		if(ret<len-1){
			// ACKを応答する
			i2c_SDA(0);							// (SDA)	L Out
			i2c_SCL(1);							// (SCL)	H Imp
			_delayMicroseconds(I2C_RAMDA);
		}else{
			// NACKを応答する
			i2c_SDA(1);							// (SDA)	H Imp
			i2c_SCL(1);							// (SCL)	H Imp
			_delayMicroseconds(I2C_RAMDA);
		}
	}
	/* STOP */
	i2c_SCL(0);								// (SCL)	L Out
	i2c_SDA(0);								// (SDA)	L Out
	_delayMicroseconds(I2C_RAMDA);
	i2c_SCL(1);								// (SCL)	H Imp
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp
	return ret;
}

byte i2c_write(byte adr, byte *tx, byte len){
/*
入力：byte adr = I2Cアドレス(7ビット)
入力：byte *tx = 送信データ用ポインタ
入力：byte len = 送信データ長（0のときはアドレスのみを送信する）
戻り値：０の時はエラー(または送信データ長0)
*/
	byte ret=0;
	if( !i2c_start() ) return 0;
	adr <<= 1;								// 7ビット->8ビット
	adr &= 0xFE;							// RW=0 送信モード
	if( i2c_tx(adr)>0 ){
		/* データ送信 */
		for(ret=0;ret<len;ret++){
			i2c_SDA(0);						// (SDA)	L Out
			i2c_SCL(0);						// (SCL)	L Out
			if( i2c_tx(tx[ret]) == 0 && ERROR_CHECK){
				i2c_error("i2c_write / no ACK (Writing)");
				return 0;
			}
		}
	}else if( len>0 && ERROR_CHECK){		// len=0の時はエラーにしないAM2320用
		i2c_error("i2c_write / no ACK (Address)");
		return 0;
	}
	/* STOP */
	i2c_SDA(0);								// (SDA)	L Out
	i2c_SCL(0);								// (SCL)	L Out
	_delayMicroseconds(I2C_RAMDA);
	if(len==0)_delayMicroseconds(800);		// AM2320用
	i2c_SCL(1);								// (SCL)	H Imp
	_delayMicroseconds(I2C_RAMDA);
	i2c_SDA(1);								// (SDA)	H Imp
	return ret;
}
