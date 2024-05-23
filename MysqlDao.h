#pragma once
#include "const.h"
#include <thread>

class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime);
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};
class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
	void checkConnection();
	std::unique_ptr<SqlConnection> getConnection();
	void returnConnection(std::unique_ptr<SqlConnection> con);
	void Close();
	~MySqlPool();
private:
	std::string _url;
	std::string _user;
	std::string _pass;
	std::string _schema;
	int _poolSize;
	std::queue<std::unique_ptr<SqlConnection>> _pool;
	std::mutex _mutex;
	std::condition_variable _cond;
	std::atomic<bool> b_stop_;
	std::thread _check_thread;
};

struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
	bool CheckEmail(const std::string& name, const std::string& email);
	bool UpdatePwd(const std::string& name, const std::string& newpwd);
	bool CheckPwd(const std::string& email, const std::string& pwd, UserInfo& userInfo);
	bool TestProcedure(const std::string& email, int& uid, string& name);
private:
	std::unique_ptr<MySqlPool> _pool;
};
