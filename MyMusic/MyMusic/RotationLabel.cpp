#include "RotationLabel.h"
#include <QTransform>

RotationLabel::RotationLabel(QWidget* parent) : QLabel(parent)
{
	setScaledContents(true);

	_rotation_timer = new QTimer(this);
	connect(_rotation_timer, &QTimer::timeout, this, &RotationLabel::rotateImage);
}

void RotationLabel::setPixmap(const QPixmap& pixmap)
{
	_pixmap = pixmap.scaled(size(), Qt::KeepAspectRatio);
	updateDisplay();
}

void RotationLabel::startRotation()
{
	if (!_is_rotating) {
        _is_rotating = true;
        _rotation_timer->start(50);
	}
}

void RotationLabel::stopRotation()
{
	if (_is_rotating) {
        _is_rotating = false;
        _rotation_timer->stop();
	}
}

void RotationLabel::setRotationSpeed(double speed)
{
	_rotation_speed = speed;
}

void RotationLabel::resetRotation()
{
	_rotation_angle = 0;
	updateDisplay();
}

void RotationLabel::updateDisplay()
{
	if (!_pixmap.isNull()) {
		QTransform transform;
		transform.rotate(_rotation_angle);
		QPixmap rotated_pixmap = _pixmap.transformed(transform);
		QLabel::setPixmap(rotated_pixmap);
	}
}

void RotationLabel::rotateImage() {
	_rotation_angle += _rotation_speed;
	if (_rotation_angle >= 360) _rotation_angle -= 360;
	updateDisplay();
}
