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

#define MAX_FILE_LEN 1024 * 32          // 文件分片大小，最大不能超过16位(服务器限制，除非消息长度类型改为int)

UploadWidget::UploadWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::UploadWidgetClass())
{
	ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);     // 设置窗口销毁时自动删除
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
		// 先发送标签数据给服务器，再在回调中发送文件数据
		QPixmap cover = ui->select_cover_lab->pixmap();
		if (cover.isNull())	return;
		QByteArray coverData;
		QBuffer coverbuffer(&coverData);
        coverbuffer.open(QIODevice::WriteOnly);
		if (!cover.save(&coverbuffer, "PNG")) {
            qWarning() << "cover save error" << coverbuffer.errorString();
            return;
		}
		QJsonObject metaJson;
        metaJson["title"] = ui->title_LineE->text();
        metaJson["album"] = ui->album_name_LineE->text();
        metaJson["artists"] = ui->artist_LineE->text();
        
        QTime time = QTime::fromString(ui->duration_LineE->text(), "mm:ss");
        int totalSeconds = 0;
        if (time.isValid()) {
            int minutes = time.minute();
            totalSeconds = minutes * 60 + time.second();
        }
        metaJson["duration"] = totalSeconds;
        metaJson["track"] = ui->album_track_LineE->text().toInt();
        metaJson["description"] = ui->album_desc->toPlainText();
        metaJson["icon"] = QString(coverData.toBase64());
        metaJson["release_date"] = ui->birthday_LineE->text();
		QJsonDocument metaDoc(metaJson);
        emit TcpManager::GetInstance()->sig_send_data(ID_UPLOAD_META_TYPE_REQ, metaDoc.toJson());
	});

	connect(TcpManager::GetInstance().get(), &TcpManager::sig_upload_file, this, [this] {
        //读取文件数据
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
        QFileInfo fileInfo(_file_path);
        QString fileName = fileInfo.fileName();
        qDebug() << "fileName:" << fileName;

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
        int seq = 0;
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
    qDebug() << "~UploadWidget()";
	delete ui;
}

