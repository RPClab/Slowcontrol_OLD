#include "serial.h"
#include<iostream>
#include<string>
#include<map>
#include <chrono>
#include <thread>
#include "ConfigReader.hpp"
#include "Database.hpp"
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char* cmd) {
    std::array<char,5000> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::string checklastentry(const std::string& name, Database& dat)
{
    std::string query = "SELECT * FROM "+dat.getName()+"."+dat.getTable()+" WHERE date=(SELECT MAX(date) FROM "+dat.getName()+"."+dat.getTable()+" WHERE gas="+name+") AND gas="+name+";";
    mariadb::result_set_ref result = dat()->query(query);
    if(result->row_count()==0) return "";
    else
    {
            result->set_row_index(0);
            return result->get_string(2);
    }
}



int main()
{
    //Connect to Database 
    ConfigReader conf("Weight","Database");
    Database database(conf.getParameters());
    database()->connect();
    /////////////////////////////////////////////////////////////
    //Find witch USB to witch tty
    std::map<std::string,std::string> usbtobottle {{"1-1.1.2","FREON"},{"1-1.3","SF6"},{"1-1.1.3","ISOBUTHANE"},{"1-1.2","N2"}};
    std::map<std::string,std::string> usbtotty {{"1-1.1.2",""},{"1-1.3",""},{"1-1.1.3",""},{"1-1.2",""}};
    for(unsigned int i=0;i!=100;++i)
    {
        std::string command="dmesg | grep ttyUSB"+std::to_string(i)+" | tail -1";
        std::string output=exec(command.c_str());
        for(std::map<std::string,std::string>::iterator it=usbtotty.begin(); it!=usbtotty.end();++it)
        {
            if(output.find(it->first)!=std::string::npos) 
            {
                it->second="/dev/ttyUSB"+std::to_string(i);
                //std::cout<<output<<std::endl;
                //std::cout<<"/dev/ttyUSB"+std::to_string(i)<<std::endl;
            }
		
        }
    }
    // Create and open Serial connections
    std::map<std::string,serial::Serial> weights;
    for(std::map<std::string,std::string>::iterator it=usbtotty.begin();it!=usbtotty.end();++it)
    {
        weights[usbtobottle[it->first]].setPort(it->second);
        weights[usbtobottle[it->first]].setBaudrate(9600);
        weights[usbtobottle[it->first]].open();
        if(weights[usbtobottle[it->first]].isOpen()) 
        {
            std::cout << " Yes." << std::endl;
        }
        else std::cout << " No." << std::endl;
    }
   while(1)
   {

	for(std::map<std::string,serial::Serial>::iterator it=weights.begin();it!=weights.end();++it)
    	{
            std::cout<<"****"<<checklastentry(usbtobottle[it->first],database)<<"***"<<std::endl;
       		std::string buffer;
       		it->second.read(buffer,13);
       		std::cout<<it->first<<" : "<<buffer<<std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
   }
}
