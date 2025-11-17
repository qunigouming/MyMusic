#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QLabel>

class SearchLine  : public QWidget
{
	Q_OBJECT

public:
	SearchLine(QWidget*parent);
	~SearchLine();
	void setRadius(int radius);

protected:
	void paintEvent(QPaintEvent* event) override;

private:
	QLabel* _iconLabel = nullptr;
	QLineEdit* _lineEdit = nullptr;
	int _cornerRadius = 10;
	QColor _backgroundColor = QColor(Qt::white);
	QColor _textColor =	QColor(Qt::black);
	QColor _borderColor = QColor(Qt::lightGray);
};

