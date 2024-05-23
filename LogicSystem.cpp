#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VerifyGrpcClient.h"
#include "RedisMgr.h"

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
		auto rsp=VerifyGrpcClient::GetInstance()->GetVarifyCode(email);
		//std::cout << "email is " << email << std::endl;
		root["error"] = rsp.error();
		root["email"] = src_root["email"];
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		});

	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
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
			return;
		}
		std::string varify_code;
		const std::string key = std::string("code_") + src_root["email"].asString();
		bool b_get_varify = RedisMgr::GetInstance()->Get(key, varify_code);
		if (!b_get_varify) {
			std::cout << "key is:" << key << std::endl;
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return ;
		}
		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return ;
		}

		//访问redis查找
		bool b_usr_exist = RedisMgr::GetInstance()->ExistsKey(src_root["user"].asString());
		if (b_usr_exist) {
			std::cout << " user exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return ;
		}

		//查找数据库判断用户是否存在

		root["error"] = 0;
		root["email"] = src_root["email"];
		root["user"] = src_root["user"].asString();
		root["passwd"] = src_root["passwd"].asString();
		root["confirm"] = src_root["confirm"].asString();
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return ;
	});
}