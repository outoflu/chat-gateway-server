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

class VerifyGrpcClient:public Singleton<VerifyGrpcClient>
{
	friend class Singleton<VerifyGrpcClient>;
public:
	GetVarifyResponse GetVarifyCode(std::string email) {
		ClientContext context;
		GetVarifyResponse response;
		GetVarifyRequest request;
		request.set_email(email);
		Status status = stub_->GetVarifyCode(&context, request,&response);
		if (status.ok()) {
			return response;
		}
		else {
			response.set_error(ErrorCodes::RPCFailed);
			return response;
		}
	}
private:
	VerifyGrpcClient() {
		std::shared_ptr<Channel> channel = grpc::CreateChannel("0.0.0.0:50051", grpc::InsecureChannelCredentials());
		stub_ = VarifyService::NewStub(channel);
	}
	std::unique_ptr<VarifyService::Stub> stub_;

};

