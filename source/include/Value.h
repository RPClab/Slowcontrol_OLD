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


#ifndef Value_h
#define Value_h
#include <string>
#include <iostream>
#include <typeinfo>
#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>

class Value
{
public:
    Value();
    Value(const std::string& value);
    template<class T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr> Value(const T& t)
{
    try
    {
        Init();
        //m_original_type=setOriginalType(t);
        m_value=std::to_string(t);
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<t<<" not convertible to Value type"<<std::endl;
        throw;
    }
};
template<class T, typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
Value& operator=(const T & value)
{
    try
    {
       // m_original_type=setOriginalType(value);
        m_value=std::to_string(value);
        return *this;
    }
    catch(std::exception& e)
    {
        std::cout<<"Value : "<<value<<" not convertible to Value type"<<std::endl;
        throw;
    }
};
    std::string String();
    std::string String() const;
    const char* CString();
    int Int();
    int Int() const;
    bool operator<( const Value &val ) const
    {
        return m_value<val.m_value;
    }
    bool operator>(const Value val) const
    {
        return m_value>val.m_value;
    }
    unsigned int UInt();
    unsigned int UInt() const;
    long Long();
    unsigned long ULong();
    unsigned short UShort()
    {
        return static_cast<unsigned short>(this->Int());
    }
    short Short()
    {
        return static_cast<short>(this->Int());
    }
    long long LLong();
    unsigned long long ULLong();
    float Float();
    double Double();
    double Double() const;
    long double LDouble();
    std::size_t Size();
    Value& operator=(Value const & aValue);
    bool operator==(const std::string& str);
    bool operator==(const Value& str)
    {
        if(this->m_value==str.m_value) return true;
        else return false;
    }
    bool operator==(const Value& str)const
    {
        if(this->m_value==str.m_value) return true;
        else return false;
    }
    bool operator==(const std::string& str) const;
    bool operator!=(const std::string& str);
    Value& operator=(const std::string & value);
    bool IsEmpty();
    std::string getOriginalType();
    void setPersonalType(const std::string& perso)
    {
        m_personal_type=perso;
    }
    std::string getPersonalType()
    {
        return  m_personal_type;
    }
    Value& operator=(const char*  value);
    Value& operator=(char*  value);
    std::vector<Value> Tokenize(const std::string& delimiters);
    std::vector<Value> Tokenize(const std::string& delimiters) const
    {
        return Tokenize(delimiters);
    }
private:
    void Init();
    std::string m_value{""};
    std::string CleanString(const std::string& stri);
    friend std::ostream& operator<<(std::ostream& out, const Value& a );
    std::string m_personal_type{""};
    std::string m_original_type{""};
  /*  struct Hasher 
    {
        std::size_t operator()(std::reference_wrapper<const std::type_info> code) const
        {
            return code.get().hash_code();
        }
    };
 
    struct EqualTo 
    {
        bool operator()(std::reference_wrapper<const std::type_info> lhs, std::reference_wrapper<const std::type_info> rhs) const
        {
            return lhs.get() == rhs.get();
        }
    };
   template<class T> std::string setOriginalType(const T & t)
   {
       if(type_names.find(typeid(t))!=type_names.end()) return type_names[typeid(t)];
        else
        {
            type_names.insert(std::pair<std::reference_wrapper<const std::type_info>, std::string>(typeid(t),typeid(t).name()));
            return type_names[typeid(t)];
        }
   }
   std::unordered_map<std::reference_wrapper<const std::type_info>, std::string, Hasher, EqualTo> type_names;*/
};
#endif
