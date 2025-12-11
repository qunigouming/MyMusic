#ifndef __FDFS_CLIENT_H__
#define __FDFS_CLIENT_H__

#include <string>
#include <memory>
#include <cstddef>
#ifdef __cplusplus
extern "C" {
#endif

#include "fastdfs/fdfs_client.h"
#include "fastcommon/logger.h"

#ifdef __cplusplus
}
#endif

struct FastDFSFileInfo {
    std::string group_name;
    std::string remote_filename;
    std::string storage_ip;
    int64_t file_size;
};

class FdfsClient {
public:
    FdfsClient();

    bool uploadFile(const char* buffer, int64_t buffer_size, const std::string extension, FastDFSFileInfo& fileinfo);
    ~FdfsClient();

    std::string getStorageUrl();

private:
    std::string _config_file_path;
    FastDFSFileInfo _fileinfo;
    std::shared_ptr<ConnectionInfo> _trackerServer;
};

#endif