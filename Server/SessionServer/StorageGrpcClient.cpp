#include "StorageGrpcClient.h"
#include "global.h"
#include "ConfigManager.h"
#include "Common/Tools/ImgFmtInspector/ImgFmtInspector.h"

UploadImageResponse StorageGrpcClient::UploadImage(std::string file_name, std::string file_data)
{
	UploadImageResponse rsp;
	auto stub = _pool->getConnection();
	Defer defer([this, &stub, &rsp] {
		_pool->returnConnection(std::move(stub));
		return rsp;
	});
	UploadImageRequest req;
	FileMetadata metadata;
	metadata.set_filename(file_name);
	metadata.set_file_size(file_data.size());
	std::string mime_type = ImgFmtInspector::getImageMimeType(file_data);
	metadata.set_mime_type(mime_type);
	req.mutable_metadata()->CopyFrom(metadata);
	
	ClientContext context;
	std::unique_ptr<grpc::ClientWriter<UploadImageRequest>> writer(stub->UploadImage(&context, &rsp));
	if (!writer->Write(req)) {
		rsp.set_error(ErrorCodes::RPCFailed);
	}

	// 发送文件数据
	UploadImageRequest chunkReq;
    chunkReq.set_chunk_data(file_data);
	if (!writer->Write(chunkReq)) {
        rsp.set_error(ErrorCodes::RPCFailed);
	}

	writer->WritesDone();
	Status status = writer->Finish();

	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
	}
	rsp.set_error(ErrorCodes::Success);
	return rsp;
}

StorageGrpcClient::StorageGrpcClient()
{
	auto& cfg = ConfigManager::GetInstance();
	std::string host = cfg["StorageServer"]["Host"];
    std::string port = cfg["StorageServer"]["Port"];
	_pool.reset(new StorageConPool(5, host, port));
}
