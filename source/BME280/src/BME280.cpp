#include "BME280.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <tuple>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
extern "C"
{
    #include "bme280/bme280.h"
}

int BME280::m_fd{0};

int8_t BME280::user_i2c_read(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    write(m_fd, &reg_addr,1);
    read(m_fd, data, len);
    return 0;
}
    
void BME280::user_delay_ms(uint32_t period)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(period));
}
    
int8_t BME280::user_i2c_write(uint8_t id, uint8_t reg_addr, uint8_t *data, uint16_t len)
{
    int8_t *buf;
    buf = static_cast<int8_t*>(malloc(len +1));
    buf[0] = reg_addr;
    memcpy(buf +1, data, len);
    write(m_fd, buf, len +1);
    free(buf);
    return 0;
}

void BME280::init()
{
    m_dev.dev_id = BME280_I2C_ADDR_PRIM;
    m_dev.intf = BME280_I2C_INTF;
    m_dev.read =user_i2c_read;
    m_dev.write = user_i2c_write;
    m_dev.delay_ms = user_delay_ms;
    /* Recommended mode of operation: Indoor navigation */
    m_dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    m_dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    m_dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    m_dev.settings.filter = BME280_FILTER_COEFF_16;
    m_settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
    m_rslt = bme280_set_sensor_settings(m_settings_sel,&m_dev);
}

void BME280::setBus(const std::string& bus)
{
    m_bus=bus;
}
    
std::string BME280::getBus()
{
    return m_bus;
}
    
void BME280::connect()
{
    if ((m_fd = open(m_bus.c_str(), O_RDWR)) < 0) 
    {
        std::cout<<"Failed to open the i2c bus "<<m_bus<<" !"<<std::endl;
        std::exit(1);
    }
    if (ioctl(m_fd, I2C_SLAVE, 0x76) < 0) 
    {
        std::cout<<"Failed to acquire bus access and/or talk to slave.\n"<<std::endl;
        exit(1);
    }
    m_rslt = bme280_init(&m_dev);
}

std::tuple<int32_t,uint32_t,uint32_t> BME280::getMeasures()
{
    struct bme280_data comp_data;
    /* Continuously stream sensor data */
    m_rslt = bme280_set_sensor_mode(BME280_FORCED_MODE,&m_dev);
    /* Wait for the measurement to complete and print data @25Hz */
    m_dev.delay_ms(40);
    m_rslt = bme280_get_sensor_data(BME280_ALL, &comp_data,&m_dev);
    return std::make_tuple(comp_data.pressure,comp_data.temperature,comp_data.humidity);
}
