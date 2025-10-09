#include "NavItemWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>

NavItemWidget::NavItemWidget(const QChar& icon, const QString& text, int index, QWidget *parent)
	: QWidget(parent), _index(index)
{
    setFixedHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 15, 0);

    // 图标标签（这里用文本代替图标）
    QLabel* iconLabel = new QLabel();
    iconLabel->setFont(QFont("otherfont"));
    iconLabel->setText(icon);
    iconLabel->setStyleSheet("font-size: 16px;");
    iconLabel->setFixedWidth(20);

    // 文本标签
    QLabel* textLabel = new QLabel(text);
    textLabel->setStyleSheet("font-size: 14px;");

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addStretch();

    setAttribute(Qt::WA_Hover);

    setStyleSheet(R"(
        QWidget:hover {
            background-color: #8bc7ff;
        }
    )");
}

NavItemWidget::~NavItemWidget()
{}

void NavItemWidget::mousePressEvent(QMouseEvent * event)
{
    setStyleSheet("background-color: #59c5ff;");
    QWidget::mousePressEvent(event);
    emit clicked(_index);
}

void NavItemWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

