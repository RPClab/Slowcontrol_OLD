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

#ifndef DATABASE_H
#define DATABASE_H
#include "mariadb++/account.hpp"
#include "mariadb++/connection.hpp"
#include "Parameters.h"
class Database 
{
public:
    Database(Parameters params)
    {
        Myaccount= mariadb::account::create(params.getParam("Host").String(),params.getParam("User").String(),params.getParam("Password").String());
        m_name=params.getParam("Database").String();
        m_name=params.getParam("Table").String();
        Myconnection=mariadb::connection::create(Myaccount);
    }
    mariadb::connection_ref& operator()()
    {
        return Myconnection;
    }
    std::string getName(){return m_name;}
    std::string getTable(){return m_table;}
private:
    mariadb::account_ref Myaccount;
	mariadb::connection_ref Myconnection;
    std::string m_name;
    std::string m_table;
};
#endif
