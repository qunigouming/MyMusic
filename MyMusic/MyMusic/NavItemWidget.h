#pragma once

#include <QWidget>
#include <QLabel>

class NavItemWidget  : public QWidget
{
	Q_OBJECT

public:
	NavItemWidget(const QChar& icon, const QString& text, int index, QWidget *parent = nullptr);
	~NavItemWidget();

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

