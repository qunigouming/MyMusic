#ifndef DIRECTORYSCANNERTASK_H
#define DIRECTORYSCANNERTASK_H

#include <QRunnable>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include "FileInfo.h"

class DirectoryScannerTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    DirectoryScannerTask(const QString& directoryPath, QString extension, bool recursive = true);
    ~DirectoryScannerTask() override = default;

    void run() override;

signals:
    void filesScanned(const QList<FileInfo>& files);            //文件搜寻结果
    void directoriesFound(const QStringList& directories);      //目录搜寻结果
    void taskFinished();            //任务完成信号

private:
    QString m_directoryPath;     //当前目录路径
    QString m_extension;        // 扩展名
    bool m_recursive;           //是否启用递归扫描
};

#endif // DIRECTORYSCANNERTASK_H
