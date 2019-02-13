#include "ConfigReader.hpp"
#include "Database.hpp"
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <set>
#include<vector>
#include<cmath>
#include<string>
#include <iomanip>
#include "bme280/bme280.hpp"
#include "bme280/I2C.hpp"

template<typename T>
std::string to_stringN(const T& value, const int&n=3)
{
	std::ostringstream out;
	out<<std::setprecision(n)<<std::fixed<<value;
	return std::move(out.str());
}


void print_sensor_data(const data& dat)
{
#ifdef BME280_FLOAT_ENABLE
  printf("temp %0.2f, p %0.2f, hum %0.2f\r\n",dat.getTemperature(), dat.getPressure(), dat.getHumidity());
#else
  printf("temp %ld, p %ld, hum %ld\r\n",dat.getTemperature(),dat.getPressure(),dat.getHumidity());
#endif
}

class Val
{
public:
    Val(const int& iter,const int& id,const std::string name):m_iter(iter),m_id(id),m_name(name)
    {
    }
    void resetValues()
    {
        pressure.clear();
        temperature.clear();
        humidity.clear();
        mean_pressure=0;
        mean_temperature=0;
        mean_humidity=0;
        std_temperature=0;
        std_pressure=0;
        std_humidity=0;
    }
    int getID()
    {
        return m_id;
    }
    std::string getName()
    {
        return m_name;
    }
    void CalculateMeans()
    {
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            mean_pressure+=pressure[i];
            mean_temperature+=temperature[i];
            mean_humidity+=humidity[i];
        }
        mean_temperature/=m_iter;
        mean_pressure/=m_iter;
        mean_humidity/=m_iter;
    }
    void CalculateStd()
    {
        for(unsigned int i=0;i!=temperature.size();++i)
        {
            std_temperature=(temperature[i]-mean_temperature)*(temperature[i]-mean_temperature);
            std_pressure   =(pressure[i]-mean_pressure)*(pressure[i]-mean_pressure);
            std_humidity   =(humidity[i]-mean_humidity)*(humidity[i]-mean_humidity);
        }
        std_pressure=std::sqrt(std_pressure/(m_iter-1));
        std_humidity=std::sqrt(std_humidity/(m_iter-1));
        std_temperature=std::sqrt(std_temperature/(m_iter-1));
    }
    double MeanTemperature()
    {
        return mean_temperature;
    }
    
    double MeanPressure()
    {
        return mean_pressure;
    }
    
    double MeanHumidity()
    {
        return mean_humidity;
    }
    
    
    double StdTemperature()
    {
        return std_temperature;
    }
    
    
    double StdPressure()
    {
        return std_pressure;
    }
    
    double StdHumidity()
    {
        return std_humidity;
    }
    void readData(const data& dat)
    {
        pressure.push_back(dat.getPressure());
        temperature.push_back(dat.getTemperature());
        humidity.push_back(dat.getHumidity());
    }
    
private:
    std::string m_name{""};
    int m_id{0};
    int m_iter{0};
    std::vector<double>pressure;
    std::vector<double>temperature;
    std::vector<double>humidity;
    double mean_pressure{0};
    double mean_temperature{0};
    double mean_humidity{0};
    double std_temperature{0};
    double std_pressure{0};
    double std_humidity{0};
};

int main(int argc,char **argv)
{
    settings setting;
    std::vector<Val> Vals;
    std::vector<I2C> i2cs;
    std::vector<bme280> bme280s;
    //Read Global Options :
    ConfigReader opt("Slowcontrol","GlobalOptions");
    setting.setOversamplingPressure(opt.getParameter("OversamplingPressure").String());
    setting.setOversamplingHumidity(opt.getParameter("OversamplingHumidity").String());
    setting.setOversamplingTemperature(opt.getParameter("OversamplingTemperature").String());
    setting.setFilterCoefficient(opt.getParameter("FilterCoefficient").String());
    int NbrSensors{opt.getParameter("NbrSensors").Int()};
    long time=opt.getParameter("Wait").Long();
    int iteration{opt.getParameter("Iteration").Int()};
    // Read Sensors Options and Initialize them :
    for(std::size_t sen=0;sen!=NbrSensors;++sen)
    {
        ConfigReader opt("Slowcontrol","Sensor_"+std::to_string(sen+1));
        std::cout<<opt.getParameter("Device").String()<<" "<<opt.getParameter("Adress").String()<<std::endl;
        i2cs.emplace_back(opt.getParameter("Device").String(),opt.getParameter("Adress").String());
        i2cs[sen].connect();
        bme280s.emplace_back(i2cs[sen],setting);
        bme280s[sen].init();
        Vals.emplace_back(iteration,opt.getParameter("ID").Int(),opt.getParameter("Name").String());
        for(unsigned int j=0;j!=100;++j)
        {
            print_sensor_data(bme280s[sen].getDataForcedMode());
        }
    }
    //Read Database Options :
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
        for(unsigned int i=0;i!=iteration;++i)
        {
            for(unsigned int j=0;j!=NbrSensors;++j)
            {
                data dat=bme280s[j].getDataForcedMode();
                Vals[j].readData(dat);
            }
        }
        for(unsigned int j=0;j!=NbrSensors;++j)
        {
            Vals[j].CalculateMeans();
            Vals[j].CalculateStd();
            std::string string4 = "VALUES ("+std::to_string(Vals[j].getID())+",\""+tim.str()+"\","+to_stringN(Vals[j].MeanPressure())+","+to_stringN(Vals[j].StdPressure())+","+to_stringN(Vals[j].MeanTemperature())+","+to_stringN(Vals[j].StdTemperature())+","+to_stringN(Vals[j].MeanHumidity())+","+to_stringN(Vals[j].StdHumidity())+")";
            std::string com= string1 + string2 + string3 + string4;
            database()->execute(com);
            std::cout<<"Sensor : "<<Vals[j].getName()<<", ID : "<<Vals[j].getID()<<", Time : "<<tim.str()<<", Mean Pressure : "<<to_stringN(Vals[j].MeanPressure())<<" (Std : "<<to_stringN(Vals[j].StdPressure())<<"), Mean Temperature : "<<to_stringN(Vals[j].MeanTemperature())<<" (Std : "<<to_stringN(Vals[j].StdTemperature())<<"), Mean Humidity : "<<to_stringN(Vals[j].MeanHumidity())<<" (Std : "<<to_stringN(Vals[j].StdHumidity())<<")"<<std::endl;
            Vals[j].resetValues();
        }
        std::chrono::high_resolution_clock::time_point t2=std::chrono::high_resolution_clock::now();
        long rest=time*1000000-std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count();
        if(rest<=0) std::this_thread::sleep_for(std::chrono::seconds(time));
        else std::this_thread::sleep_for(std::chrono::microseconds(rest));
	}
	database()->disconnect();
	return 0;
}
