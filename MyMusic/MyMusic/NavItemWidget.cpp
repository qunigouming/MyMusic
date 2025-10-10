#include "NavItemWidget.h"
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

NavItemWidget::NavItemWidget(const QChar& icon, const QString& text, int index, QWidget *parent)
	: QWidget(parent), _index(index)
{
    setFixedHeight(40);

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(15, 0, 15, 0);

    // 图标标签（这里用文本代替图标）
    _iconLabel = new QLabel();
    _iconLabel->setFont(QFont("otherfont"));
    _iconLabel->setText(icon);
    _iconLabel->setStyleSheet("font-size: 16px;");
    _iconLabel->setFixedWidth(20);
    _iconLabel->installEventFilter(this);

    // 文本标签
    _textLabel = new QLabel(text);
    _textLabel->setStyleSheet("font-size: 14px;");
    _textLabel->installEventFilter(this);

    layout->addWidget(_iconLabel);
    layout->addWidget(_textLabel);
    layout->addStretch();

    setAttribute(Qt::WA_Hover);

    setMouseTracking(true);
}

NavItemWidget::~NavItemWidget()
{}

void NavItemWidget::setSelected(bool selected)
{
    _isPressed = selected;
    updateStyle();
}

void NavItemWidget::mousePressEvent(QMouseEvent * event)
{
    // setStyleSheet("background-color: #59c5ff;");
    _isPressed = true;
    updateStyle();
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

bool NavItemWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == _iconLabel || obj == _textLabel) {
        switch (event->type()) {
        case QEvent::Enter:
            _isHovered = true;
            updateStyle();
            break;
        case QEvent::Leave:
            _isHovered = false;
            updateStyle();
            break;
        case QEvent::MouseButtonPress:
            _isPressed = true;
            updateStyle();
            emit clicked(_index);
            break;
        default:
            break;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void NavItemWidget::updateStyle()
{
    QString styleSheet;
    if (_isPressed) {
        // 按下状态样式
        styleSheet =
            "NavItemWidget { background-color: #59c5ff; }"
            "QLabel { background-color: #59c5ff; font-size: %1px; }";
    } else if (_isHovered) {
        // 鼠标悬停状态样式
        styleSheet =
            "NavItemWidget { background-color: #8bc7ff; }"
            "QLabel { background-color: #8bc7ff; font-size: %1px; }";
    } else {
        // 默认样式
        styleSheet =
            "NavItemWidget { background-color: transparent; }"
            "QLabel { color: black; font-size: %1px; }";
    }

    // 应用样式
    QWidget::setStyleSheet(styleSheet.arg(14));
    _iconLabel->setStyleSheet(styleSheet.arg(16));
    _textLabel->setStyleSheet(styleSheet.arg(14));
}

