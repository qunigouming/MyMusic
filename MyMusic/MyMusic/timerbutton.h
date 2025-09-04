#ifndef TIMERBUTTON_H
#define TIMERBUTTON_H

/******************************************************************************
 *
 * @file       timerbutton.h
 * @brief      The button have count down functional
 *
 * @author     qunigouming
 * @date       2025/07/02
 * @history
 *****************************************************************************/

#include <QPushButton>
#include <QTimer>

class TimerButton : public QPushButton
{
    Q_OBJECT
public:
    TimerButton(QWidget* parent = nullptr);
    ~TimerButton();
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QTimer* _timer;
    int _counter;
};

#endif // TIMERBUTTON_H
