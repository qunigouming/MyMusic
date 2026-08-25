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
    // 流式上传：FastDFS 库在调用线程内联调用 callback 获取文件数据（向 fd 写入 file_size 字节）
    bool uploadFileByCallback(int64_t file_size, const std::string& extension,
                              int (*callback)(void*, int64_t, int), void* arg, FastDFSFileInfo& fileinfo);
    ~FdfsClient();

    std::string getStorageUrl();

private:
    std::string _config_file_path;
    FastDFSFileInfo _fileinfo;
    std::shared_ptr<ConnectionInfo> _trackerServer;
};

#endif