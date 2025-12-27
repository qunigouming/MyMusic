#include "NetworkImageLabel.h"

#include <QNetworkDiskCache>
#include <QDir>
#include <QNetworkReply>

NetworkImageLabel::NetworkImageLabel(QWidget *parent)
	: QLabel(parent)
{
	_manager = new QNetworkAccessManager(this);

	QNetworkDiskCache *cache = new QNetworkDiskCache(this);
	cache->setCacheDirectory(QDir::tempPath() + "/image_cache");
    _manager->setCache(cache);

	connect(_manager, &QNetworkAccessManager::finished, this, &NetworkImageLabel::onImageDownloaded);
}

void NetworkImageLabel::setImageUrl(const QString& url)
{
	QUrl qurl(url);

	if (qurl.isLocalFile() || !qurl.isValid() || qurl.scheme().isEmpty()) {
		// 本地文件（包括相对路径、绝对路径、file://协议）
		QString filePath;
		if (qurl.isLocalFile()) {
			filePath = qurl.toLocalFile();
		}
		else {
			filePath = url; // 直接使用传入的路径
		}

		QPixmap pixmap(filePath);
		if (!pixmap.isNull()) {
			// 设置本地图片
			this->setPixmap(pixmap);
		}
		else {
			this->setText("Local Image Load failed");
		}
	}
	else if (qurl.scheme().startsWith("http")) {
		// 网络图片（支持http/https）
		clear();

		QNetworkRequest request(url);
		request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
		_manager->get(request);
	}
	else {
		// 其他协议（如ftp等）
		this->setText("unsupported protocol type");
	}
}

void NetworkImageLabel::onImageDownloaded(QNetworkReply* reply) {
	if (reply->error() == QNetworkReply::NoError) {
		QByteArray data = reply->readAll();
        QPixmap image;
		if (image.loadFromData(data)) {
			setPixmap(image.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
		else {
			// 后续可能以错误图片显示处理
			setText(tr("Failed to load image"));
		}
	}
	else {
		setText(tr("Failed to load image"));
	}
	reply->deleteLater();
}
