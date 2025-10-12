#include "PlaylistItemWidget.h"
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

PlaylistItemWidget::PlaylistItemWidget(const QString& icon_url, const QString& name, int index, QWidget *parent)
	: QWidget(parent), _index(index)
{
	setFixedHeight(40);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(15, 0, 15, 0);

	_iconLabel = new QLabel();
	_iconLabel->setPixmap(QPixmap(icon_url));
	_iconLabel->setFixedWidth(20);

	_textLabel = new QLabel(name);
	_textLabel->setStyleSheet("font-size: 14px;");

	layout->addWidget(_iconLabel);
    layout->addWidget(_textLabel);
    layout->addStretch();

	setAttribute(Qt::WA_Hover);
}

PlaylistItemWidget::~PlaylistItemWidget()
{}

void PlaylistItemWidget::setSelected(bool selected)
{
	_isPressed = selected;
	updateStyle();
}

void PlaylistItemWidget::mousePressEvent(QMouseEvent * event)
{
	_isPressed = true;
	updateStyle();
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

bool PlaylistItemWidget::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == _iconLabel || obj == _textLabel) {
		switch (event->type()) {
		case QEvent::Enter:
			_isHovered = true;
			updateStyle();
			return true;
		case QEvent::Leave:
			_isHovered = false;
			updateStyle();
			return true;
		case QEvent::MouseButtonPress:
			_isPressed = true;
			updateStyle();
			emit clicked(_index);
			return true;
		default:
			break;
		}
	}
	return QWidget::eventFilter(obj, event);
}

void PlaylistItemWidget::updateStyle()
{
	QString styleSheet;
	if (_isPressed) {
		// 按下状态样式
		styleSheet =
			"PlaylistItemWidget { background-color: #59c5ff; }"
			"QLabel { background-color: #59c5ff; font-size: %1px; }";
	}
	else if (_isHovered) {
		// 鼠标悬停状态样式
		styleSheet =
			"PlaylistItemWidget { background-color: #8bc7ff; }"
			"QLabel { background-color: #8bc7ff; font-size: %1px; }";
	}
	else {
		// 默认样式
		styleSheet =
			"PlaylistItemWidget { background-color: transparent; }"
			"QLabel { color: black; font-size: %1px; }";
	}

	// 应用样式
	QWidget::setStyleSheet(styleSheet.arg(14));
	_iconLabel->setStyleSheet(styleSheet.arg(16));
	_textLabel->setStyleSheet(styleSheet.arg(14));
}