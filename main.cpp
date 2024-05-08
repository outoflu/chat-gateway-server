#include <iostream>
#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"
#include "CServer.h"
#include "ConfigMgr.h"
#include "TestRedisConnect.h"
int main() {
	
	/*
	system("chcp 65001");
	TestRedisConnect::TestRedis();
	*/
	/*
	system("chcp 65001");
	TestRedisConnect::TestRedisMgr();
	*/
	auto& gcConfigMgr=ConfigMgr::getInstance();
	std::string gate_port_str = gcConfigMgr["GateServer"]["Port"];
	unsigned short gate_port = atoi(gate_port_str.c_str());
	std::cout << gate_port << std::endl;
	try {
		//unsigned short port = static_cast<unsigned short>(8080);
		net::io_context ioc{ 1 };
		boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
		signals.async_wait([&ioc](const boost::system::error_code ec,int  signal_number) {
			if (ec) {
				return;
			}
			ioc.stop();
			});
		std::make_shared<CServer>(ioc,gate_port)->Start();
		std::cout << "Gate Server listen on port" << port << std::endl;
		ioc.run();
	}
	catch (std::exception const& e) {
		std::cerr << "Error:" << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}