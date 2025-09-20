#include "VolumeWidget.h"
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QPainterPath>

VolumeWidget::VolumeWidget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::VolumeWidgetClass())
{
	ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	//setContentsMargins(5, 5, 5, 5);
	//QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
	//effect->setColor(QColor(0, 0, 0, 50));
	//effect->setOffset(0, 0);
	//effect->setBlurRadius(5);
	//setGraphicsEffect(effect);

	setFixedWidth(30);

	setStyleSheet(R"(
		VolumeWidget {
			background-color: rgba(255, 255, 255, 0.5);
			border-radius: 10px;
			border: 1px solid #cccccc;
		}

		QLabel {
			font-size: 9px;
		}
	)");

	connect(ui->volume_slider, &QSlider::valueChanged, this, &VolumeWidget::setVolume);
}

VolumeWidget::~VolumeWidget()
{
	delete ui;
}

void VolumeWidget::setVolume(int volume)
{
	ui->volume_lab->setText(QString::number(volume) + "%");
	ui->volume_slider->setValue(volume);
	emit volumeChanged(volume);
}

void VolumeWidget::mute()
{
	_lastVolume = ui->volume_slider->value();
	ui->volume_slider->setValue(0);
}

int VolumeWidget::unMute()
{
	ui->volume_slider->setValue(_lastVolume);
	return _lastVolume;
}

void VolumeWidget::leaveEvent(QEvent* event)
{
	hide();
    QWidget::leaveEvent(event);
}

void VolumeWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	QPainterPath path;
	path.addRoundedRect(rect(), 10, 10);
	painter.fillPath(path, QBrush(QColor(255, 255, 255, 100)));
    painter.drawPath(path);

	// 绘制边框
	painter.setPen(QPen(QColor(220, 220, 220), 1));
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(path);

	QWidget::paintEvent(event);
}
