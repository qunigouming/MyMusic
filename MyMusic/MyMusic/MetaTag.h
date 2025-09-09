#pragma once
#include <QString>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <QPixmap>

class MetaTag
{
public:
	MetaTag(QString file_name);
	~MetaTag() = default;
	QString getTitle();
	QString getArtist();
	QString getAlbum();
    QString getDuration();
    QString getFileSize();
	QPixmap getCover();			// 获取封面图片，仅限本地文件
	bool isMP3File();
private:
	TagLib::FileRef _fileRef;
	QString _fileName;
};

