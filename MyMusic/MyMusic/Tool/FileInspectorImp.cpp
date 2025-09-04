#include "FileInspectorImp.h"
#include <QThread>

FileInspectorImp::FileInspectorImp(QObject* parent)
    : m_worker(new FileScannerWorker) {

    // 将工作对象移动到专用线程
    QThread* workerThread = new QThread(this);
    m_worker->moveToThread(workerThread);
    workerThread->start();

    connect(m_worker, &FileScannerWorker::filesFound, this, [this](const QList<FileInfo>& files){ emit filesFound(files); }, Qt::QueuedConnection);
    connect(m_worker, &FileScannerWorker::scanFinished, this, [this]{ emit scanFinished(); }, Qt::QueuedConnection);
}

FileInspectorImp::~FileInspectorImp() {
    m_worker->cancelScan();
    m_worker->thread()->quit();
    m_worker->thread()->wait();
    m_worker->thread()->deleteLater();
    m_worker->deleteLater();
    qDebug() << "FileInspectorImp::~FileInspectorImp()";
}

void FileInspectorImp::startScan(const QStringList& directoryPaths, const QString& extension, bool recursive) {
    m_worker->startScan(directoryPaths, extension, recursive);
}

void FileInspectorImp::cancelScan() {
    m_worker->cancelScan();
}

bool FileInspectorImp::directoryExists(const QString& path) {
    return QDir(path).exists();
}

int FileInspectorImp::activeTaskCount() const {
    return m_worker->activeTaskCount();
}

int FileInspectorImp::pendingDirectoryCount() const {
    return m_worker->pendingDirectoryCount();
}
