#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <map>
#include <memory>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/asio.hpp"

#include "Singleton.h"
#include "ConfigMgr.h"

#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"

#include "jdbc/mysql_driver.h"
#include "jdbc/mysql_connection.h"
#include "jdbc/cppconn/prepared_statement.h"
#include "jdbc/cppconn/resultset.h"
#include "jdbc/cppconn/statement.h"
#include "jdbc/cppconn/exception.h"


namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using std::string;
using std::unique_ptr;

enum ErrorCodes {
	Success = 0,
	ErrorJson = 1001,
	RPCFailed = 1002,
	VarifyExpired = 1003,
	VarifyCodeErr = 1004,
	UserExist = 1005,
	PasswdError = 1006,
};

class Defer {
public:
	Defer(std::function<void()> func):_func(func) {}
	~Defer() {
		_func();
	}
private:
	std::function<void()> _func;
};