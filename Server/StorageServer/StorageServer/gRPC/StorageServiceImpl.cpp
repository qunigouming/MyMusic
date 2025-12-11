#include "StorageServiceImpl.h"
#include "../Common/FdfsClient.h"
#include "../Common/global.h"
#include "../Common/LogManager.h"

Status StorageServiceImpl::UploadImage(grpc::ServerContext* context, grpc::ServerReader<UploadImageRequest>* reader, UploadImageResponse* response)
{
	UploadImageRequest request;

	bool metadata_received = false;
	UploadSession session;
	int received_chunks = 0;
	while (reader->Read(&request))
    {
		if (request.has_metadata()) {
			const auto& metadata = request.metadata();
            session.filename = metadata.filename();
            session.mime_type = metadata.mime_type();
			session.expected_size = metadata.file_size();
			session.received_size = 0;
			metadata_received = true;
		}
		else if (request.chunk_data().size() > 0 && metadata_received) {
			const std::string& chunk_data  = request.chunk_data();
			received_chunks++;
			session.buffer.insert(session.buffer.end(), chunk_data.begin(), chunk_data.end());
			session.received_size += chunk_data.size();
		}
    }

	if (metadata_received) {
		if (session.expected_size != session.received_size) {
			response->set_error(ErrorCodes::RPCFailed);
			return grpc::Status::CANCELLED;
		}

		FastDFSFileInfo file_info;
		FdfsClient client;

		// 截取文件扩展名
		std::string extension = session.mime_type.substr(session.mime_type.find_last_of("/") + 1);
		bool success = client.uploadFile(session.buffer.data(), session.buffer.size(), extension, file_info);
		if (!success) {
			LOG(ERROR) << "Failed to upload file to FastDFS";
            response->set_error(ErrorCodes::RPCFailed);
            return grpc::Status::CANCELLED;
		}
		response->set_error(ErrorCodes::Success);
		response->set_fastdfs_group(file_info.group_name);
		response->set_fastdfs_path(file_info.remote_filename);
		response->set_file_size(file_info.file_size);
		response->set_storage_url(client.getStorageUrl());
	}
	return Status::OK;
}
