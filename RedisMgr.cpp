#include "RedisMgr.h"

RedisConPool::RedisConPool(size_t poolSize, const char *host, int port, const char *pwd)
	: poolSize_(poolSize), host_(host), port_(port), b_stop_(false)
{
	for (size_t i = 0; i < poolSize_; ++i)
	{
		auto *context = redisConnect(host, port);
		if (context == nullptr || context->err != 0)
		{
			if (context != nullptr)
			{
				redisFree(context);
			}
			continue;
		}

		auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd);
		if (reply->type == REDIS_REPLY_ERROR)
		{
			std::cout << "认证失败" << std::endl;
			// 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
			freeReplyObject(reply);
			continue;
		}

		// 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
		freeReplyObject(reply);
		std::cout << "认证成功" << std::endl;
		connections_.push(context);
	}
}

RedisConPool::~RedisConPool()
{

	std::lock_guard<std::mutex> lock(mutex_);
	while (!connections_.empty())
	{
		connections_.pop();
	}
}

redisContext *RedisConPool::getConnection()
{

	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this]
			   { 
			if (b_stop_) {
				return true;
			}
			return !connections_.empty(); });
	// 如果停止则直接返回空指针
	if (b_stop_)
	{
		return nullptr;
	}
	auto *context = connections_.front();
	connections_.pop();
	return context;
}

void RedisConPool::returnConnection(redisContext *context)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (b_stop_)
	{
		return;
	}
	connections_.push(context);
	cond_.notify_one();
}

void RedisConPool::Close()
{

	b_stop_ = true;
	cond_.notify_all();
}

RedisMgr::RedisMgr()
{
	auto& config = ConfigMgr::getInstance();
	auto host = config["Redis"]["Host"];
	auto port = config["Redis"]["Port"];
	auto pwd = config["Redis"]["Passwd"];
	_con_pool.reset(new RedisConPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}

RedisMgr::~RedisMgr()
{
	Close();
}



bool RedisMgr::Get(const std::string &key, std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "GET %s", key.c_str());
	if (_reply == nullptr)
	{
		std::cout << "[ GET  " << key << " ] failed" << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	if (_reply->type != REDIS_REPLY_STRING)
	{
		std::cout << "[ GET  " << key << " ] failed" << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	value = _reply->str;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);

	std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
	return true;
}

bool RedisMgr::Set(const std::string &key, const std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	// 执行redis命令行
	auto _reply = (redisReply *)redisCommand(_connect, "SET %s %s", key.c_str(), value.c_str());

	// 如果返回NULL则说明执行失败
	if (nullptr == _reply)
	{
		std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	// 如果执行失败则释放连接
	if (!(_reply->type == REDIS_REPLY_STATUS && (strcmp(_reply->str, "OK") == 0 || strcmp(_reply->str, "ok") == 0)))
	{
		std::cout << "Execut command [ SET " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	// 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	std::cout << "Execut command [ SET " << key << "  " << value << " ] success ! " << std::endl;
	return true;
}


bool RedisMgr::LPush(const std::string &key, const std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "LPUSH %s %s", key.c_str(), value.c_str());
	if (nullptr == _reply)
	{
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0)
	{
		std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	std::cout << "Execut command [ LPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::LPop(const std::string &key, std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "LPOP %s ", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL)
	{
		std::cout << "Execut command [ LPOP " << key << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::RPush(const std::string &key, const std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "RPUSH %s %s", key.c_str(), value.c_str());
	if (nullptr == _reply)
	{
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	if (_reply->type != REDIS_REPLY_INTEGER || _reply->integer <= 0)
	{
		std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}

	std::cout << "Execut command [ RPUSH " << key << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::RPop(const std::string &key, std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "RPOP %s ", key.c_str());
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL)
	{
		std::cout << "Execut command [ RPOP " << key << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}
	value = _reply->str;
	std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::HSet(const std::string &key, const std::string &hkey, const std::string &value)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER)
	{
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::HSet(const char *key, const char *hkey, const char *hvalue, size_t hvaluelen)
{
	const char* argv[4]{ 0 };
	size_t argvlen[4]{ 0 };
	argv[0] = "HSET";
	argvlen[0] = 4;
	argv[1] = key;
	argvlen[1] = strlen(key);
	argv[2] = hkey;
	argvlen[2] = strlen(hkey);
	argv[3] = hvalue;
	argvlen[3] = hvaluelen;
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommandArgv( _connect, 4, argv, argvlen);
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER)
	{
		std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}
	std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

std::string RedisMgr::HGet(const std::string &key, const std::string &hkey)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	const char* argv[3]{ 0 };
	size_t argvlen[3]{ 0 };
	argv[0] = "HGET";
	argvlen[0] = 4;
	argv[1] = key.c_str();
	argvlen[1] = key.length();
	argv[2] = hkey.c_str();
	argvlen[2] = hkey.length();
	auto _reply = (redisReply *)redisCommandArgv(_connect, 3, argv, argvlen);
	if (_reply == nullptr || _reply->type == REDIS_REPLY_NIL)
	{
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		std::cout << "Execut command [ HGet " << key << " " << hkey << "  ] failure ! " << std::endl;
		return "";
	}

	std::string value = _reply->str;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! " << std::endl;
	return value;
}

bool RedisMgr::Del(const std::string &key)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "DEL %s", key.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER)
	{
		std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
		freeReplyObject(_reply);
		_con_pool->returnConnection(_connect);
		return false;
	}
	std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
	freeReplyObject(_reply);
	_con_pool->returnConnection(_connect);
	return true;
}

bool RedisMgr::ExistsKey(const std::string &key)
{
	auto _connect = _con_pool->getConnection();
	if (_connect == nullptr) {
		return false;
	}
	auto _reply = (redisReply *)redisCommand(_connect, "exists %s", key.c_str());
	if (_reply == nullptr || _reply->type != REDIS_REPLY_INTEGER || _reply->integer == 0)
	{
		std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
		freeReplyObject(_reply);
		return false;
	}
	std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
	freeReplyObject(_reply);
	return true;
}

void RedisMgr::Close()
{
	_con_pool->Close();
}
