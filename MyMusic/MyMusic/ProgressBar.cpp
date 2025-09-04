#include "ProgressBar.h"
#include <QMouseEvent>
#include <QStyleOptionSlider>
#include <QStyle>
#include <QScreen>
#include <QApplication>

ProgressBar::ProgressBar(QWidget *parent)
    : QSlider(parent)
{
    setMouseTracking(true);

    _tooltipLabel = new QLabel;
    _tooltipLabel->setStyleSheet(R"(
        background-color: rgba(0, 0, 0, 220);
        color: white;
        padding: 2px 5px;
        border-radius: 4px;
        font-size: 11px;
    )");
    _tooltipLabel->setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    _tooltipLabel->setAlignment(Qt::AlignCenter);
    _tooltipLabel->hide();
}

ProgressBar::~ProgressBar()
{
    delete _tooltipLabel;
}

void ProgressBar::setValue(int value)
{
    // qDebug() << "setValue:" << value;
    if (!_is_dragging) {
        QSlider::setValue(value);
    }
}

void ProgressBar::setTotalTime(int totalTime)
{
    _totalTime = totalTime;
}

void ProgressBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        _is_dragging = true;
    }
    QSlider::mousePressEvent(event);
}

void ProgressBar::mouseMoveEvent(QMouseEvent* event)
{
    updateTooltip(event->pos().x());
    QSlider::mouseMoveEvent(event);
}

void ProgressBar::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && _is_dragging) {
        _is_dragging = false;
        int ratio = (event->pos().x() * 1.0 / width()) * (maximum() - minimum());
        // qDebug() << "finalValue:" << ratio;
        QSlider::setValue(ratio);
        emit sliderReleased();
    }
    QSlider::mouseReleaseEvent(event);
}

void ProgressBar::leaveEvent(QEvent* event)
{
    _tooltipLabel->hide();
    QSlider::leaveEvent(event);
}

QString ProgressBar::formatTime(int seconds)
{
    int minutes = seconds / 60;
    seconds = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
                           .arg(seconds, 2, 10, QChar('0'));
}

void ProgressBar::updateTooltip(int position)
{
    if (_totalTime <= 0) return;

    // 计算当前位置对应的时间
    double ratio = static_cast<double>(position) / width();
    int currentTime = ratio * _totalTime;

    // 设置提示文本
    QString text = formatTime(currentTime) + "/" + formatTime(_totalTime);
    _tooltipLabel->setText(text);
    _tooltipLabel->adjustSize();

    // 计算提示框的全局位置
    QPoint globalPos = mapToGlobal(QPoint(position, 0));
    int labelWidth = _tooltipLabel->width();
    int labelHeight = _tooltipLabel->height();

    // 调整位置，确保提示框不会超出屏幕
    QRect  screenRect;
    QScreen* screenObj = screen();
    if (screenObj) {
        screenRect = screenObj->availableGeometry();
    }
    else {
        screenRect = QApplication::primaryScreen()->availableGeometry();
    }
    int x = globalPos.x() - labelWidth / 2;
    x = qMax(screenRect.left(), qMin(x, screenRect.right() - labelWidth));

    // 获取滑块位置
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt,
        QStyle::SC_SliderHandle, this);

    int y = mapToGlobal(handleRect.topLeft()).y() - labelHeight - 2;

    _tooltipLabel->move(x, y);
    _tooltipLabel->show();
}
