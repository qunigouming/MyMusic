#include "SearchLine.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

SearchLine::SearchLine(QWidget*parent)
	: QWidget(parent)
{
    setFixedSize(100, 25);
	QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(12, 6, 12, 6);
    main_layout->setStretch(1, 9);

	_iconLabel = new QLabel;
	_iconLabel->setFixedSize(16, 16);
	_iconLabel->setFont(QFont("otherfont"));
	_iconLabel->setStyleSheet("color: gray;");
	_iconLabel->setText(QChar(0xe005));

	_lineEdit = new QLineEdit;
	_lineEdit->setFrame(false);
	_lineEdit->setPlaceholderText(tr("搜索"));

	main_layout->addWidget(_iconLabel);
    main_layout->addWidget(_lineEdit);
}

SearchLine::~SearchLine()
{}

void SearchLine::setRadius(int radius)
{
	_cornerRadius = radius;
}

void SearchLine::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆角背景
    QPainterPath path;
    path.addRoundedRect(rect(), _cornerRadius, _cornerRadius);

    painter.fillPath(path, _backgroundColor);

    // 绘制边框
    if (_borderColor.isValid()) {
        QPen pen(_borderColor);
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    QWidget::paintEvent(event);
}

