#ifndef FILESCANNERWORKER_H
#define FILESCANNERWORKER_H

#include <QObject>
#include <QThreadPool>
#include <QAtomicInt>
#include <atomic>
#include <QQueue>
#include <QMutex>
#include "DirectoryScannerTask.h"

class FileScannerWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileScannerWorker(QObject* parent = nullptr);
    ~FileScannerWorker() override;

    void startScan(const QStringList& rootDirectory, const QString& extension, bool recursive = true);
    void cancelScan();

    int activeTaskCount() const;
    int pendingDirectoryCount() const;

signals:
    void progressChanged(int percent);              //进度变化
    void filesFound(const QList<FileInfo>& files);  //扫描某个目录下文件完成
    void scanFinished();                            //扫描完成

private slots:
    void handleFilesScanned(const QList<FileInfo>& files);
    void handleDirectoriesFound(const QStringList& directories);
    void handleTaskFinished();

private:
    void processNextDirectory();

    QThreadPool m_threadPool;
    std::atomic_int m_totalFiles;                //总文件数
    std::atomic_int m_directoriesProcessed;      //完成目录数
    std::atomic_int m_totalDirectories;          //目录总长
    std::atomic_bool m_cancelRequested;           //取消标记

    QString m_extension;                    // 扫描文件拓展名

    QQueue<QString> m_directoryQueue;       //缓存队列
    mutable QMutex m_queueMutex;

    bool m_recursive;
};

#endif // FILESCANNERWORKER_H
