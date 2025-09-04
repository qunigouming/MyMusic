#ifndef COLORBLOCKLABEL_H
#define COLORBLOCKLABEL_H

#include <QLabel>

class ColorBlockLabel : public QLabel
{
    Q_OBJECT
public:
    ColorBlockLabel(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void clicked();
};

#endif // COLORBLOCKLABEL_H
