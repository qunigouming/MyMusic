#ifndef __NETWORKIMAGELABEL_H__
#define __NETWORKIMAGELABEL_H__

#include <QLabel>
#include <QNetworkAccessManager>


class NetworkImageLabel : public QLabel
{
	Q_OBJECT

public:
	NetworkImageLabel(QWidget *parent = nullptr);

	void setImageUrl(const QString &url);
	~NetworkImageLabel() = default;

private slots:
	void onImageDownloaded(QNetworkReply* reply);

private:
	QNetworkAccessManager* _manager;
};

#endif