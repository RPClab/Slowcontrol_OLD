/*
 * Copyright (c) 2018 Lagarde François lagarde.at.sjtu.edu.cn
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef CONFIGREADER_H
#define CONFIGREADER_H
#include "json/json.h"
#include "Parameters.h"
#include <iostream>
#include <fstream>
#include <string>
class ConfigReader 
{
public:
    ConfigReader(){};
    ConfigReader(const std::string& envName,const std::string& part):m_envName(envName),m_part(part)
    {
        parse();
    }
    void setEnvName(const std::string& envName){m_envName=envName;}
    void setPart(const std::string& part){ m_part=part;}
    std::string getPart(){return m_part;}
    std::string getEnvName(){return m_envName;}
    void parse()
    {
        Json::Value root=OpenJSONFile(m_envName);
        ExtractData(root);
    }
    void print()
    {
        m_params.printParameters();
    }
    const Parameters& getParameters() const { return m_params;}
    const Value& getParameter(const std::string & val)  { return m_params[val];}
private:
    std::string getEnvVar( std::string const & key )
    {
        if(std::getenv( key.c_str() )==nullptr) return "";
        else return std::string(std::getenv( key.c_str() ));
    }
    Json::Value OpenJSONFile(std::string envVar)
    {
        Json::CharReaderBuilder builder;
        Json::Value obj;   // will contain the root value after parsing.
        std::string errs;
        std::string FileName=getEnvVar(envVar.c_str());
        if(FileName=="")
        {
            std::string error="Please add "+envVar+" as variable environment ! \n";
            std::cout<<error<<std::endl;
            throw error;
        }
        std::ifstream ConfFile(FileName.c_str(),std::ifstream::binary);
        bool ok = Json::parseFromStream(builder,ConfFile,&obj,&errs);
        if ( !ok )
        {
            std::cout  << errs << "\n";
        }
        return obj;
    }
    void ExtractData(const Json::Value& params)
    {
        std::vector<std::string> id = params[m_part].getMemberNames();
        for(unsigned int i=0;i!=id.size();++i)
        {
            m_params.addParameter(id[i],params[m_part][id[i]].asString());
        }
    }
    std::string m_envName{""};
    std::string m_part{""};
    Parameters m_params;
};
#endif
