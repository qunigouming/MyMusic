#pragma once
#include <QLabel>
#include <QTimer>
class RotationLabel : public QLabel
{
public:
	explicit RotationLabel(QWidget* parent = nullptr);

	void setPixmap(const QPixmap& pixmap);

	void startRotation();

	void stopRotation();

	void setRotationSpeed(double speed);
	void resetRotation();

protected:
	void updateDisplay();

private slots:
	void rotateImage();

private:
	QPixmap _pixmap;
	QTimer* _rotation_timer = nullptr;
	double _rotation_angle = 0;
	double _rotation_speed = 2.0;
	bool _is_rotating = false;
};

