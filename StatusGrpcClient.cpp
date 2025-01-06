#include "StatusGrpcClient.h"

StatusGrpcConPool::StatusGrpcConPool(std::size_t poolsize,const std::string& host,const std::string& port):poolSize_(poolsize),host_(host),port_(port),b_stop_(false)
{
	for (size_t i = 0; i < poolsize; i++) {
		std::shared_ptr<Channel> channel = grpc::CreateChannel(host + ":" + port, grpc::InsecureChannelCredentials());
		connections_.push(StatusService::NewStub(channel));
	}
}

StatusGrpcConPool::~StatusGrpcConPool()
{
	std::lock_guard<std::mutex> lock(mutex_);
	close();
	while (!connections_.empty()) {
		connections_.pop();
	}
}

std::unique_ptr<StatusService::Stub> StatusGrpcConPool::getConnection()
{
	std::unique_lock<std::mutex> lock(mutex_);
	cond_.wait(lock, [this]() {
		if (b_stop_) {
			return true;
		}
		return !connections_.empty();
		});
	if (b_stop_) {
		return nullptr;
	}
	auto context = std::move(connections_.front());
	connections_.pop();
	return context;
}

void StatusGrpcConPool::returnConnection(std::unique_ptr<StatusService::Stub> con)
{
	std::lock_guard<std::mutex> lock(mutex_);
	if (b_stop_) {
		return;
	}
	connections_.push(std::move(con));
	cond_.notify_one();
	return;
}

void StatusGrpcConPool::close()
{
	b_stop_ = true;
	cond_.notify_all();
}

StatusGrpcClient::~StatusGrpcClient()
{
}

GetChatServerRsp StatusGrpcClient::GetChatServer(int uid)
{
	ClientContext context;
	GetChatServerReq request;
	GetChatServerRsp response;
	request.set_uid(uid);
	auto stub = _pool->getConnection();
	Status status = stub->GetChatServer(&context,request,&response);
	Defer defer([&stub, this]() {
		_pool->returnConnection(std::move(stub));
	});
	if (status.ok()) {
		return response;
	}
	else {
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}

StatusGrpcClient::StatusGrpcClient()
{
	auto& grpcConfig = ConfigMgr::getInstance();
	std::string host = grpcConfig["StatusServer"]["host"];
	std::string port = grpcConfig["StatusServer"]["port"];
	_pool.reset(new StatusGrpcConPool(5, host, port));

}
