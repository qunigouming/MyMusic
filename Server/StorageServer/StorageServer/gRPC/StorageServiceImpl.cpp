#include "StorageServiceImpl.h"
#include "../Common/FdfsClient.h"
#include "../Common/global.h"
#include "../Common/LogManager.h"
#include "../Common/ConfigManager.h"
#include <unistd.h>
#include <cctype>

// 流式上传回调：FastDFS 库在当前线程内联调用，从 gRPC 流读取数据分片写入 storage socket
static int upload_callback(void* arg, const int64_t file_size, int fd)
{
	auto* session = static_cast<UploadSession*>(arg);
	int64_t remaining = file_size;
	while (remaining > 0) {
		UploadImageRequest request;
		if (!session->reader->Read(&request)) {
			// 流提前结束：必须中止，否则 FastDFS 协议帧错乱导致连接悬挂
			session->ok = false;
			return -1;
		}
		const std::string& data = request.chunk_data();
		const char* p = data.data();
		size_t n = data.size();
		while (n > 0) {
			ssize_t w = ::write(fd, p, n);
			if (w <= 0) {
				session->ok = false;
				return -1;
			}
			p += w;
			n -= static_cast<size_t>(w);
		}
		remaining -= static_cast<int64_t>(data.size());
		session->received_size += static_cast<int64_t>(data.size());
	}
	return 0;
}

Status StorageServiceImpl::UploadImage(grpc::ServerContext* context, grpc::ServerReader<UploadImageRequest>* reader, UploadImageResponse* response)
{
	// 鉴权：服务间共享密钥（SessionServer 通过 gRPC metadata 传入）
	const auto& md = context->client_metadata();
	auto it = md.find("authorization");
	std::string expected = "Bearer " + ConfigManager::GetInstance()["Auth"]["Secret"];
	std::string actual = (it != md.end()) ? std::string(it->second.data(), it->second.size()) : "";
	if (actual != expected) {
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "missing or invalid authorization");
	}

	// 第一条消息必须是 metadata（客户端契约：先发 metadata 再发数据分片）
	UploadImageRequest request;
	if (!reader->Read(&request) || !request.has_metadata()) {
		response->set_error(ErrorCodes::RPCFailed);
		return grpc::Status::CANCELLED;
	}

	UploadSession session;
	session.filename = request.metadata().filename();
	session.mime_type = request.metadata().mime_type();
	session.expected_size = request.metadata().file_size();
	session.received_size = 0;
	session.reader = reader;
	session.ok = true;

	// 扩展名优先取 filename 的后缀，但仅接受纯字母数字（封面名是"专辑_歌手_标题"拼接，
	// 标题里可能含 "."，如 "春日影 (MyGO!!!!! ver.)" 会误提取出 ")"），否则回退 mime 类型后缀
	std::string extension;
	size_t dot = session.filename.find_last_of('.');
	if (dot != std::string::npos && dot + 1 < session.filename.size()) {
		std::string suffix = session.filename.substr(dot + 1);
		bool valid = !suffix.empty() && suffix.size() <= 8;
		for (unsigned char c : suffix) {
			if (!std::isalnum(c)) { valid = false; break; }
		}
		if (valid) extension = suffix;
	}
	if (extension.empty()) {
		extension = session.mime_type.substr(session.mime_type.find_last_of("/") + 1);
	}

	// 临时诊断日志：查看实际到达的文件名/扩展名（排查 file_ext_name 非法问题）
	LOG(INFO) << "upload request: filename=[" << session.filename << "] mime=[" << session.mime_type
		<< "] size=" << session.expected_size << " ext=[" << extension << "]";

	FastDFSFileInfo file_info;
	FdfsClient client;
	bool success = client.uploadFileByCallback(session.expected_size, extension, upload_callback, &session, file_info);
	if (!success || !session.ok) {
		LOG(ERROR) << "Failed to upload file to FastDFS";
		response->set_error(ErrorCodes::RPCFailed);
		return grpc::Status::CANCELLED;
	}

	response->set_error(ErrorCodes::Success);
	response->set_fastdfs_group(file_info.group_name);
	response->set_fastdfs_path(file_info.remote_filename);
	response->set_file_size(file_info.file_size);
	response->set_storage_url(client.getStorageUrl());
	return Status::OK;
}
