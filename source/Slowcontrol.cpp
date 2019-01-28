#include "ConfigReader.hpp"
#include "Database.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <set>
#include <fstream>
#include<vector>
#include<cmath>
#include<string>
#include<cstdlib>
#include <iomanip>
#include "bme280/bme280.hpp"
#include "bme280/I2C.hpp"
#include "bme280/data.hpp"
template<typename T>
std::string to_stringN(const T& value, const int&n=3)
{
	std::ostringstream out;
	out<<std::setprecision(n)<<std::fixed<<value;
	return std::move(out.str());
}

class bme_i2c
{
public:
    bme_i2c():m_i2c("/dev/i2c-1","0x76"),m_bme280(m_i2c,m_setting)
    {

    }
    void setIterations(const unsigned int& i)
    {
        iter=i;
    }
    void reload()
    {
        m_bme280=bme280(m_i2c,m_setting);
        temperature.reserve(iter);
        pressure.reserve(iter);
        humidity.reserve(iter);
    }
    void setI2C(const std::string& path,const std::string& adress)
    {
        m_i2c=I2C(path,adress);
        
    }
    void setSetting(const std::string& P,const std::string& H, const std::string& T,const std::string& F)
    {
        m_setting.setOversamplingPressure(P);
        m_setting.setOversamplingHumidity(H);
        m_setting.setOversamplingTemperature(T);
        m_setting.setFilterCoefficient(F);
    }
    void setNbrIteration(const unsigned int& i)
    {
        
    }
    void stream_sensor_data_forced_mode()
    {
        int8_t rslt;
        uint8_t settings_sel;
        settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;
        rslt = m_bme280.bme280_set_sensor_settings(settings_sel);
        rslt = m_bme280.bme280_set_sensor_mode(BME280_FORCED_MODE);
        /* Wait for the measurement to complete and print data @25Hz */
        m_bme280.delay_ms(40);
        rslt = m_bme280.bme280_get_sensor_data(BME280_ALL);
        data dat=m_bme280.getData();
        temperature.push_back(dat.getTemperature()/100.0);
        pressure.push_back(dat.getPressure()/10000.0);
        humidity.push_back(dat.getHumidity()/1024.0);
    }
    void Calculations()
    {
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            mean_pressure+=pressure[i];
            mean_temperature+=temperature[i];
            mean_humidity+=humidity[i];
        }
        mean_temperature/=iter;
        mean_pressure/=iter;
        mean_humidity/=iter;
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            var_temperature=(temperature[i]-mean_temperature)*(temperature[i]-mean_temperature);
            var_pressure   =(pressure[i]-mean_pressure)*(pressure[i]-mean_pressure);
            var_humidity   =(humidity[i]-mean_humidity)*(humidity[i]-mean_humidity);
        }
        var_pressure=std::sqrt(var_pressure/(iter-1));
        var_humidity=std::sqrt(var_humidity/(iter-1));
        var_temperature=std::sqrt(var_temperature/(iter-1));
    }
    double getMeanTemperature()
    {
        return mean_temperature;
    }
    double getMeanPressure()
    {
        return mean_pressure;
    }
    double getMeanHumidity()
    {
        return mean_humidity;
    }
    double getStdTemperature()
    {
        return var_temperature;
    }
    double getStdPressure()
    {
        return var_pressure;
    }
    double getStdHumidity()
    {
        return var_humidity;
    }
    void init()
    {
        m_i2c.connect();
        m_bme280.bme280_init();
    }
    void setID(const int& i)
    {
        m_id=i;
    }
    int getID()
    {
        return m_id;
    }
private:
    I2C m_i2c;
    settings m_setting;
    bme280 m_bme280;
    std::vector<double> temperature;
    std::vector<double> pressure;
    std::vector<double> humidity;
    double mean_pressure=0;
    double mean_temperature=0;
    double mean_humidity=0;
    double var_temperature=0;
    double var_pressure=0;
    double var_humidity=0;
    unsigned int iter{50};
    int m_id{0};
};

int main(int argc,char **argv)
{
    std::vector<bme_i2c> bme;
    ConfigReader opt("Slowcontrol","CommonOptions");
    long time=opt.getParameter("Wait").Long();
    unsigned int iterations=opt.getParameter("NbrIterations").UInt();
    for(unsigned int i=0;i!=opt.getParameter("NbrSensor").UInt();++i)
    {
        ConfigReader op("Slowcontrol","Sensor"+std::to_string(i+1));
        bme.push_back(bme_i2c());
        bme[i].setI2C(opt.getParameter("Device").String(),opt.getParameter("Adress").String());
        bme[i].setSetting(opt.getParameter("OversamplingPressure").String(),opt.getParameter("OversamplingHumidity").String(),opt.getParameter("OversamplingTemperature").String(),opt.getParameter("FilterCoefficient").String());
        bme[i].setID(opt.getParameter("ID").Int());
        bme[i].setNbrIteration(iterations);
        bme[i].reload();
        bme[i].init();
    }
    //Read ConfigFile
    ConfigReader conf("Slowcontrol","Database");
    //Connect to Database
    Database database(conf.getParameters());
    database()->connect();
    std::string string1 = "INSERT INTO ";
    std::string string2 = database.getName()+"."+database.getTable();
    std::string string3 = " (sensor,date,pressure,std_pressure,temperature,std_temperature,humidity,std_humidity) ";
	while(1)
	{
        std::chrono::high_resolution_clock::time_point t1=std::chrono::high_resolution_clock::now();
		std::time_t ti = ::time(nullptr);
		mariadb::date_time tim(ti);
        for(unsigned int i=0;i!=iterations;++i)
        {
            for(unsigned int j=0;j!=bme.size();++j)
            {
                bme[j].stream_sensor_data_forced_mode();
            }
        }
        for(unsigned int j=0;j!=bme.size();++j)
        {
            bme[j].Calculations();
            std::string string4 = "VALUES ("+std::to_string(bme[j].getID())+",\""+tim.str()+"\","+to_stringN(bme[j].getMeanPressure())+","+to_stringN(bme[j].getStdPressure())+","+to_stringN(bme[j].getMeanTemperature())+","+to_stringN(bme[j].getStdTemperature())+","+to_stringN(bme[j].getMeanHumidity())+","+to_stringN(bme[j].getStdHumidity())+")";
            std::string com= string1 + string2 + string3 + string4;
            database()->execute(com);
            std::cout<<bme[j].getID()<<"  "<<tim.str()<<", Mean Pressure : "<<to_stringN(bme[j].getMeanPressure())<<" (Std : "<<to_stringN(bme[j].getStdPressure())<<"), Mean Temperature : "<<to_stringN(bme[j].getMeanTemperature())<<" (Std : "<<to_stringN(bme[j].getStdTemperature())<<"), Mean Humidity : "<<to_stringN(bme[j].getMeanHumidity())<<" (Std : "<<to_stringN(bme[j].getStdHumidity())<<")"<<std::endl;
        }
        
        std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
        long rest=time*1000000-std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
        if(rest<=0) std::this_thread::sleep_for(std::chrono::seconds(time));
        else std::this_thread::sleep_for(std::chrono::microseconds(rest));
	}
	database()->disconnect();
	return 0;
}
