#pragma once
#include <iostream>
#include "mysql/jdbc.h"

//MYSQL CONNECT TEST SUCCESS 2024.6.5
inline void TestMySQLConnect() {
	sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
	auto* con = driver->connect("127.0.0.1:3306", "root", "123456");
	if (con == nullptr) {
		std::cerr << "Connection failed." << std::endl;
		return;
	}
	con->setSchema("chat");
}