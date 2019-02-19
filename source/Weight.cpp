#include "serial.h"
#include<iostream>
#include<string>
#include<map>
#include <chrono>
#include <thread>

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

int main()
{
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
			std::cout<<output<<std::endl;
			std::cout<<"/dev/ttyUSB"+std::to_string(i)<<std::endl;
		}
		
	}
    }
    /*for(std::map<std::string,std::string>::iterator it=usbtotty.begin(); it!=usbtotty.end();++it)
    {   
	for(unsigned int i=0;i!=usbtotty.size();++i)
	{ 
		std::string command="dmesg | grep ttyUSB"+std::to_string(i)+" | tail -1";
		std::string output=exec(command.c_str());
        	std::cout<<output<<std::endl;
		if(output.find(it->first)!=std::string::npos) 
		{
			//std::size_t found = output.find("ttyUSB");
			//std::cout<<"/dev/ttyUSB"+output[found+7];
        		//std::cout<<it->second<<std::endl;
		}
	}
    }*/
    //std::cout<<output<<std::endl;
    std::map<std::string,serial::Serial> weights;
    //std::vector<std::string> Names{"SF6","ISOBUTHANE","FREON","N2"};
    //std::vector<std::string> Ports{"/dev/ttyUSB0","/dev/ttyUSB1","/dev/ttyUSB2","/dev/ttyUSB3"};
    for(std::map<std::string,std::string>::iterator it=usbtotty.begin();it!=usbtotty.end();++it)
    {
	weights[usbtobottle[it->first]].setPort(it->second);
	weights[usbtobottle[it->first]].setBaudrate(9600);
        weights[usbtobottle[it->first]].open();
	if(weights[usbtobottle[it->first]].isOpen()) std::cout << " Yes." << std::endl;
  	else std::cout << " No." << std::endl;

    }
    //serial::Serial ("/dev/ttyUSB0",9600, serial::Timeout::simpleTimeout(500));
    //serial::Serial my_serial("/dev/ttyUSB1",9600, serial::Timeout::simpleTimeout(500));
    //serial::Serial my_serial("/dev/ttyUSB2",9600, serial::Timeout::simpleTimeout(500));
    //serial::Serial my_serial("/dev/ttyUSB3",9600, serial::Timeout::simpleTimeout(500));
    //std::cout << "Is the serial port open?";
  //if(my_serial.isOpen())
   // std::cout << " Yes." << std::endl;
  //else
//std::cout << " No." << std::endl;
   while(1)
   {
	for(std::map<std::string,serial::Serial>::iterator it=weights.begin();it!=weights.end();++it)
    	{
       		std::string buffer;
       		it->second.read(buffer,13);
       		std::cout<<it->first<<" : "<<buffer<<std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));
   }
}
