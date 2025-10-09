#pragma once

#include <QWidget>

class PlaylistItemWidget  : public QWidget
{
	Q_OBJECT

public:
	PlaylistItemWidget(const QString& icon_url, const QString& name, int index, QWidget *parent = nullptr);
	~PlaylistItemWidget();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

signals:
	void clicked(int index);

private:
	int _index = 0;
};

