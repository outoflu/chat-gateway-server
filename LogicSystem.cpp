#include "LogicSystem.h"
#include "HttpConnection.h"
LogicSystem::~LogicSystem()
{
}

bool LogicSystem::HandleGet(std::string path,std::shared_ptr<HttpConnection> con)
{
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}
	else {
		_get_handlers[path](con);
		return true;
	}
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_post_handlers.find(path) == _post_handlers.end()) {
		std::cout << path << std::endl;
		return false;
	}
	else {
		_post_handlers[path](con);
		return true;
	}
}

void LogicSystem::RegGet(std::string url,HttpHandler handler)
{
	_get_handlers.insert(make_pair(url, handler));
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

LogicSystem::LogicSystem() {
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req"<<std::endl;
		int i = 0;
		for (const auto & elem : connection->_get_params) {
			i++;
			beast::ostream(connection->_response.body()) << "param " << i << " key is " << elem.first;
			beast::ostream(connection->_response.body())<< " value is" << elem.second << std::endl;
			//std::cout << "write Error" << std::endl;
		}
	});
	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		//std::cout << "receive body is" << body_str << std::endl;
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value  src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse Json Data" << std::endl;
			root["error"] = ErrorCodes::ErrorJson;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return ;
		}
		if (!src_root.isMember("email")) {
			std::cout << "Failed to parse Json Data" << std::endl;
			root["error"] = ErrorCodes::ErrorJson;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return ;
		}
		auto email = src_root["email"].asString();
		//std::cout << "email is " << email << std::endl;
		root["error"] = 0;
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		});
}