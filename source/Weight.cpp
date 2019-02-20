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
#include <cmath>

class weight
{
public:
    void parse(const std::string arg)
    {
        // The format for Gross Weight : ww000.000kg or ww000.000lb
        // The format for Net Weight : wn000.000kg or wn000.000lb
        // So :
        //std::cout<<arg<<"*****"<<arg.size()<<std::endl;
	if(arg.size()!=12) return;
        m_value=std::stof(arg.substr(2,6))/(std::pow(10,std::stoi(arg.substr(8,1))));
        std::cout<<m_value<<std::endl;
        //std::string unit=arg.substr(8,1);
        //std::cout<<value<<"  "<<unit<<std::endl;        
	//if(arg[1]!='w'&&arg[1]!='n') return;
   	//else if(arg[1]=='n')  m_isNet=true;
        
        //m_value=arg.substr(2,7);
        //if(arg.substr(9,2)=="lb") convert();
	try
	{
		m_value.Double();
		m_isgood=true;
	}
        catch(...)
        {
        }
	//std::cout<<arg<<"***"<<m_value<<std::endl;
    }
    Value getWeight()
    {
        return m_value;
    }
    bool isNet()
    {
        return m_isNet;
    }
    bool isGood()
    {
        return m_isgood;
    }
private:
    void convert()
    {
        m_value=m_value.Double()*m_lbTOkg;
    }
    Value m_value{""};
    bool m_isNet{false};
    // Always put kg even is balance is in lb;
    constexpr static double m_lbTOkg{0.45359237};
    bool m_isgood{false};
};


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

Value checklastentry(const std::string& name, Database& dat)
{
    std::string query = "SELECT * FROM "+dat.getName()+"."+dat.getTable()+" WHERE date=(SELECT MAX(date) FROM "+dat.getName()+"."+dat.getTable()+" WHERE gas=\""+name+"\") AND gas=\""+name+"\";";
    //std::cout<<query<<std::endl;
    mariadb::result_set_ref result = dat()->query(query);
    //std::cout<<"Result"<<result->row_count()<<std::endl;
    if(result->row_count()==0) return Value("");
    else
    {
            result->set_row_index(0);
            return Value(result->get_double(2));
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
    ////// Effet de bords : Check if at list one entry exist for gas name in database else add one and say it,s new bottle !!!
    std::time_t ti= ::time(nullptr);
    mariadb::date_time tim(ti);
    for(std::map<std::string,serial::Serial>::iterator it=weights.begin();it!=weights.end();++it)
    {
        if(checklastentry(it->first,database).String()=="")
        {
            weight wei;
            do
            {
                std::string buffer;
                it->second.read(buffer,13);
                wei.parse(buffer);
            }
            while(wei.isGood()==false);
            std::string command=std::string("INSERT INTO "+database.getName()+"."+database.getTable()+" (date,gas,weight,net_weight,new_bottle) VALUES (\"")+tim.str()+"\",\""+it->first+"\","+std::to_string(wei.getWeight().Double())+","+(wei.isNet() ? std::string("TRUE") : std::string("FALSE"))+",TRUE"+std::string(");");
            //std::cout<<command<<std::endl;
            database()->execute(command);
            
        }
    }
   while(1)
   {
    int nbriteration=10;
    std::time_t ti= ::time(nullptr);
    mariadb::date_time tim(ti);
	for(std::map<std::string,serial::Serial>::iterator it=weights.begin();it!=weights.end();++it)
    {
        double value=0;
        for(unsigned int i=0;i!=nbriteration;++i)
        {
            // Access New Value :
            weight wei;
            do
            {
                std::string buffer;
                it->second.read(buffer,13);
                wei.parse(buffer);
            }
            while(wei.isGood()==false);
            double last =checklastentry(it->first,database).Double();
            double neww =wei.getWeight().Double();
            std::cout<<last<<"  "<<neww<<"  "<<(neww-last)/last*100<<" "<<0.05<<std::endl;
            if((neww-last)/last>=0.05)
            {
                if(i==0)
                {
                    std::cout<<"New Bottle of "<<it->first<<" suspected "<<std::endl;
                    std::cout<<"Trying to check if it's true ! "<<std::endl;
                }
                std::cout<<"Iteration nbr "<<i<<"/"<<nbriteration<<std::endl;
            }
            else
            {
                ti= ::time(nullptr);
                tim=ti;
                std::string command=std::string("INSERT INTO "+database.getName()+"."+database.getTable()+" (date,gas,weight,net_weight,new_bottle) VALUES (\"")+tim.str()+"\",\""+it->first+"\","+std::to_string(neww)+","+(wei.isNet() ? std::string("TRUE") : std::string("FALSE"))+",FALSE"+std::string(");");
                 std::cout<<command<<std::endl;
                database()->execute(command);
                break;
            }
            if(i==nbriteration-1)
            {
                ti=::time(nullptr);
                tim=ti;
                std::string command=std::string("INSERT INTO "+database.getName()+"."+database.getTable()+" (date,gas,weight,net_weight,new_bottle) VALUES (\"")+tim.str()+"\",\""+it->first+"\","+std::to_string(wei.getWeight().Double())+","+(wei.isNet() ? std::string("TRUE") : std::string("FALSE"))+",TRUE"+std::string(");");
                std::cout<<command<<std::endl;
                database()->execute(command);
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
	}
	std::this_thread::sleep_for(std::chrono::seconds(600));
   }
}
