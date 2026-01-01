#include "BubbleFrame.h"
#include <QLabel>

BubbleFrame::BubbleFrame(QWidget *parent)
	: QWidget(parent)
{
	_scrollArea = new QScrollArea(this);
	_scrollArea->setWidgetResizable(true);
	_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	_scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

	_mainLayout = new QHBoxLayout(this);
	_mainLayout->setContentsMargins(0, 0, 0, 0);
	_mainLayout->setSpacing(0);
	_mainLayout->addWidget(_scrollArea);
}

void BubbleFrame::addMessage(const QString& msg)
{
	QLabel* label = new QLabel(msg);
	_mainLayout->addWidget(label);
}

