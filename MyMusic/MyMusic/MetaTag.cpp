#include "MetaTag.h"
#include <QTime>
#include <QFileInfo>
#include <QStringConverter>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/mpegfile.h>
#include <QMediaPlayer>
#include <QMediaMetadata>
#include <QEventLoop>
#include <QTimer>

extern "C" {
#include <libavformat/avformat.h>
}

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

unsigned int MetaTag::getTrack()
{
	if (!_fileRef.isNull() && _fileRef.tag()) {
        unsigned int track = _fileRef.tag()->track();
		return track;
	}
	return 0;
}
unsigned int MetaTag::getYear()
{
	if (!_fileRef.isNull() && _fileRef.tag()) {
        unsigned int year = _fileRef.tag()->year();
		return year;
	}
	return 0;
}
QString MetaTag::getDescription()
{
    if (!_fileRef.isNull() && _fileRef.tag()) {
		TagLib::String description = _fileRef.tag()->comment();
		if (description.isEmpty())	return QString();
		return QString::fromUtf8(description.toCString(true));
	}
	return QString();
}
//#define QMEDIA_GET_COVER
QPixmap MetaTag::getCover()
{
#ifdef QMEDIA_GET_COVER
	if (_fileName != "C:/Users/admin/Music/平凡之路.mp3")	return QPixmap();
	QMediaPlayer player;
	QEventLoop loop;
	QTimer timer;
	timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
	QObject::connect(&player, &QMediaPlayer::mediaStatusChanged, [&](QMediaPlayer::MediaStatus status) {
		qDebug() << "Media status changed to:" << status;
		if (status == QMediaPlayer::LoadedMedia ||
			status == QMediaPlayer::BufferedMedia ||
			status == QMediaPlayer::InvalidMedia) {
			loop.quit();
		}
	});
	QObject::connect(&player, &QMediaPlayer::metaDataChanged, [&]() {
		loop.quit();
	});
	player.setSource(QUrl::fromLocalFile(_fileName));
	player.play();
	timer.start(5000);
    loop.exec();
	QVariant coverVarient = player.metaData().value(QMediaMetaData::ThumbnailImage);
	if (coverVarient.isValid()) {
		return coverVarient.value<QPixmap>();
	}
	return QPixmap();
#elif TAGLIB_GET_COVER
	//qDebug() << "getCover" << _fileName.toUtf8().data();
	if (_fileName != "C:/Users/admin/Music/平凡之路.mp3")	return QPixmap();
#ifdef Q_OS_WIN
	TagLib::MPEG::File mpegFile(_fileName.toStdWString().c_str());
#else
	TagLib::MPEG::File mpegFile(_fileName.toUtf8().toStdString().c_str());
#endif
	if (!mpegFile.isValid()) {
		qDebug() << "open failed";
		return QPixmap();
	}
	//TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(_fileRef.file());
	//if (!mpegFile) {
	//	qDebug() << "Not an MP3 file";
	//	return QPixmap();
	//}
	qDebug() << "MP3 file" << _fileName << mpegFile.hasID3v2Tag() << mpegFile.hasID3v1Tag();

	TagLib::ID3v2::Tag* id3v2Tag = mpegFile.ID3v2Tag();
	if (!id3v2Tag) {
		qDebug() << "No ID3v2 tag";
		return QPixmap();
	}

	//TODO: 当前问题，一读取frames的迭代器，迭代器就会失效，导致后面无法获取到数据，疑似frames没有数据
	// 查找所有 APIC 帧
	const TagLib::ID3v2::FrameList& frames = id3v2Tag->frameListMap()["APIC"];
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

#else
	AVFormatContext* formatContext = nullptr;
	QPixmap coverPixmap;

	// 打开输入文件
	if (avformat_open_input(&formatContext, _fileName.toUtf8().constData(), nullptr, nullptr) != 0) {
		qWarning() << "无法打开文件:" << _fileName;
		return coverPixmap;
	}

	// 获取流信息
	if (avformat_find_stream_info(formatContext, nullptr) < 0) {
		qWarning() << "无法获取流信息";
		avformat_close_input(&formatContext);
		return coverPixmap;
	}

	// 查找附件流（包含封面）
	for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
		AVStream* stream = formatContext->streams[i];

		// 检查是否是附件流（通常包含封面）
		if (stream->disposition & AV_DISPOSITION_ATTACHED_PIC) {
			AVPacket coverPacket = stream->attached_pic;

			// 直接从内存加载图片到QPixmap
			coverPixmap.loadFromData(coverPacket.data, coverPacket.size);

			if (!coverPixmap.isNull()) {
				qDebug() << "成功加载封面图片，尺寸:" << coverPixmap.width() << "x" << coverPixmap.height();
			}
			else {
				qWarning() << "无法从数据加载封面图片";
			}

			break;
		}
	}

	avformat_close_input(&formatContext);
	return coverPixmap;
#endif
}

bool MetaTag::isMP3File()
{
	// 尝试转换为MP3文件
	TagLib::MPEG::File* mpegFile = dynamic_cast<TagLib::MPEG::File*>(_fileRef.file());
	return mpegFile != nullptr;
}
