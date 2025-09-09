#ifndef FILEINFO_H
#define FILEINFO_H

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QDebug>
#include <QPixmap>

struct FileInfo {
    QString filePath;
    QString title;
    QString artist;
    QString album;
    QString duration;
    QString size;
    QPixmap cover;
    QDateTime created;
    QDateTime modified;

    FileInfo() = default;
    FileInfo(const QString& path, QString title, QString artist,
        QString album, QString duration, QString size,
        QPixmap cover, QDateTime created, QDateTime modified)
        : filePath(path), title(title), artist(artist),
        album(album), duration(duration), size(size),
        cover(cover), created(created), modified(modified) {}
};

inline QDebug operator<< (QDebug debug, const FileInfo &info) {
    QDebugStateSaver saver(debug); // 自动保存/恢复调试流状态
    debug.nospace() << "FileInfo("
                    << "filePath=" << info.filePath << ", "
                    << "title=" << info.title << ", "
                    << "artist=" << info.artist << ", "
                    << "album=" << info.album << ", "
                    << "duration=" << info.duration << ", "
                    << "size=" << info.size << ", "
                    << "created=" << info.created.toString(Qt::ISODate) << ", "
                    << "modified=" << info.modified.toString(Qt::ISODate) << ")";
    return debug;
}

//Q_DECLARE_METATYPE(FileInfo)
//Q_DECLARE_METATYPE(QList<FileInfo>)

#endif // FILEINFO_H
