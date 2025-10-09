#include "SeparatorWidget.h"

SeparatorWidget::SeparatorWidget(QWidget *parent)
	: QFrame(parent)
{
	setFixedHeight(1);
	setStyleSheet("background-color: #555555;");
	setFrameShape(QFrame::HLine);
}

SeparatorWidget::~SeparatorWidget()
{}

