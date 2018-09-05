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

#ifndef PARAMETERS_H
#define PARAMETERS_H
#include <string>
#include <map>
#include "Value.h"
#include <stdexcept>
using Parameters_iterator = std::map<Value,Value>::iterator;

class Parameters
{
public : 
    Parameters(){}
    template<typename U,typename V>
    Parameters(const typename std::map<U,V>& params)
    {
        this->clear();
        for(typename std::map<U,V>::const_iterator it=params.begin();it!=params.end();++it)
        {
            this->m_params.insert({Value(it->first),Value(it->second)});
        }
    }
    template<typename U,typename V>
    Parameters(const typename std::map<U,V>* params)
    {
        this->clear();
        for(typename std::map<U,V>::const_iterator it=params.begin();it!=params.end();++it)
        {
            this->m_params.insert({Value(it->first),Value(it->second)});
        }
    }
    Parameters(const Parameters& param)
    {
        m_params=param.m_params;
    }
    void addParameter(const std::string& key ,const std::string& value)
    {
        m_params.insert({key,value});
    }
    Parameters& operator=(const Parameters& params)
    {
        m_params=params.m_params;
        return *this;
    }
    template<typename U,typename V> Parameters& operator=(std::map<U,V>& params)
    {
        this->clear();
        for(typename std::map<U,V>::iterator it=params.begin();it!=params.end();++it)
        {
            this->m_params.insert({Value(it->first),Value(it->second)});
        }
        return *this;
    }
    void clear(){m_params.clear();}
    Parameters_iterator begin(){return m_params.begin();}
    Parameters_iterator end(){return m_params.end();}
    bool hasParam(const std::string& param)
    {
        if(m_params.find(param)!=m_params.end()) return true;
        else return false;
    }
    Value getParam(const std::string& key)
    {
        if(hasParam(key)==true)
        {
            return m_params[key];
        }
        else
        {
          throw std::out_of_range("key \""+key+"\" not found in Parameters class");   
        }
    }
    Value& operator[](const Value& key)
    {
        return m_params[key];
    }
    Value& operator[](const std::string& key)
    {
        return m_params[key];
    }
    std::size_t size(){return m_params.size();}
    Parameters_iterator find(const Value& key)
    {
        return m_params.find(key);
    }
    void printParameters(std::ostream& stream=std::cout,const std::string& mover="" )
    {
        for(Parameters_iterator it=m_params.begin();it!=m_params.end();++it)
        stream<<mover<<"["<<it->first<<"] : "<<it->second<<"\n";
    }
    void printParameters(const std::string& mover)
    {
        for(Parameters_iterator it=m_params.begin();it!=m_params.end();++it)
        std::cout<<mover<<"["<<it->first<<"] : "<<it->second<<"\n";
    }
private :
    std::map<Value,Value> m_params;
};
#endif
