#pragma once

#include <QWidget>
#include "ui_BottomPlayWidget.h"
#include "FFPlayer/FFPlayer.h"
#include "tableview/tableviewmodel.h"
#include "global.h"
#include "VolumeWidget.h"
#include <QSharedPointer>

QT_BEGIN_NAMESPACE
namespace Ui { class BottomPlayWidgetClass; };
QT_END_NAMESPACE

class BottomPlayWidget : public QWidget
{
	Q_OBJECT
public:
	BottomPlayWidget(QWidget *parent = nullptr);
	~BottomPlayWidget();

	void play(const SongInfo& info);

signals:
	void playLastSong(PlayModel);
	void playNextSong(PlayModel);
	void playModelChanged(PlayModel);

protected:
	bool eventFilter(QObject *watched, QEvent *event) override;

private:
	void playSigConnect();
	void uiSigConnect();

	void setVolumeUI(int volume);

private:
	Ui::BottomPlayWidgetClass *ui;
	FFPlayer* _player = nullptr;
	int _duration = 0;
	QString _current_SongPath;
	PlayModel _playModel = PlayModel::LISTLOOP;
	QSharedPointer<VolumeWidget> _volumeWidget = nullptr;
};

