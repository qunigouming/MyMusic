#include "MetaTag.h"
#include <QTime>
#include <QFileInfo>
#include <QStringConverter>

MetaTag::MetaTag(QString file_name) : _fileName(file_name), _fileRef(file_name.toStdWString().c_str())
{
}

QString MetaTag::getTitle()
{
	if (!_fileRef.isNull() && _fileRef.tag()) {
		TagLib::String title = _fileRef.tag()->title();
		if (title.isEmpty()) {
			QFileInfo fileinfo(_fileName);
			return fileinfo.fileName().chopped(4);
		}
		return QString::fromUtf8(title.toCString(true));
	}
	return QString();
}

QString MetaTag::getArtist()
{
	if (!_fileRef.isNull() && _fileRef.tag()) {
		TagLib::String artist = _fileRef.tag()->artist();
		return QString::fromUtf8(artist.toCString(true));
	}
	return QString();
}

QString MetaTag::getAlbum()
{
	if (!_fileRef.isNull() && _fileRef.tag()) {
		TagLib::String album = _fileRef.tag()->album();
		if (!album.isEmpty())	return QString::fromUtf8(album.toCString(true));
	}
	return QString("未知专辑");
}

QString MetaTag::getDuration()
{
	if (!_fileRef.isNull() && _fileRef.audioProperties()) {
		QTime time(0, 0, 0);
        time = time.addSecs(_fileRef.audioProperties()->lengthInSeconds());
		return time.toString("mm:ss");
	}
	return QString();
}

QString MetaTag::getFileSize()
{
	if (!_fileRef.isNull()) {
		long fileSize = _fileRef.file()->length();
		const long KB = 1024;
		const long MB = KB * KB;
		const long GB = MB * KB;
		QString unit;
		double formattedSize;

		if (fileSize >= GB) {
			formattedSize = static_cast<double>(fileSize) / GB;
			unit = "G";
		}
		else if (fileSize >= MB) {
			formattedSize = static_cast<double>(fileSize) / MB;
			unit = "M";
		}
		else if (fileSize >= KB) {
			formattedSize = static_cast<double>(fileSize) / KB;
			unit = "K";
		}
		else {
			formattedSize = static_cast<double>(fileSize);
			unit = "B";
		}

		// 格式化输出，保留两位小数，但如果是整数则不显示小数部分
		QString sizeStr;
		if (formattedSize == static_cast<int>(formattedSize)) {
			sizeStr = QString::number(static_cast<int>(formattedSize));
		}
		else {
			sizeStr = QString::number(formattedSize, 'f', 1);
		}

		return QString("%1%2").arg(sizeStr).arg(unit);
	}
	return QString();
}

bool MetaTag::isMP3File()
{
	// 尝试转换为MP3文件
	TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(_fileRef.file());
	return mpegFile != nullptr;
}
