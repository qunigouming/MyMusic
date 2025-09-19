#include "RotationLabel.h"
#include <QTransform>
#include <QPainter>

RotationLabel::RotationLabel(QWidget* parent) : QLabel(parent)
{
	setScaledContents(true);

	_rotation_timer = new QTimer(this);
	connect(_rotation_timer, &QTimer::timeout, this, &RotationLabel::rotateImage);
}

void RotationLabel::setPixmap(const QPixmap& pixmap)
{
	QSize size = this->size();
	QPixmap scaled_pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	_pixmap = QPixmap(size);
    _pixmap.fill(Qt::transparent);

	QPainter painter(&_pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(QBrush(scaled_pixmap));
	painter.drawRoundedRect(_pixmap.rect(), size.width(), size.width());

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
		// 保证旋转操作在图片中心进行
		QTransform transform;
		QPointF center(_pixmap.width() >> 1, _pixmap.height() >> 1);
		transform.translate(center.x(), center.y());
		transform.rotate(_rotation_angle);
		transform.translate(-center.x(), -center.y());
		QPixmap rotated_pixmap = _pixmap.transformed(transform, Qt::SmoothTransformation);

		// 创建一个与原始图片大小相同的透明图片
		QPixmap final_pixmap(_pixmap.size());
		final_pixmap.fill(Qt::transparent);
		QPainter painter(&final_pixmap);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

		// 将旋转后的图片绘制到最终的图片中心
		QPointF offset = final_pixmap.rect().center() - rotated_pixmap.rect().center();
		painter.drawPixmap(offset, rotated_pixmap);

		QLabel::setPixmap(final_pixmap);
	}
}

void RotationLabel::rotateImage() {
	_rotation_angle += _rotation_speed;
	if (_rotation_angle >= 360) _rotation_angle -= 360;
	updateDisplay();
}
