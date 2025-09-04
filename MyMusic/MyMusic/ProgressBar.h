#pragma once
#include <QSlider>
#include <QLabel>
class ProgressBar : public QSlider
{
	Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = nullptr);
    ~ProgressBar();

    void setValue(int value);
    void setTotalTime(int totalTime);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    QString formatTime(int seconds);
    void updateTooltip(int position);

private:
    bool _is_dragging = false;

    QLabel* _tooltipLabel = nullptr;
    int _totalTime = 0;
};

