#include "TitleWidget.h"
#include <QHBoxLayout>
#include <QLabel>

TitleWidget::TitleWidget(const QString& title, QWidget *parent)
	: QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 10, 15, 10);

    QLabel* label = new QLabel(title);
    label->setStyleSheet("color: #888888; font-weight: bold; font-size: 14px;");
    layout->addWidget(label);
    layout->addStretch();
}

TitleWidget::~TitleWidget()
{}

