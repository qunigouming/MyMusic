#include "colorblocklabel.h"

#include <QMouseEvent>

ColorBlockLabel::ColorBlockLabel(QWidget *parent) : QLabel(parent)
{
    this->setCursor(Qt::PointingHandCursor);
}

void ColorBlockLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit clicked();
        return;
    }

    QLabel::mousePressEvent(ev);
}
