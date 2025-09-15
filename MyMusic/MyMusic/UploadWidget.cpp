#include "UploadWidget.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include "MetaTag.h"
#include <QFile>
#include <QCryptographicHash>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonDocument>
#include "tcpmanager.h"
#include <QBuffer>

#define MAX_FILE_LEN 1024 * 1024 * 8

UploadWidget::UploadWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::UploadWidgetClass())
{
	ui->setupUi(this);
	ui->select_file_lab->setSuffixs({"mp3"});
	ui->select_cover_lab->setSuffixs({"jpg", "png", "jpeg"});

	connect(ui->select_file_lab, &DragDropLabel::fileDropped, [this](const QString& filePath) {
		ui->tabWidget->setCurrentIndex(1);
		_file_path = filePath;
		ui->select_file_lab->setText(_file_path);

		// 读取文件信息
		MetaTag metaTag(_file_path);
		QPixmap cover = metaTag.getCover();
		if (!cover.isNull()) {
            ui->select_cover_lab->setPixmap(cover.scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
		}
        ui->title_LineE->setText(metaTag.getTitle());
        ui->artist_LineE->setText(metaTag.getArtist());
        ui->album_name_LineE->setText(metaTag.getAlbum());
		ui->duration_LineE->setText(metaTag.getDuration());
		ui->birthday_LineE->setText(QString::number(metaTag.getYear()));
		ui->album_track_LineE->setText(QString::number(metaTag.getTrack()));
	});

	connect(ui->select_cover_lab, &DragDropLabel::fileDropped, [this](const QString& filePath) {
		ui->select_cover_lab->setPixmap(QPixmap(filePath).scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	});

	connect(ui->cancel_btn, &QPushButton::clicked, [this]() {
		deleteLater();
	});

	connect(ui->sure_btn, &QPushButton::clicked, [this]() {
		// 上传至服务器
		QFile file(_file_path);
		if (!file.open(QIODevice::ReadOnly)) {
			qWarning() << "file open error" << file.errorString();
            return;
		}

		qint64 originalPos = file.pos();
		QCryptographicHash hash(QCryptographicHash::Md5);
		if (!hash.addData(&file)) {
			qWarning() << "hash error" << file.errorString();
			return;
		}
		QString file_md5 = hash.result().toHex();
		QByteArray buffer;
		int seq = 0;
		QFileInfo fileInfo(_file_path);
		QString fileName = fileInfo.fileName();
		qDebug() << "fileName:" << fileName;

		// 先发送标签数据，再发送文件数据
		QJsonObject metaJson;
		metaJson["title"] = ui->title_LineE->text();
		metaJson["album"] = ui->album_name_LineE->text();
        metaJson["artists"] = ui->artist_LineE->text();
		metaJson["duration"] = ui->duration_LineE->text();
		metaJson["track"] = ui->album_track_LineE->text();
		metaJson["description"] = ui->album_desc->toPlainText();
		metaJson["release_date"] = ui->birthday_LineE->text();
		// 将图片存储到ByteArray中
		QByteArray coverData;
		QBuffer coverbuffer(&coverData);
		coverbuffer.open(QIODevice::WriteOnly);
		ui->select_cover_lab->pixmap().toImage().save(&coverbuffer, "PNG");
		metaJson["icon"] = QString(coverData.toBase64());
		QJsonDocument metaDoc(metaJson);
		emit TcpManager::GetInstance()->sig_send_data(ID_UPLOAD_META_TYPE_REQ, metaDoc.toJson());

		// 发送文件给服务器
		int total_size = fileInfo.size();
		int last_seq = 0;
		if (total_size > MAX_FILE_LEN) {
			last_seq = (total_size / MAX_FILE_LEN) + 1;
		}
		else {
			last_seq = total_size / MAX_FILE_LEN;
		}
		file.seek(originalPos);
		while (!file.atEnd()) {
			buffer = file.read(MAX_FILE_LEN);
			++seq;
			QJsonObject jsonObj;
            jsonObj["md5"] = file_md5;
            jsonObj["name"] = fileName;
			jsonObj["seq"] = seq;
			jsonObj["trans_size"] = buffer.size() + (seq - 1) * MAX_FILE_LEN;
            jsonObj["total_size"] = total_size;
			if (buffer.size() < MAX_FILE_LEN) {
				jsonObj["last"] = 1;
			}
			else {
				jsonObj["last"] = 0;
			}
			jsonObj["data"] = QString(buffer.toBase64());
			jsonObj["last_seq"] = last_seq;
			QJsonDocument doc(jsonObj);
			QByteArray data = doc.toJson();
			emit TcpManager::GetInstance()->sig_send_data(ReqID::ID_UPLOAD_FILE_REQ, data);
		}
		file.close();
	});
}

UploadWidget::~UploadWidget()
{
	delete ui;
}

