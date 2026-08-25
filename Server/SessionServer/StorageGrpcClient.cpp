#include "StorageGrpcClient.h"
#include "global.h"
#include "ConfigManager.h"
#include "Common/Tools/ImgFmtInspector/ImgFmtInspector.h"
#include <chrono>

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
	// 服务间鉴权：共享密钥走 gRPC metadata
	context.AddMetadata("authorization", "Bearer " + _auth_secret);
	// 流式上传：deadline 作用于整个流，必须在 stub 调用前设置
	context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(60));
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

UploadImageResponse StorageGrpcClient::UploadImage(std::string file_name, std::string file_data, std::string mime_type)
{
	UploadImageResponse rsp;
	rsp.set_error(ErrorCodes::Success);
	auto stub = _pool->getConnection();
	Defer defer([this, &stub, &rsp] {
		_pool->returnConnection(std::move(stub));
		return rsp;
	});
	UploadImageRequest req;
	FileMetadata metadata;
	metadata.set_filename(file_name);
	metadata.set_file_size(file_data.size());
	metadata.set_mime_type(mime_type);
	req.mutable_metadata()->CopyFrom(metadata);

	ClientContext context;
	// 服务间鉴权：共享密钥走 gRPC metadata
	context.AddMetadata("authorization", "Bearer " + _auth_secret);
	// 流式上传：deadline 作用于整个流，必须在 stub 调用前设置
	context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(60));
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

	return rsp;
}

UploadImageResponse StorageGrpcClient::UploadAudio(const std::string& file_name, int64_t file_size, const ReadCallback& read_cb)
{
	UploadImageResponse rsp;
	auto stub = _pool->getConnection();
	Defer defer([this, &stub, &rsp] {
		_pool->returnConnection(std::move(stub));
		return rsp;
	});

	ClientContext context;
	context.AddMetadata("authorization", "Bearer " + _auth_secret);
	// 大文件流式上传，deadline 放宽到 300 秒（作用于整个流）
	context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(300));
	std::unique_ptr<grpc::ClientWriter<UploadImageRequest>> writer(stub->UploadImage(&context, &rsp));

	// 先发 metadata（服务端契约：首条消息必须是 metadata）
	UploadImageRequest metaReq;
	FileMetadata metadata;
	metadata.set_filename(file_name);
	metadata.set_file_size(file_size);
	metadata.set_mime_type("audio/mpeg");
	*metaReq.mutable_metadata() = metadata;
	if (!writer->Write(metaReq)) {
		rsp.set_error(ErrorCodes::RPCFailed);
		return rsp;
	}

	// 256KB 分块写入流
	const size_t kChunkSize = 256 * 1024;
	std::string chunk(kChunkSize, '\0');
	while (true) {
		size_t got = read_cb(chunk.data(), kChunkSize);
		if (got == 0) break;
		UploadImageRequest chunkReq;
		chunkReq.set_chunk_data(chunk.data(), got);
		if (!writer->Write(chunkReq)) {
			rsp.set_error(ErrorCodes::RPCFailed);
			return rsp;
		}
	}

	writer->WritesDone();
	Status status = writer->Finish();
	if (!status.ok()) {
		rsp.set_error(ErrorCodes::RPCFailed);
	}

	return rsp;
}

StorageGrpcClient::StorageGrpcClient()
{
	auto& cfg = ConfigManager::GetInstance();
	std::string host = cfg["StorageServer"]["Host"];
    std::string port = cfg["StorageServer"]["Port"];
	_pool.reset(new StorageConPool(5, host, port));
	_auth_secret = cfg["Auth"]["Secret"];
}
