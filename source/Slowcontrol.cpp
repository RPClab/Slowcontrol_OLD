#include "ConfigReader.hpp"
#include "Database.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <set>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soft_i2c.h"
#include<vector>
#include<cmath>
#include<string>
#include<cstdlib>
#include <chrono>

typedef uint8_t byte; 
uint8_t I2C_bme280=0x76;

// #define DEBUG

// BST-BME280-DS001-11 | Revision 1.2 | October 2015 Bosch Sensortec

/* singed integer type*/
typedef	int8_t s8;/**< used for signed 8bit */
typedef	int16_t s16;/**< used for signed 16bit */
typedef	int32_t s32;/**< used for signed 32bit */
typedef	int64_t s64;/**< used for signed 64bit */

typedef	uint8_t u8;/**< used for unsigned 8bit */
typedef	uint16_t u16;/**< used for unsigned 16bit */
typedef	uint32_t u32;/**< used for unsigned 32bit */
typedef	uint64_t u64;/**< used for unsigned 64bit */

u16 dig_T1;/**<calibration T1 data*/
s16 dig_T2;/**<calibration T2 data*/
s16 dig_T3;/**<calibration T3 data*/
u16 dig_P1;/**<calibration P1 data*/
s16 dig_P2;/**<calibration P2 data*/
s16 dig_P3;/**<calibration P3 data*/
s16 dig_P4;/**<calibration P4 data*/
s16 dig_P5;/**<calibration P5 data*/
s16 dig_P6;/**<calibration P6 data*/
s16 dig_P7;/**<calibration P7 data*/
s16 dig_P8;/**<calibration P8 data*/
s16 dig_P9;/**<calibration P9 data*/

u8	dig_H1;/**<calibration H1 data*/
s16 dig_H2;/**<calibration H2 data*/
u8	dig_H3;/**<calibration H3 data*/
s16 dig_H4;/**<calibration H4 data*/
s16 dig_H5;/**<calibration H5 data*/
s8	dig_H6;/**<calibration H6 data*/

