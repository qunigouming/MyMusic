#pragma once

#include <QWidget>

class NavItemWidget  : public QWidget
{
	Q_OBJECT

public:
	NavItemWidget(const QChar& icon, const QString& text, int index, QWidget *parent = nullptr);
	~NavItemWidget();

protected:
	void mousePressEvent(QMouseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

signals:
	void clicked(int index);

private:
	int _index = 0;
};

