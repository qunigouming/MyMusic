#pragma once
#include <QString>
#include <taglib/fileref.h>
#include <taglib/mpegfile.h>

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
	bool isMP3File();
private:
	TagLib::FileRef _fileRef;
	QString _fileName;
};

