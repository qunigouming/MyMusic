#include "MetaTag.h"
#include <QTime>
#include <QFileInfo>
#include <QStringConverter>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mpegfile.h>
#include <taglib/flacfile.h>

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

QPixmap MetaTag::getCover()
{
	qDebug() << "getCover" << _fileName;
	if (_fileName != "C:/Users/admin/Music/杀死那个石家庄人.mp3")	return QPixmap();
	TagLib::MPEG::File mpegFile(_fileName.toStdWString().c_str());
	//TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(_fileRef.file());
	//if (!mpegFile) {
	//	qDebug() << "Not an MP3 file";
	//	return QPixmap();
	//}
	qDebug() << "MP3 file" << _fileName << mpegFile.hasID3v2Tag() << mpegFile.hasID3v1Tag();
	if (!mpegFile.isValid()) {
		qDebug() << "Invalid file";
		return QPixmap();
	}

	TagLib::ID3v2::Tag* id3v2Tag = mpegFile.ID3v2Tag();
	if (!id3v2Tag) {
		qDebug() << "No ID3v2 tag";
		return QPixmap();
	}

	//TODO: 当前问题，一读取frames的迭代器，迭代器就会失效，导致后面无法获取到数据，疑似frames没有数据
	// 查找所有 APIC 帧
	TagLib::ID3v2::FrameList frames = id3v2Tag->frameList("APIC");
	if (frames.isEmpty()) {
		qDebug() << "No APIC frame";
		return QPixmap();
	}
	for (auto it = frames.begin(); it != frames.end(); ++it) {
		if (!*it) {
			qDebug() << "Invalid frame";
            continue;
		}
		
        TagLib::ID3v2::AttachedPictureFrame* pictureFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(*it);

		if (pictureFrame) {
			TagLib::ByteVector imageData = pictureFrame->picture();
			// 将TagLib的ByteVector转换为QByteArray
			QByteArray imageByteArray(imageData.data(), imageData.size());

			// 从字节数组创建QPixmap
			QPixmap coverImage;
			if (coverImage.loadFromData(imageByteArray)) {
				qDebug() << "成功加载专辑封面，尺寸: "
					<< coverImage.width() << "x" << coverImage.height();
				return coverImage;
			}
			else {
				qDebug() << "无法从数据加载图像";
			}
		}
	}

	qDebug() << "无法获取专辑封面";
    return QPixmap();
}

bool MetaTag::isMP3File()
{
	// 尝试转换为MP3文件
	TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(_fileRef.file());
	return mpegFile != nullptr;
}
