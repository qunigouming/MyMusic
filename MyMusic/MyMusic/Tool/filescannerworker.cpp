#include "filescannerworker.h"
#include <QCoreApplication>
#include <cmath>

FileScannerWorker::FileScannerWorker(QObject* parent)
    : QObject(parent), m_totalFiles(0),
    m_directoriesProcessed(0), m_totalDirectories(0),
    m_cancelRequested(false), m_recursive(true) {

    // 配置线程池
    int idealThreads = QThread::idealThreadCount();
    m_threadPool.setMaxThreadCount(qMax(4, idealThreads * 2));
}

FileScannerWorker::~FileScannerWorker() {
    cancelScan();
    m_threadPool.waitForDone();
}

void FileScannerWorker::startScan(const QStringList& rootDirectory, const QString& extension, bool recursive) {
    m_cancelRequested = false;
    m_totalFiles = 0;
    m_directoriesProcessed = 0;
    m_totalDirectories = rootDirectory.size(); // 初始目录

    m_extension = extension;
    m_recursive = recursive;

    {
        QMutexLocker locker(&m_queueMutex);
        m_directoryQueue.clear();
        for (const auto& dir : rootDirectory) {
            m_directoryQueue.enqueue(dir);
        }
    }
    // 启动初始任务
    int init_task_count = qMin(m_directoryQueue.size(), m_threadPool.maxThreadCount());
    for (int i = 0; i < init_task_count; ++i)
        processNextDirectory();
}

void FileScannerWorker::cancelScan() {
    m_cancelRequested = true;
    m_threadPool.clear();

    QMutexLocker locker(&m_queueMutex);
    m_directoryQueue.clear();
}

int FileScannerWorker::activeTaskCount() const {
    return m_threadPool.activeThreadCount();
}

int FileScannerWorker::pendingDirectoryCount() const {
    QMutexLocker locker(&m_queueMutex);
    return m_directoryQueue.size();
}

void FileScannerWorker::processNextDirectory() {
    if (m_cancelRequested) {
        return;
    }

    QString nextDir;
    {
        QMutexLocker locker(&m_queueMutex);
        if (m_directoryQueue.isEmpty()) {
            // 检查是否所有任务完成
            if (m_threadPool.activeThreadCount() == 0) {
                emit scanFinished();
            }
            return;
        }
        nextDir = m_directoryQueue.dequeue();
    }
    DirectoryScannerTask* task = new DirectoryScannerTask(nextDir, m_extension, m_recursive);
    connect(task, &DirectoryScannerTask::filesScanned,
            this, &FileScannerWorker::handleFilesScanned, Qt::QueuedConnection);
    connect(task, &DirectoryScannerTask::directoriesFound,
            this, &FileScannerWorker::handleDirectoriesFound, Qt::QueuedConnection);
    connect(task, &DirectoryScannerTask::taskFinished,
            this, &FileScannerWorker::handleTaskFinished, Qt::QueuedConnection);

    m_threadPool.start(task);
}

void FileScannerWorker::handleFilesScanned(const QList<FileInfo>& files) {
    if (m_cancelRequested.load()) return;

    m_totalFiles += files.size();

    emit filesFound(files);
}

void FileScannerWorker::handleDirectoriesFound(const QStringList& directories) {
    if (m_cancelRequested.load()) return;

    int newDirs = directories.size();
    m_totalDirectories += newDirs;

    {
        QMutexLocker locker(&m_queueMutex);
        for (const QString& dir : directories) {
            m_directoryQueue.enqueue(dir);
        }
    }

    // 启动新任务处理发现的目录
    for (int i = 0; i < qMin(newDirs, m_threadPool.maxThreadCount()); i++) {
        processNextDirectory();
    }
}

void FileScannerWorker::handleTaskFinished() {
    m_directoriesProcessed++;
    processNextDirectory(); // 处理下一个目录
}
