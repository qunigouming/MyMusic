#include "PlaylistItemWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QStyleOption>
#include <QPainter>

PlaylistItemWidget::PlaylistItemWidget(const QString& icon_url, const QString& name, int index, QWidget *parent)
	: QWidget(parent), _index(index)
{
	setFixedHeight(40);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(15, 0, 15, 0);

	QLabel* iconLabel = new QLabel();
	iconLabel->setPixmap(QPixmap(icon_url));
	iconLabel->setFixedWidth(20);

	QLabel* nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-size: 14px;");

	layout->addWidget(iconLabel);
    layout->addWidget(nameLabel);
    layout->addStretch();

	setAttribute(Qt::WA_Hover);

	setStyleSheet(R"(
        QWidget:hover {
            background-color: #8bc7ff;
        }
    )");
}

PlaylistItemWidget::~PlaylistItemWidget()
{}

void PlaylistItemWidget::mousePressEvent(QMouseEvent * event)
{
	setStyleSheet("background-color: #59c5ff;");
	QWidget::mousePressEvent(event);
	emit clicked(_index);
}

void PlaylistItemWidget::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

