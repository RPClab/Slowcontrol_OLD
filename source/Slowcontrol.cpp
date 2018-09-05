#include "ConfigReader.hpp"
#include "Database.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <set>
#include <fstream>
#include <string.h>
#include<vector>
#include<cmath>
#include<string>
#include<cstdlib>
#include <iomanip>
#include "bme280/bme280.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

template<typename T>
std::string to_stringN(const T& value, const int&n=3)
{
	std::ostringstream out;
	out<<std::setprecision(n)<<std::fixed<<value;
	return std::move(out.str());
}

int fd;

int8_t user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  write(fd, &reg_addr,1);
  read(fd, data, len);
  return 0;
}

void user_delay_ms(uint32_t period)
{
  usleep(period*1000);
}

int8_t user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
  int8_t *buf;
  buf = (int8_t*)malloc(len +1);
  buf[0] = reg_addr;
  memcpy(buf +1, data, len);
  write(fd, buf, len +1);
  free(buf);
  return 0;
}

void print_sensor_data(struct bme280_data *comp_data)
{
#ifdef BME280_FLOAT_ENABLE
  printf("temp %0.2f, p %0.2f, hum %0.2f\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#else
  printf("temp %ld, p %ld, hum %ld\r\n",comp_data->temperature, comp_data->pressure, comp_data->humidity);
#endif
}

bme280_data stream_sensor_data_forced_mode(struct bme280_dev *dev)
{
  int8_t rslt;
  uint8_t settings_sel;
  struct bme280_data comp_data;
  /* Recommended mode of operation: Indoor navigation */
  dev->settings.osr_h = BME280_OVERSAMPLING_1X;
  dev->settings.osr_p = BME280_OVERSAMPLING_16X;
  dev->settings.osr_t = BME280_OVERSAMPLING_2X;
  dev->settings.filter = BME280_FILTER_COEFF_16;
  settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
  rslt = bme280_set_sensor_settings(settings_sel, dev);
  rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, dev);
  /* Wait for the measurement to complete and print data @25Hz */
  dev->delay_ms(40);
  rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, dev);
  return comp_data;
}

int main(int argc,char **argv)
{
    struct bme280_dev dev;
    int8_t rslt = BME280_OK;
    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) 
    {
        printf("Failed to open the i2c bus %s", argv[1]);
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, 0x76) < 0) 
    {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }
    dev.dev_id = BME280_I2C_ADDR_PRIM;
    dev.intf = BME280_I2C_INTF;
    dev.read = user_i2c_read;
    dev.write = user_i2c_write;
    dev.delay_ms = user_delay_ms;
    rslt = bme280_init(&dev);
    //Read ConfigFile
    ConfigReader conf("Slowcontrol","Database");
    //Connect to Database
    Database database(conf.getParameters());
    database()->connect();
    ConfigReader opt("Slowcontrol","Options");
    long time=opt.getParameter("Wait").Long();
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
            bme280_data meas=stream_sensor_data_forced_mode(&dev);
            pressure.push_back(meas.pressure/10000.0);
            temperature.push_back(meas.temperature/100.0);
            humidity.push_back(meas.humidity/1000.0);
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
       
        std::string string4 = "VALUES (0,\""+tim.str()+"\","+to_stringN(mean_pressure)+","+to_stringN(std_pressure)+","+to_stringN(mean_temperature)+","+to_stringN(std_temperature)+","+to_stringN(mean_humidity)+","+to_stringN(std_humidity)+")";
        
        std::string com= string1 + string2 + string3 + string4;
        database()->execute(com);
        std::cout<<tim.str()<<", Mean Pressure : "<<to_stringN(mean_pressure)<<" (Std : "<<to_stringN(std_pressure)<<"), Mean Temperature : "<<to_stringN(mean_temperature)<<" (Std : "<<to_stringN(std_temperature)<<"), Mean Humidity : "<<to_stringN(mean_humidity)<<" (Std : "<<to_stringN(std_humidity)<<")"<<std::endl;
        std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
        long rest=time*1000000-std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
        if(rest<=0) std::this_thread::sleep_for(std::chrono::seconds(time));
	else std::this_thread::sleep_for(std::chrono::microseconds(rest));
	}
	database()->disconnect();
	return 0;
}
