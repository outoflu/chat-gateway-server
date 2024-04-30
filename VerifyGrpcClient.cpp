#include "VerifyGrpcClient.h"

GetVarifyResponse VerifyGrpcClient::GetVarifyCode(std::string email)
{
	ClientContext context;
	GetVarifyResponse response;
	GetVarifyRequest request;
	request.set_email(email);
	auto stub = _Pool->getConnection();
	Status status = stub->GetVarifyCode(&context, request, &response);
	if (status.ok()) {
		_Pool->returenConnection(std::move(stub));
		return response;
	}
	else {
		_Pool->returenConnection(std::move(stub));
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}

VerifyGrpcClient::VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::getInstance();
	std::string host = gCfgMgr["VarifyServer"]["Host"];
	std::string port = gCfgMgr["VarifyServer"]["Port"];
	_Pool.reset(new RPCConPool(5, host, port));
}


RPCConPool::RPCConPool(size_t poolsize, const std::string& host, const std::string& port)
	:_poolSize(poolsize),_host(host),_port(port),b_stop_(false)
{
	std::string url = _host + ":" + _port;
	for (size_t i = 0; i < _poolSize; i++) {
		std::shared_ptr<Channel> Channel = grpc::CreateChannel(url,
			grpc::InsecureChannelCredentials());
		_connections.push(VarifyService::NewStub(Channel));
	}
}

RPCConPool::~RPCConPool()
{
	std::lock_guard<std::mutex> lock(_mutex);
	Close();
	while (!_connections.empty()) {
		_connections.pop();
	}
}

void RPCConPool::Close()
{
	b_stop_ = true;
	_cond.notify_all();
}

std::unique_ptr<VarifyService::Stub> RPCConPool::getConnection()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_cond.wait(lock, [this] {
		if (b_stop_) {
			return true;
		}
		return !_connections.empty();
		});

	if (b_stop_) {
		return nullptr;
	}
	std::unique_ptr<VarifyService::Stub> context = std::move(_connections.front());
	_connections.pop();
	return context;
}

void RPCConPool::returenConnection(std::unique_ptr<VarifyService::Stub> context)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (b_stop_) {
		return;
	}
	_connections.push(std::move(context));
	_cond.notify_one();
	return;
}
