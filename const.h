#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <functional>
#include <map>
#include <memory>

#include "boost/beast/core.hpp"
#include "boost/beast/http.hpp"
#include "boost/asio.hpp"

#include "Singleton.h"

#include "json/json.h"
#include "json/value.h"
#include "json/reader.h"


namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
using tcp = net::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

enum ErrorCodes {
	Success = 0,
	ErrorJson = 1001,
	RPCFailed = 1002,
};