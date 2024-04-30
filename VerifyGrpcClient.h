#pragma once
#include "grpcpp/grpcpp.h"
#include "message.grpc.pb.h"
#include "const.h"
#include "Singleton.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::GetVarifyRequest;
using message::GetVarifyResponse;
using message::VarifyService;

class RPCConPool {
public:
	RPCConPool(size_t poolsize, const std::string& host, const std::string& port);
	~RPCConPool();
	void Close();
	std::unique_ptr<VarifyService::Stub> getConnection();
	void returenConnection(std::unique_ptr<VarifyService::Stub> context);
private:
	std::atomic<bool> b_stop_;
	size_t _poolSize;
	std::string _host;
	std::string _port;
	std::queue<std::unique_ptr<VarifyService::Stub>> _connections;
	std::condition_variable _cond;
	std::mutex _mutex;
};

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVarifyResponse GetVarifyCode(std::string email);
private:
	VerifyGrpcClient();
	//std::unique_ptr<VarifyService::Stub> stub_;
	std::unique_ptr<RPCConPool> _Pool;

};

