#include "directoryscannertask.h"
#include <QDirIterator>
#include "MetaTag.h"
#include <QDebug>

DirectoryScannerTask::DirectoryScannerTask(const QString &directoryPath, QString extension, bool recursive)
    : m_directoryPath(directoryPath), m_recursive(recursive), m_extension(extension)
{

}

void DirectoryScannerTask::run()
{
    QList<FileInfo> files;
    QStringList directories;
    QDirIterator::IteratorFlags flags = QDirIterator::NoIteratorFlags;
    if (m_recursive) {
        flags = QDirIterator::Subdirectories;
    }

    //设置为列出文件，且不列出..和.
    QStringList nameFilters;
    nameFilters << m_extension;
    QDirIterator it(m_directoryPath,
                    nameFilters,
                    QDir::Files | QDir::NoDotAndDotDot,
                    flags);
    while (it.hasNext()) {
        it.next();
        // 检查是否为mp3文件，并读取metadata
        QString file_path = it.fileInfo().absoluteFilePath();
        MetaTag tag(file_path);
        if (tag.isMP3File() == false)   continue;   // 跳过非MP3文件
        FileInfo info(file_path, tag.getTitle(), tag.getArtist(),
            tag.getAlbum(), tag.getDuration(), tag.getFileSize(), tag.getCover(),
            it.fileInfo().birthTime(), it.fileInfo().lastModified());

        files.append(info);
    }

    // 查找子目录
    if (m_recursive) {
        QDirIterator dirIt(m_directoryPath,
                           QDir::Dirs | QDir::NoDotAndDotDot,
                           QDirIterator::NoIteratorFlags);

        while (dirIt.hasNext()) {
            dirIt.next();
            directories.append(dirIt.filePath());
        }
    }

    emit filesScanned(files);
    emit directoriesFound(directories);         //若搜索到了目录通知线程池开线程进行搜索
    emit taskFinished();
}