s32 t_fine;/**<calibration T_FINE data*/
	
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
int32_t BME280_compensate_T_int32(int32_t adc_T)
{
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) *
	((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BME280_compensate_P_int64(int32_t adc_P)
{
	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0) return 0; // avoid exception caused by division by zero
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);
	return (uint32_t)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32_t bme280_compensate_H_int32(int32_t adc_H)
{
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) +
	((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r *
	((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
	((int32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
}

int _bme280_setByte(byte reg, byte data)
{
		byte config[2];
		config[0]=reg;
		config[1]=data;
		return !i2c_write(I2C_bme280,config,2);
}

uint16_t _bme280_getReg(byte reg)
{
		byte data;
		i2c_write(I2C_bme280,&reg,1);	   // 書込みの実行
		delay(1);
		i2c_read(I2C_bme280,&data,1);	   // 読み出し
		return (int)data;
}

void _bme280_cal()
{
	dig_T1 = (u16)(_bme280_getReg(0x88) + (_bme280_getReg(0x89)<<8));
	dig_T2 = (s16)(_bme280_getReg(0x8A) + (_bme280_getReg(0x8B)<<8));
	dig_T3 = (s16)(_bme280_getReg(0x8C) + (_bme280_getReg(0x8D)<<8));
	dig_P1 = (u16)(_bme280_getReg(0x8E) + (_bme280_getReg(0x8F)<<8));
	dig_P2 = (s16)(_bme280_getReg(0x90) + (_bme280_getReg(0x91)<<8));
	dig_P3 = (s16)(_bme280_getReg(0x92) + (_bme280_getReg(0x93)<<8));
	dig_P4 = (s16)(_bme280_getReg(0x94) + (_bme280_getReg(0x95)<<8));
	dig_P5 = (s16)(_bme280_getReg(0x96) + (_bme280_getReg(0x97)<<8));
	dig_P6 = (s16)(_bme280_getReg(0x98) + (_bme280_getReg(0x99)<<8));
	dig_P7 = (s16)(_bme280_getReg(0x9A) + (_bme280_getReg(0x9B)<<8));
	dig_P8 = (s16)(_bme280_getReg(0x9C) + (_bme280_getReg(0x9D)<<8));
	dig_P9 = (s16)(_bme280_getReg(0x9E) + (_bme280_getReg(0x9F)<<8));
	dig_H1 = (u8)(_bme280_getReg(0xA1));
	dig_H2 = (s16)(_bme280_getReg(0xE1) + (_bme280_getReg(0xE2)<<8));
	dig_H3 = (u8)(_bme280_getReg(0xE3));
	dig_H4 = (s16)((_bme280_getReg(0xE4)<<4) + (_bme280_getReg(0xE5)&0x0F));
	dig_H5 = (s16)(((_bme280_getReg(0xE5)&0xF0)>>4) + (_bme280_getReg(0xE6)<<4));
	dig_H6 = (s8)(_bme280_getReg(0xE7));
}

float bme280_getTemp()
{
	int32_t in;
	in = _bme280_getReg(0xFA);			  		// temp_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xFB); 				// temp_lsb[7:0]
	in <<= 4;
	in |= _bme280_getReg(0xFC); 				// temp_xlsb[3:0]
//	  printf("getTemp  %08X %d\n",in,in);
	return ((float)BME280_compensate_T_int32(in))/100.;
}


float bme280_getHum()
{
	int32_t in;
	in = _bme280_getReg(0xFD);			  		// hum_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xFE); 				// hum_lsb[7:0]
//	printf("getHum   %08X\n",in);
	return ((float)bme280_compensate_H_int32(in))/1024.;
}

float bme280_getPress()
{
	int32_t in;
	in = _bme280_getReg(0xF7);					// press_msb[7:0]
	in <<= 8;
	in |= _bme280_getReg(0xF8); 	 			// press_lsb[7:0]
	in <<= 4;
	in |= _bme280_getReg(0xF9);				// press_xlsb[3:0]
//	printf("getPress %08X\n",in);
	return ((float)BME280_compensate_P_int64(in))/25600.;
}

int bme280_init()
{
	byte reg,data,in;
	int i;
    i2c_init();
	
	_bme280_cal();
	
	reg= 0xF5;				   	// config
//	data=0b11000000;
	data=0b00000000;
	//	   | || | |___________________ 触るな SCI切換え
	//	   | ||_|_____________________ filter[2:0]
	//	   |_|________________________ t_sb[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
			fprintf(stderr,"ERROR(11): i2c writing config reg\n");
		return 11;
	}
	
	reg= 0xF2;				   	// trl_hum
	data=0b00000001;
	//			|_|___________________ osrs_h[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
			fprintf(stderr,"ERROR(12): i2c writing trl_hum reg\n");
		return 12;
	}
	
	reg= 0xF4;				   	// ctrl_meas
	data=0b00100111;
	//	   | || |||___________________ mode[1:0]
	//	   | ||_|_____________________ osrs_p[2:0]
	//	   |_|________________________ osrs_t[2:0]
	if(_bme280_setByte(reg,data)){		// 書込みの実行
			fprintf(stderr,"ERROR(13): i2c writing ctrl_meas reg\n");
		return 13;
	}	 
	in=_bme280_getReg(0xD0);
	if(in != 0x58 && in != 0x60){
			fprintf(stderr,"ERROR(21):  chip_id (%02X)\n",in);
		return 21;
	}
	for(i=0;i<50;i++){
		in=_bme280_getReg(0xF3);
		#ifdef DEBUG
				printf("getReg   %02X\n",in);
		#endif
		if((in&0x04)==0) break;
		delay(20);
	}
	if(i==50){
			fprintf(stderr,"ERROR(31): failed to read results\n");
		return 31;
	}
	return 0;
}

int bme280_stop()
{
	byte reg,data;
	int ret;
	reg= 0xF4;					   	// ctrl_meas
	data=0x00;
	ret=_bme280_setByte(reg,data);	// 書込みの実行
    i2c_close();
	return ret;
}

void bme280_print(float temp, float hum, float press)
{
		printf("Temperature : %3.2f C",temp);
		printf("Humidity : %3.2f \%",hum);
		printf("Pressure : %4.2f\n hPa",press);
}


int main(int argc,char **argv)
{
    //Read ConfigFile
    ConfigReader conf("Slowcontrol","Database");
    //conf.print();
    //Connect to Database
    Database database(conf.getParameters());
    database()->connect();
    ConfigReader opt("Slowcontrol","Options");
    long time=opt.getParameter("Wait").Long();
    
	//mariadb::account_ref Myaccount= mariadb::account::create("10.42.0.120","sjturpc","RPC2018");
	//mariadb::connection_ref Myconnection=mariadb::connection::create(Myaccount);

    
    if( argc == 2 ) I2C_bme280=(byte)strtol(argv[1],NULL,16);
	if( I2C_bme280>=0x80 ) I2C_bme280>>=1;
	if( argc < 1 || argc > 2 ){
		fprintf(stderr,"usage: %s [I2C_bme280]\n",argv[0]);
		return -1;
	}
	#ifdef DEBUG
		printf("I2C_bme280 =0x%02X\n",I2C_bme280);
	#endif

	bme280_init();
    
     std::string string1 = "INSERT INTO ";
     std::string string2 = database.getName()+"."+database.getTable();
     std::string string3 = " (sensor,date,pressure,std_pressure,temperature,std_temperature,humidity,std_humidity) ";
    
	while(1)
	{
        std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
		std::time_t ti = ::time(nullptr);
		mariadb::date_time tim(ti);
        
        int iter=50;
        
        std::vector<double>pressure;
        pressure.reserve(iter);
        
	    std::vector<double>temperature;
	    temperature.reserve(iter);
        
        std::vector<double>humidity;
        humidity.reserve(iter);
        
        for(unsigned int i=0;i!=iter;++i)
        {
            pressure.push_back(bme280_getPress());
            humidity.push_back(bme280_getHum());
            temperature.push_back(bme280_getTemp());
        }
	
        double mean_pressure=0;
        double mean_temperature=0;
        double mean_humidity=0;
	
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            mean_pressure+=pressure[i];
            mean_temperature+=temperature[i];
            mean_humidity+=humidity[i];
        }
        mean_temperature/=iter;
        mean_pressure/=iter;
        mean_humidity/=iter;
	
        double var_temperature=0;
        double var_pressure=0;
        double var_humidity=0;
	
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            var_temperature=(temperature[i]-mean_temperature)*(temperature[i]-mean_temperature);
            var_pressure   =(pressure[i]-mean_pressure)*(pressure[i]-mean_pressure);
            var_humidity   =(humidity[i]-mean_humidity)*(humidity[i]-mean_humidity);
        }
        double std_pressure=std::sqrt(var_pressure/(iter-1));
        double std_humidity=std::sqrt(var_humidity/(iter-1));
        double std_temperature=std::sqrt(var_temperature/(iter-1));
        
        char press[20],std_press[20],temp[20],std_temp[20],humid[20],std_humid[20];
        std::string strp,strsp,strt,strst,strh,strsh;
        
        gcvt(mean_pressure,6,press);
        strp = press;
        gcvt(std_pressure,5,std_press);
        strsp = std_press;
        
        gcvt(mean_temperature,6,temp);
        strt = temp;
        gcvt(std_temperature,5,std_temp);
        strst = std_temp;
        
        gcvt(mean_humidity,6,humid);
        strh = humid;
        gcvt(std_humidity,5,std_humid);
        strsh = std_humid;
        
       
        std::string string4 = "VALUES (0,\""+tim.str()+"\","+strp+","+strsp+","+strt+","+strst+","+strh+","+strsh+")";
        
        std::string com= string1 + string2 + string3 + string4;
       // std::cout<<com<<std::endl;
        database()->execute(com);
        std::cout<<"Mean Pressure : "<<strp<<" ( Std : "<<strsp<<"), Mean Temperature : "<<strt<<" ( Std : "<<strst<<"), Mean Humidity : "<<strh<<" ( Std : "<<strsh<<")"<<std::endl;
        std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
        long rest=time*1000000-std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
        if(rest<=0) std::this_thread::sleep_for(std::chrono::seconds(time));
	else std::this_thread::sleep_for(std::chrono::microseconds(rest));
	}
	  database()->disconnect();
	  bme280_stop();
	return 0;
}
