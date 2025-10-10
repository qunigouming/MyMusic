#pragma once

#include <QWidget>
#include <QLabel>

class PlaylistItemWidget  : public QWidget
{
	Q_OBJECT

public:
	PlaylistItemWidget(const QString& icon_url, const QString& name, int index, QWidget *parent = nullptr);
	~PlaylistItemWidget();

	void setSelected(bool selected);

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

signals:
	void clicked(int index);

private:
	void updateStyle();

private:
	int _index = 0;
	QLabel* _iconLabel = nullptr;
	QLabel* _textLabel = nullptr;
	bool _isHovered = false;
	bool _isPressed = false;
};

