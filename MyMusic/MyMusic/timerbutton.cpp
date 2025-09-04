#include "timerbutton.h"
#include <QMouseEvent>

TimerButton::TimerButton(QWidget *parent) : QPushButton(parent), _counter(10)
{
    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, [this]{
        _counter--;
        if (_counter <= 0) {
            _timer->stop();
            _counter = 10;
            this->setText("发送验证码");
            this->setEnabled(true);
            return;
        }
        this->setText(QString::number(_counter));
    });
}

TimerButton::~TimerButton()
{
    _timer->stop();
}

void TimerButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        this->setEnabled(false);
        this->setText(QString::number(_counter));
        _timer->start(1000);
        emit clicked();
    }
    QPushButton::mouseReleaseEvent(e);
}
