#pragma once

#include <QWidget>
#include "ui_BottomPlayWidget.h"
#include "FFPlayer/FFPlayer.h"
#include "tableview/tableviewmodel.h"
#include "global.h"

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
	void playLastSong();
	void playNextSong();

private:
	void informMainWindow();		// 通知主窗口
	void playSigConnect();
	void uiSigConnect();

private:
	Ui::BottomPlayWidgetClass *ui;
	FFPlayer* _player = nullptr;
	int _duration = 0;
	QString _current_SongPath;
	PlayModel _playModel = PlayModel::LISTLOOP;
};

