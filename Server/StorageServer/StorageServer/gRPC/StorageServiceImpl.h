#ifndef STORAGE_SERVICE_IMPL_H_
#define STORAGE_SERVICE_IMPL_H_
#include <queue>
#include <mutex>
#include <condition_variable>

#include <grpcpp/grpcpp.h>
#include "../Common/Singleton.h"

#include "message.grpc.pb.h"
#include "message.pb.h"

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

using message::UploadImageRequest;
using message::UploadImageResponse;
using message::FileMetadata;
using message::StorageService;

struct UploadSession {
	std::string filename;
	std::string mime_type;
	int64_t expected_size;
	int64_t received_size;
	std::vector<char> buffer;
};

class StorageServiceImpl final : public StorageService::Service
{
public:
	StorageServiceImpl() = default;
	Status UploadImage(grpc::ServerContext* context, grpc::ServerReader<UploadImageRequest>* reader, UploadImageResponse* response) override;
};

#endif
