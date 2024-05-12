#pragma once
#include "hiredis.h"
#include <iostream>
#include "RedisMgr.h"
/**
* 这一页使用UTF8，因为Redis执行命令时要求使用UTF8来编码命令，
* 使用GBK会导致密码内容正确但不会被正确识别导致连不上，我也不知道为什么
*/
using namespace std;
namespace TestRedisConnect {
inline void TestRedis() {
	// redis默认监听端口为6379 可以再配置文件中修改
	// 连接redis
	redisContext* c = redisConnect("127.0.0.1", 6380);
	if (c->err)
	{
		printf("Connect to redisServer faile:%s\n", c->errstr);
		redisFree(c);        return;
	}
	printf("Connect to redisServer Success\n");

	std::string redis_password = "123456";
	redisReply* r = (redisReply*)redisCommand(c, "AUTH %s" ,redis_password.c_str());
	if (r->type == REDIS_REPLY_ERROR) {
		cout << "---------------" << endl;
		cout << "err:" << r->str << endl;
		cout << "---------------" << endl;
		printf("Redis连接失败\n");
	}
	else {
		printf("Redis连接成功\n");
	}

	//为redis设置key和value
	const char* command1 = "set stest1 value1";

	//ִ执行redis命令
	r = (redisReply*)redisCommand(c, command1);

	//如果返回NULL则说明执行失败
	if (NULL == r)
	{
		printf("Execut command1 failure\n");
		redisFree(c);        return;
	}

	//如果返回不为OK则说明执行失败
	if (!(r->type == REDIS_REPLY_STATUS && (strcmp(r->str, "OK") == 0 || strcmp(r->str, "ok") == 0)))
	{
		printf("Failed to execute command[%s]\n", command1);
		freeReplyObject(r);
		redisFree(c);        return;
	}

	//ִ执行成功后释放资源
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command1);

	const char* command2 = "strlen stest1";
	r = (redisReply*)redisCommand(c, command2);

	//如果返回类型不为integer则说明执行失败
	if (r->type != REDIS_REPLY_INTEGER)
	{
		printf("Failed to execute command[%s]\n", command2);
		freeReplyObject(r);
		redisFree(c);        return;
	}

	//获取字符串长度
	int length = r->integer;
	freeReplyObject(r);
	printf("The length of 'stest1' is %d.\n", length);
	printf("Succeed to execute command[%s]\n", command2);

	//获取redis键值对信息
	const char* command3 = "get stest1";
	r = (redisReply*)redisCommand(c, command3);
	if (r->type != REDIS_REPLY_STRING)
	{
		printf("Failed to execute command[%s]\n", command3);
		freeReplyObject(r);
		redisFree(c);        return;
	}
	printf("The value of 'stest1' is %s\n", r->str);
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command3);

	const char* command4 = "get stest2";
	r = (redisReply*)redisCommand(c, command4);
	if (r->type != REDIS_REPLY_NIL)
	{
		printf("Failed to execute command[%s]\n", command4);
		freeReplyObject(r);
		redisFree(c);        return;
	}
	freeReplyObject(r);
	printf("Succeed to execute command[%s]\n", command4);

	//释放连接资源
	redisFree(c);

}

inline void TestRedisMgr() {
	assert(RedisMgr::GetInstance()->Set("blogwebsite","xxx.club"));
	std::string value="";
	assert(RedisMgr::GetInstance()->Get("blogwebsite", value) );
	assert(RedisMgr::GetInstance()->Get("nonekey", value) == false);
	assert(RedisMgr::GetInstance()->HSet("bloginfo","blogwebsite", "xxx.club"));
	assert(RedisMgr::GetInstance()->HGet("bloginfo","blogwebsite") != "");
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->Del("bloginfo"));
	assert(RedisMgr::GetInstance()->ExistsKey("bloginfo") == false);
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue1"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue2"));
	assert(RedisMgr::GetInstance()->LPush("lpushkey1", "lpushvalue3"));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->RPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey1", value));
	assert(RedisMgr::GetInstance()->LPop("lpushkey2", value)==false);
	//RedisMgr::GetInstance()->Close();
}
}