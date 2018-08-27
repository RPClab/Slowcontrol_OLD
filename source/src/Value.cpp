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


#include "Value.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <typeinfo>
#include <vector>

void Value::Init()
{
   /* if(type_names.size()==0)
    {
        type_names[typeid(std::string)] = "string";
        type_names[typeid(const char*)] = "C-string";
        type_names[typeid(int)] = "int";
        type_names[typeid(unsigned int)] = "unsigned int";
        type_names[typeid(long)] = "long";
        type_names[typeid(unsigned long)] = "unsigned long";
        type_names[typeid(long long)] = "long long";
        type_names[typeid(unsigned long long)] = "unsigned long long";
        type_names[typeid(float)] = "float";
        type_names[typeid(double)] = "double";
        type_names[typeid(long double)] = "long double";
    }*/
}

Value::Value()
{
    Init();
};

Value::Value(const std::string& value):m_value(CleanString(value))
{
    Init();
};
    
std::string Value::String()
{
    try
    {
        return m_value;
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to string type"<<std::endl;
        throw;
    }
}


std::string Value::String() const
{
    try
    {
        return m_value;
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to string type"<<std::endl;
        throw;
    }
}



const char* Value::CString()
{
    try
    {
        return m_value.c_str();
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to const char*"<<std::endl;
        throw;
    }
}
    
int Value::Int()
{
    try
    {
        return static_cast<int>(std::stof(m_value));
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to int"<<std::endl;
        throw;
    }
}
    
int Value::Int() const
{
    try
    {
        return static_cast<int>(std::stof(m_value));
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to int"<<std::endl;
        throw;
    }
}
unsigned int Value::UInt()
{
    try
    {
        return static_cast<unsigned int>(std::stof(m_value));
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to int"<<std::endl;
        throw;
    }
}
    
unsigned int Value::UInt() const
{
    try
    {
        return static_cast<unsigned int>(std::stof(m_value));
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to int"<<std::endl;
        throw;
    }
}
    
long Value::Long()
{
    try
    {
        return std::stol(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to long"<<std::endl;
        throw;
    }
}
    
unsigned long Value::ULong() 
{
    try
    {
        return std::stoul(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to unsigned long"<<std::endl;
        throw;
    }
}

long long Value::LLong() 
{
    try
    {
        return std::stoll(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to long long"<<std::endl;
        throw;
    }
}

unsigned long long Value::ULLong() 
{
    try
    {
        return std::stoull(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to unsigned long long"<<std::endl;
        throw;
    }
}

float Value::Float()
{
    try
    {
        return std::stof(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to float"<<std::endl;
        throw;
    }
};
    
double Value::Double()
{
    try
    {
        return std::stod(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to double"<<std::endl;
        throw;
    }
};


double Value::Double() const
{
    try
    {
        return std::stod(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to double"<<std::endl;
        throw;
    }
};


long double Value::LDouble()
{
    try
    {
        return std::stold(m_value);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<m_value<<" not convertible to long double"<<std::endl;
        throw;
    }
};
    

Value& Value::operator=(Value const & aValue)
{
    m_value=aValue.m_value; 
   // m_original_type=aValue.m_original_type;
    m_personal_type=aValue.m_personal_type;
    return *this;
};
    

    
Value& Value::operator=(const std::string & value)
{ 
   //m_original_type=setOriginalType(value);
    m_value=CleanString(value); 
    return *this;
};
    
Value& Value::operator=(const char*  value)
{ 
    m_value=CleanString(std::string(value)); 
    return *this;
};

Value& Value::operator=(char*  value)
{ 
    m_value=CleanString(std::string(value)); 
    return *this;
};


bool Value::IsEmpty()
{ 
    return m_value.empty();
};

std::ostream& operator<<(std::ostream& out, const Value& aValue ) 
{
	out <<aValue.m_value;
	return out;
}

bool Value::operator==(const std::string& str)
{
    if (str==m_value) return true;
    else return false;
}

bool Value::operator==(const std::string& str)const
{
    if (str==m_value) return true;
    else return false;
}


bool Value::operator!=(const std::string& str)
{
    return *this==str;
}

std::size_t Value::Size()
{
    return m_value.size();
}

std::string Value::CleanString(const std::string& str)
{
   std::string ret=str;
   //Supress not printable chars
   ret.erase(std::remove_if(ret.begin(), ret.end(),[](const char & element){return !std::isprint(element);}),ret.end());
   //Supress space before and after
   std::string::size_type lastPos = ret.find_first_not_of(" ", 0);
   ret.erase(0,lastPos);
   ret=std::string(ret.rbegin(),ret.rend());
   lastPos = ret.find_first_not_of(" ", 0);
   ret.erase(0,lastPos);
   ret=std::string(ret.rbegin(),ret.rend());
   return std::move(ret);
}

std::string Value::getOriginalType()
{
    return m_original_type;
}


std::vector<Value> Value::Tokenize(const std::string& delimiters)
{
    std::vector<Value> tokens;
    // Skip delimiters at beginning.
    std::string::size_type lastPos = m_value.find_first_not_of(delimiters, 0);
    // Find first non-delimiter.
    std::string::size_type pos = m_value.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos) 
    {
        // Found a token, add it to the vector.
        tokens.push_back(m_value.substr(lastPos, pos - lastPos));
        // Skip delimiters.
        lastPos = m_value.find_first_not_of(delimiters, pos);
        // Find next non-delimiter.
        pos = m_value.find_first_of(delimiters, lastPos);
    }
    return std::move(tokens);
}

