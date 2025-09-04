#include "funclistwidgetitem.h"

#include <QHBoxLayout>

FuncListWidgetItem::FuncListWidgetItem(QChar icon, QString text, QWidget *parent)
    : QWidget{parent}
{
    setMinimumSize(180, 50);

    QHBoxLayout* hpagelayout = new QHBoxLayout(this);
    icon_Lab = new QLabel();
    icon_Lab->setFont(QFont("otherfont"));
    icon_Lab->setText(icon);
    icon_Lab->setObjectName("pageIcon");
    icon_Lab->setStyleSheet(R"(#pageIcon { font-size: 20px; })");
    icon_Lab->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    title_Lab = new QLabel();
    title_Lab->setText(text);
    title_Lab->setObjectName("pageText");
    title_Lab->setStyleSheet(R"(#pageText { font-size: 18px; })");
    title_Lab->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    hpagelayout->addWidget(icon_Lab);
    hpagelayout->addWidget(title_Lab);
    hpagelayout->setStretch(0, 1);
    hpagelayout->setStretch(1, 3);
}
