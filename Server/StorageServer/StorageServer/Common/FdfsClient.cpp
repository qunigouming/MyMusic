#include "FdfsClient.h"

#include "ConfigManager.h"
#include "logger.h"
#include "LogManager.h"

FdfsClient::FdfsClient()
{
	auto& config = ConfigManager::GetInstance();
	_config_file_path = config["FDFS"]["CLIENT_CONF"];
	if (log_init() != 0) {
		LOG(ERROR) << "FastDfs Log initialize failed!!!";
	}

	if (fdfs_client_init(_config_file_path.c_str()) != 0) {
		log_destroy();
		LOG(ERROR) << "FastDfs Client initialize failed!!!";
	}

	_trackerServer = std::shared_ptr<ConnectionInfo>(tracker_get_connection(), [](ConnectionInfo* conn) {
		if (conn != nullptr) tracker_close_connection_ex(conn, true);
	});
	if (_trackerServer == nullptr) {
        LOG(ERROR) << "FastDfs ConnectionInfo initialize failed!!!";
	}
}

bool FdfsClient::uploadFileByCallback(int64_t file_size, const std::string& extension,
                                      int (*callback)(void*, int64_t, int), void* arg, FastDFSFileInfo& fileinfo)
{
	if (_trackerServer == nullptr) {
		return false;
	}
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1] = { 0 };
	int store_path_index;
	ConnectionInfo storageServer;
	if (tracker_query_storage_store(_trackerServer.get(), &storageServer, group_name, &store_path_index) != 0) {
        LOG(ERROR) << "FastDfs query storage server failed!!!";
        return false;
    }

	char remote_filename[128] = {0};
	// storage_upload_by_callback1 定义于 storage_client1.h（fdfs_client.h 已包含），
	// 参数顺序：callback, arg, file_size, file_ext_name, meta_list, meta_count, group_name, file_id
	int result = storage_upload_by_callback1(_trackerServer.get(), &storageServer, store_path_index,
		callback, arg, file_size, extension.c_str(), NULL, 0, group_name, remote_filename);
	if (result != 0) {
        LOG(ERROR) << "FastDfs upload file failed, error no: " << result << ", error info: " << STRERROR(result);
        return false;
	}
	LOG(INFO) << "Upload File Success, file_id is: " << remote_filename;
	fileinfo.group_name = group_name;
	fileinfo.file_size = file_size;
	fileinfo.remote_filename = remote_filename;
	fileinfo.storage_ip = storageServer.ip_addr;
	_fileinfo = fileinfo;
	return true;
}

bool FdfsClient::uploadFile(const char* buffer, int64_t buffer_size, const std::string extension, FastDFSFileInfo& fileinfo)
{
	if (_trackerServer == nullptr) {
		return false;
	}
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1] = { 0 };
	int store_path_index;
	ConnectionInfo storageServer;
	if (tracker_query_storage_store(_trackerServer.get(), &storageServer, group_name, &store_path_index) != 0) {
        LOG(ERROR) << "FastDfs query storage server failed!!!";
        return false;
    }

	char remote_filename[128] = {0};
	int result = storage_upload_by_filebuff1(_trackerServer.get(), &storageServer, store_path_index, buffer, buffer_size, extension.c_str(), NULL, 0, group_name, remote_filename);
	if (result != 0) {
        LOG(ERROR) << "FastDfs upload file failed, error no: " << result << ", error info: " << STRERROR(result);
        return false;
	}
	LOG(INFO) << "Upload File Success, file_id is: " << remote_filename;
	fileinfo.group_name = group_name;
	fileinfo.file_size = buffer_size;
	fileinfo.remote_filename = remote_filename;
	fileinfo.storage_ip = storageServer.ip_addr;
	_fileinfo = fileinfo;
	return true;
}

FdfsClient::~FdfsClient()
{
	fdfs_client_destroy();
	log_destroy();
}

std::string FdfsClient::getStorageUrl()
{
	// 注意键大小写：config.ini 中是 [FDFS] Host，property_tree 大小写敏感，写 HOST 会取到空串
	std::string url_host = ConfigManager::GetInstance()["FDFS"]["Host"];
	return std::string("http://") + url_host + "/" + _fileinfo.remote_filename;
}
