#ifndef FILEINSPECTORINTERFACE_H
#define FILEINSPECTORINTERFACE_H

#include <QtPlugin>
#include "FileInfo.h"
#include <QObject>

class FileInspectorInterface : public QObject{
    Q_OBJECT
public:
    virtual ~FileInspectorInterface() = default;

    // 开始异步扫描
    virtual void startScan(const QStringList& directoryPaths, const QString& extension, bool recursive = true) = 0;

    // 取消扫描
    virtual void cancelScan() = 0;

    // 检查目录是否存在
    virtual bool directoryExists(const QString& path) = 0;

    // 获取活动任务数
    virtual int activeTaskCount() const = 0;

    // 获取待处理目录数
    virtual int pendingDirectoryCount() const = 0;

signals:
    // 发现新文件
    void filesFound(const QList<FileInfo>& files);

    // 扫描完成
    void scanFinished();
};

#endif // FILEINSPECTORINTERFACE_H
