#pragma once

#include <QLabel>
#include <QTimer>
#include <QNetworkAccessManager>

class RotationLabel : public QLabel
{
public:
	explicit RotationLabel(QWidget* parent = nullptr);

	void setPixmap(const QPixmap& pixmap);
	void setPixmap(const QString& url);

	void startRotation();

	void stopRotation();

	void setRotationSpeed(double speed);
	void resetRotation();

private:
	void updateDisplay();

private slots:
	void rotateImage();
	void onImageDownloaded(QNetworkReply* reply);

private:
	QPixmap _pixmap;
	QTimer* _rotation_timer = nullptr;
	double _rotation_angle = 0;
	double _rotation_speed = 1.0;
	bool _is_rotating = false;

	QNetworkAccessManager* _manager;
};

