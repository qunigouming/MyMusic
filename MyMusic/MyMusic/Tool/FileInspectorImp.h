#ifndef FILEINSPECTORPLUGIN_H
#define FILEINSPECTORPLUGIN_H

#include "FileInspectorInterface.h"
#include "FileScannerWorker.h"
#include <QObject>

class FileInspectorImp : public FileInspectorInterface
{
    Q_OBJECT
public:
    explicit FileInspectorImp(QObject *parent = nullptr);
    ~FileInspectorImp() override;
    // 接口实现
    void startScan(const QStringList& directoryPaths, const QString& extension, bool recursive = true) override;       //开始扫描
    void cancelScan() override;                                                         //取消扫描
    bool directoryExists(const QString& path) override;                                 //目录存在
    int activeTaskCount() const override;                                               //获取当前任务数
    int pendingDirectoryCount() const override;

private:
    FileScannerWorker* m_worker;
};

#endif // FILEINSPECTORPLUGIN_H
