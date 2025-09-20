#pragma once

#include <QWidget>
#include "ui_VolumeWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class VolumeWidgetClass; };
QT_END_NAMESPACE

class VolumeWidget : public QWidget
{
	Q_OBJECT

public:
	VolumeWidget(QWidget *parent = nullptr);
	~VolumeWidget();

	void setVolume(int volume);

	void mute();
	int unMute();

signals:
	void volumeChanged(int volume);

protected:
	void leaveEvent(QEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

private:
	Ui::VolumeWidgetClass *ui;
	int _lastVolume = 0;			// 静音前的音量
};

